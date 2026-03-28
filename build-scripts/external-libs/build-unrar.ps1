#Requires -Version 7.0
# ExplorerLens v7.0 - Build UnRAR DLL (RAR Archive Extraction)
# Refactored to use Build-Library-Core.ps1 module
# Date: February 18, 2026
#
# Directory structure (post-cleanup):
#   Project root:       <repo>\
#   This script:        <repo>\build-scripts\external-libs\Build-UnRAR.ps1
#   Core module:        <repo>\build-scripts\core\Build-Library-Core.ps1
#   UnRAR source:       <repo>\external\compression-libs\unrar\
#   Build output:       <repo>\x64\Release\UnRAR64.dll

param(
    [string]$Configuration = "Release",
    [switch]$Clean
)

# Import core build module
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$unrarDir = Join-Path $rootDir "external\compression-libs\unrar-7.2.2"
$projectFile = Join-Path $unrarDir "UnRARDll.vcxproj"
$outputDir = Join-Path $rootDir "x64\Release"

Write-BuildHeader "Building UnRAR DLL (RAR Archive Extraction)"

# Verify source directory
if (-not (Test-Path $unrarDir)) {
    Write-BuildLog "UnRAR source not found at $unrarDir" -Level Error
    Write-BuildLog "Please download and extract UnRAR source" -Level Warning
    exit 1
}

if (-not (Test-Path $projectFile)) {
    Write-BuildLog "UnRARDll.vcxproj not found at $projectFile" -Level Error
    exit 1
}

Write-BuildLog "Source:  $unrarDir" -Level Info
Write-BuildLog "Project: UnRARDll.vcxproj" -Level Info
Write-BuildLog "Output:  $outputDir" -Level Info

try {
    # Find MSBuild using Build-Library-Core.ps1 helper
    $msbuildPath = Find-MSBuildPath
    if (-not $msbuildPath) {
        throw "MSBuild not found"
    }
    Write-BuildLog "MSBuild: $msbuildPath" -Level Info

    # Clean if requested
    if ($Clean) {
        $cleanDirs = @(
            (Join-Path $unrarDir "x64"),
            (Join-Path $unrarDir "Release")
        )
        foreach ($d in $cleanDirs) {
            if (Test-Path $d) {
                Remove-Item $d -Recurse -Force -ErrorAction SilentlyContinue
                Write-BuildLog "Cleaned: $d" -Level Info
            }
        }
    }

    # Build UnRAR DLL
    Write-BuildLog "Building UnRAR64.dll..." -Level Info
    $buildArgs = @(
        $projectFile
        "/p:Configuration=$Configuration"
        "/p:Platform=x64"
        "/p:PlatformToolset=v145"
        "/v:minimal"
        "/m"
    )

    & $msbuildPath $buildArgs

    if ($LASTEXITCODE -ne 0) {
        throw "MSBuild failed with exit code $LASTEXITCODE"
    }

    # Find the output DLL
    Write-BuildLog "Locating output DLL..." -Level Info
    $possibleLocations = @(
        (Join-Path $unrarDir "build\unrardll64\$Configuration\UnRAR64.dll"),
        (Join-Path $unrarDir "build\unrardll64\$Configuration\UnRAR.dll"),
        (Join-Path $unrarDir "x64\Release\UnRAR64.dll"),
        (Join-Path $unrarDir "x64\Release\UnRAR.dll"),
        (Join-Path $unrarDir "Release\UnRAR64.dll")
    )

    $dllPath = $null
    foreach ($location in $possibleLocations) {
        if (Test-Path $location) {
            $dllPath = $location
            break
        }
    }

    if (-not $dllPath) {
        Write-BuildLog "UnRAR64.dll not found in expected locations:" -Level Error
        foreach ($loc in $possibleLocations) {
            Write-BuildLog "  $loc" -Level Warning
        }
        throw "UnRAR64.dll not found"
    }

    # Copy to project output directory
    $size = (Get-Item $dllPath).Length / 1KB
    Write-BuildLog "Found: $dllPath ($([math]::Round($size, 0)) KB)" -Level Success

    if (-not (Test-Path $outputDir)) {
        New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
    }

    $targetPath = Join-Path $outputDir "UnRAR64.dll"
    Copy-Item $dllPath $targetPath -Force
    Write-BuildLog "Copied to: $targetPath" -Level Success

    Write-BuildLog "UnRAR build completed successfully" -Level Success
    Write-BuildLog "Provides RAR archive decompression support" -Level Info

} catch {
    Write-BuildLog "Build failed: $($_.Exception.Message)" -Level Error
    exit 1
}
