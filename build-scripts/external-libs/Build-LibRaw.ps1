#Requires -Version 7.0
# DarkThumbs v7.0 - Build LibRaw 0.21.2 (RAW Camera Image Decoder)
# Refactored to use Build-Library-Core.ps1 module
# Date: February 18, 2026
#
# Directory structure (post-cleanup):
#   Project root:       <repo>\
#   This script:        <repo>\build-scripts\external-libs\Build-LibRaw.ps1
#   Core module:        <repo>\build-scripts\core\Build-Library-Core.ps1
#   LibRaw source:      <repo>\external\camera-libs\libraw\
#   Install dir:        <repo>\external\camera-libs\libraw-install\

param(
    [string]$Configuration = "Release",
    [switch]$Clean
)

# Import core build module
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$librawDir = Join-Path $rootDir "external\camera-libs\libraw"
$installDir = Join-Path $rootDir "external\camera-libs\libraw-install"

Write-BuildHeader "Building LibRaw 0.21.2 (Camera RAW Decoder)"

# Verify source directory
if (-not (Test-Path $librawDir)) {
    Write-BuildLog "LibRaw source not found at $librawDir" -Level Error
    Write-BuildLog "Please download LibRaw 0.21.2 from libraw.org" -Level Warning
    exit 1
}

Write-BuildLog "Source: $librawDir" -Level Info
Write-BuildLog "Install: $installDir" -Level Info

try {
    # LibRaw typically has Visual Studio solution
    $solutionFile = Join-Path $librawDir "LibRaw.sln"
    $vcxproj = Join-Path $librawDir "LibRaw.vcxproj"
    
    if (Test-Path $solutionFile) {
        Write-BuildLog "Using MSBuild with solution file" -Level Info
        
        # Build using MSBuild (LibRaw has VS solution)
        $msbuildPath = Find-MSBuildPath
        if (-not $msbuildPath) {
            throw "MSBuild not found"
        }
        
        & $msbuildPath `
            $solutionFile `
            /t:libraw `
            /p:Configuration=$Configuration `
            /p:Platform=x64 `
            /p:WindowsTargetPlatformVersion=10.0.26100.0 `
            /p:PlatformToolset=v145 `
            /m `
            /v:minimal `
            /nologo
        
        if ($LASTEXITCODE -ne 0) {
            throw "MSBuild failed with exit code $LASTEXITCODE"
        }
    } elseif (Test-Path $vcxproj) {
        Write-BuildLog "Using MSBuild with project file" -Level Info
        
        Invoke-MSBuildLibrary `
            -LibraryName "LibRaw" `
            -ProjectFile $vcxproj `
            -Configuration $Configuration `
            -Platform "x64" `
            -Clean:$Clean
    } else {
        throw "No Visual Studio solution or project file found"
    }
    
    # Find output (LibRaw uses non-standard naming)
    $searchPatterns = @(
        (Join-Path $librawDir "buildfiles\release-x86_64\libraw.lib"),
        (Join-Path $librawDir "buildfiles\x64\$Configuration\libraw.lib"),
        (Join-Path $librawDir "lib\libraw.lib")
    )
    
    $outputLib = $null
    foreach ($pattern in $searchPatterns) {
        if (Test-Path $pattern) {
            $outputLib = $pattern
            break
        }
    }
    
    if (-not $outputLib) {
        throw "libraw.lib not found in expected locations"
    }
    
    Write-BuildLog "Found: $outputLib" -Level Success
    
    # Install headers and library
    $installInclude = Join-Path $installDir "include"
    $installLib = Join-Path $installDir "lib"
    
    New-Item -ItemType Directory -Path $installInclude -Force | Out-Null
    New-Item -ItemType Directory -Path $installLib -Force | Out-Null
    
    Copy-Item $outputLib -Destination (Join-Path $installLib "libraw_static.lib") -Force
    Copy-Item (Join-Path $librawDir "libraw") -Destination (Join-Path $installInclude "libraw") -Recurse -Force
    
    Write-BuildLog "LibRaw 0.21.2 build completed successfully" -Level Success
    Write-BuildLog "Supports 100+ RAW formats: Canon CR2/CR3, Nikon NEF, Sony ARW, etc." -Level Info
    
} catch {
    Write-BuildLog "Build failed: $($_.Exception.Message)" -Level Error
    exit 1
}
