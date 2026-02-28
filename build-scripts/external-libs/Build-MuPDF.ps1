#Requires -Version 7.0
# ExplorerLens v15.0 "Zenith" — Build MuPDF 1.24.11 Static Library
#
# Directory structure:
#   Project root:       <repo>\
#   This script:        <repo>\build-scripts\external-libs\Build-MuPDF.ps1
#   Core module:        <repo>\build-scripts\core\Build-Library-Core.ps1
#   MuPDF source:       <repo>\external\pdf-libs\mupdf-1.24.11-source\
#   Output:             <repo>\external\pdf-libs\mupdf-1.24.11-source\x64\Release\

param(
    [switch]$Clean,
    [ValidateSet('Release', 'Debug')]
    [string]$Configuration = 'Release'
)

# Import core build module
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$mupdfDir = Join-Path $rootDir "external\pdf-libs\mupdf-1.24.11-source"
$solutionFile = Join-Path $mupdfDir "platform\win32\mupdf.sln"
$outputDir = Join-Path $mupdfDir "x64\$Configuration"

Write-BuildHeader "Building MuPDF 1.24.11 (MSBuild)"

# Verify source directory
if (-not (Test-Path $mupdfDir)) {
    Write-BuildLog "MuPDF source not found at $mupdfDir" -Level Error
    Write-BuildLog "Download from: https://mupdf.com/releases/mupdf-1.24.11-source.tar.gz" -Level Info
    exit 1
}

if (-not (Test-Path $solutionFile)) {
    Write-BuildLog "MuPDF solution file not found at $solutionFile" -Level Error
    exit 1
}

Write-BuildLog "Source: $mupdfDir" -Level Info
Write-BuildLog "Solution: $solutionFile" -Level Info

# Find MSBuild
$msbuild = Find-MSBuildPath
if (-not $msbuild) {
    Write-BuildLog "MSBuild not found. Install Visual Studio 18 2026 Build Tools." -Level Error
    exit 1
}

# Clean if requested
if ($Clean) {
    Write-BuildLog "Cleaning previous build artifacts..." -Level Info
    $cleanDirs = @(
        (Join-Path $mupdfDir "x64"),
        (Join-Path $mupdfDir "platform\win32\x64")
    )
    foreach ($dir in $cleanDirs) {
        if (Test-Path $dir) {
            Remove-Item $dir -Recurse -Force -ErrorAction SilentlyContinue
            Write-BuildLog "Removed $dir" -Level Info
        }
    }
}

try {
    # Step 1: Build libthirdparty (bundled dependencies: freetype, harfbuzz, libjpeg, openjpeg, etc.)
    Write-BuildLog "Building MuPDF third-party libraries..." -Level Info
    & $msbuild $solutionFile `
        /t:libthirdparty `
        /p:Configuration=$Configuration `
        /p:Platform=x64 `
        /p:RuntimeLibrary=MD `
        /p:PlatformToolset=v145 `
        /m /v:minimal
    
    if ($LASTEXITCODE -ne 0) {
        Write-BuildLog "libthirdparty build failed (exit code $LASTEXITCODE)" -Level Warning
        Write-BuildLog "Attempting full solution build instead..." -Level Info
    }

    # Step 2: Build libresources (generated fonts and CMap data)
    Write-BuildLog "Building MuPDF resources..." -Level Info
    & $msbuild $solutionFile `
        /t:libresources `
        /p:Configuration=$Configuration `
        /p:Platform=x64 `
        /p:RuntimeLibrary=MD `
        /p:PlatformToolset=v145 `
        /m /v:minimal 2>&1 | Out-Null
    
    # Step 3: Build libmupdf (core library — what we link against)
    Write-BuildLog "Building libmupdf core library..." -Level Info
    & $msbuild $solutionFile `
        /t:libmupdf `
        /p:Configuration=$Configuration `
        /p:Platform=x64 `
        /p:RuntimeLibrary=MD `
        /p:PlatformToolset=v145 `
        /m /v:minimal
    
    if ($LASTEXITCODE -ne 0) {
        throw "libmupdf build failed with exit code $LASTEXITCODE"
    }

    # Find the built library
    Write-BuildLog "Verifying build outputs..." -Level Info
    
    $possiblePaths = @(
        (Join-Path $mupdfDir "platform\win32\x64\$Configuration\libmupdf.lib"),
        (Join-Path $mupdfDir "x64\$Configuration\libmupdf.lib"),
        (Join-Path $mupdfDir "platform\win32\$Configuration\libmupdf.lib")
    )

    $foundLib = $null
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            $foundLib = $path
            break
        }
    }

    # Fallback: search recursively
    if (-not $foundLib) {
        $searchResult = Get-ChildItem $mupdfDir -Recurse -Filter "libmupdf.lib" -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -match "x64|$Configuration" } |
            Select-Object -First 1
        if ($searchResult) {
            $foundLib = $searchResult.FullName
        }
    }

    if (-not $foundLib) {
        throw "libmupdf.lib not found after build. Check build output for errors."
    }

    $libSize = [math]::Round((Get-Item $foundLib).Length / 1MB, 1)
    Write-BuildLog "Found libmupdf.lib ($libSize MB) at: $foundLib" -Level Success

    # Also look for libthirdparty.lib (needed for linking)
    $thirdpartyLib = $foundLib -replace 'libmupdf\.lib$', 'libthirdparty.lib'
    if (Test-Path $thirdpartyLib) {
        $tpSize = [math]::Round((Get-Item $thirdpartyLib).Length / 1MB, 1)
        Write-BuildLog "Found libthirdparty.lib ($tpSize MB)" -Level Success
    }

    # Copy to standard output location
    if (-not (Test-Path $outputDir)) {
        New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
    }

    if ($foundLib -ne (Join-Path $outputDir "libmupdf.lib")) {
        Copy-Item $foundLib (Join-Path $outputDir "libmupdf.lib") -Force
        Write-BuildLog "Copied libmupdf.lib to $outputDir" -Level Success
    }

    if ((Test-Path $thirdpartyLib) -and $thirdpartyLib -ne (Join-Path $outputDir "libthirdparty.lib")) {
        Copy-Item $thirdpartyLib (Join-Path $outputDir "libthirdparty.lib") -Force
        Write-BuildLog "Copied libthirdparty.lib to $outputDir" -Level Success
    }

    # Check for libresources.lib
    $resourcesLib = $foundLib -replace 'libmupdf\.lib$', 'libresources.lib'
    if (Test-Path $resourcesLib) {
        Copy-Item $resourcesLib (Join-Path $outputDir "libresources.lib") -Force
        Write-BuildLog "Copied libresources.lib" -Level Success
    }

    Write-BuildLog "" -Level Info
    Write-BuildLog "MuPDF 1.24.11 build completed successfully!" -Level Success
    Write-BuildLog "Output directory: $outputDir" -Level Info
    Write-BuildLog "" -Level Info
    Write-BuildLog "To enable PDF support in ExplorerLens:" -Level Info
    Write-BuildLog "  1. Set HAS_MUPDF=ON in CMakePresets.json" -Level Info
    Write-BuildLog "  2. Rebuild Engine: cmake --build --preset default-release" -Level Info

} catch {
    Write-BuildLog "Build failed: $($_.Exception.Message)" -Level Error
    Write-BuildLog "Make sure Visual Studio 18 2026 Build Tools are installed with v145 toolset" -Level Info
    exit 1
}
