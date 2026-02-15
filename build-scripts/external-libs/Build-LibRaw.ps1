# Build LibRaw for Camera RAW Support
# DarkThumbs External Library Build Script

param(
    [switch]$Clean,
    [string]$Configuration = "Release",
    [string]$Platform = "x64"
)

$ErrorActionPreference = "Stop"

# Paths
$repoRoot = Split-Path -Parent $PSScriptRoot
$externalDir = Join-Path $repoRoot "external"
$librawDir = Join-Path $externalDir "libraw"
$solutionFile = Join-Path $librawDir "LibRaw.sln"
$installDir = Join-Path $externalDir "libraw-install"

Write-Host "================================================" -ForegroundColor Cyan
Write-Host "  Building LibRaw for Camera RAW Support" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

# Verify LibRaw source exists
if (-not (Test-Path $librawDir)) {
    Write-Host "ERROR: LibRaw source not found at: $librawDir" -ForegroundColor Red
    Write-Host "Please download LibRaw 0.21.2 and extract to external/camera-libs/libraw/" -ForegroundColor Yellow
    exit 1
}

if (-not (Test-Path $solutionFile)) {
    Write-Host "ERROR: LibRaw.sln not found at: $solutionFile" -ForegroundColor Red
    exit 1
}

# Clean if requested
if ($Clean) {
    Write-Host "Cleaning LibRaw build outputs..." -ForegroundColor Yellow
    $patterns = @("buildfiles\$Platform\$Configuration", "bin", "lib", "obj")
    foreach ($pattern in $patterns) {
        $path = Join-Path $librawDir $pattern
        if (Test-Path $path) {
            Remove-Item -Recurse -Force $path -ErrorAction SilentlyContinue
        }
    }
    if (Test-Path $installDir) {
        Remove-Item -Recurse -Force $installDir
    }
}

# Build LibRaw with MSBuild
Write-Host "Building LibRaw with MSBuild ($Configuration|$Platform)..." -ForegroundColor Green
Write-Host "Solution: $solutionFile" -ForegroundColor Gray
Write-Host ""

Push-Location $librawDir

try {
    # Build just the libraw project (not samples)
    # Note: Retargeting to current VS toolset and Windows SDK
    msbuild $solutionFile `
        /t:libraw `
        /p:Configuration=$Configuration `
        /p:Platform=$Platform `
        /p:WindowsTargetPlatformVersion=10.0.26100.0 `
        /p:PlatformToolset=v143 `
        /m `
        /v:minimal

    if ($LASTEXITCODE -ne 0) {
        throw "MSBuild failed with exit code $LASTEXITCODE"
    }

    # Find the output library (LibRaw uses release-x86_64 directory naming)
    $searchPatterns = @(
        "buildfiles\release-x86_64\libraw.lib",
        "buildfiles\$Platform\$Configuration\libraw.lib",
        "lib\libraw.lib",
        "*\libraw.lib"
    )
    
    $outputLib = $null
    foreach ($pattern in $searchPatterns) {
        $fullPattern = Join-Path $librawDir $pattern
        Write-Host "Searching: $fullPattern" -ForegroundColor Gray
        $found = Get-Item $fullPattern -ErrorAction SilentlyContinue
        if ($found) {
            $outputLib = $found
            break
        }
    }

    if (-not $outputLib) {
        throw "Could not find libraw.lib in build output"
    }

    Write-Host ""
    Write-Host "Found library: $($outputLib.FullName)" -ForegroundColor Green

    # Create install directory structure
    $installInclude = Join-Path $installDir "include"
    $installLib = Join-Path $installDir "lib"
    
    New-Item -ItemType Directory -Path $installInclude -Force | Out-Null
    New-Item -ItemType Directory -Path $installLib -Force | Out-Null

    # Copy library
    Write-Host "Installing to: $installDir" -ForegroundColor Green
    Copy-Item $outputLib.FullName -Destination (Join-Path $installLib "libraw_static.lib") -Force

    # Copy headers
    $librawHeaders = Join-Path $librawDir "libraw"
    Copy-Item $librawHeaders -Destination (Join-Path $installInclude "libraw") -Recurse -Force

    Write-Host ""
    Write-Host "================================================" -ForegroundColor Green
    Write-Host "  LibRaw Build Complete!" -ForegroundColor Green
    Write-Host "================================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "================================================" -ForegroundColor Green
    Write-Host "  LibRaw Build Complete!" -ForegroundColor Green
    Write-Host "================================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Installation Location:" -ForegroundColor Cyan
    Write-Host "  $installDir" -ForegroundColor White
    Write-Host ""
    Write-Host "Include Directory:" -ForegroundColor Cyan
    Write-Host "  $installInclude\libraw" -ForegroundColor White
    Write-Host ""
    Write-Host "Library File:" -ForegroundColor Cyan
    Write-Host "  $installLib\libraw_static.lib" -ForegroundColor White
    
    # Show library size
    $libSize = (Get-Item (Join-Path $installLib "libraw_static.lib")).Length / 1MB
    Write-Host "  Size: $([math]::Round($libSize, 2)) MB" -ForegroundColor White
    Write-Host ""
    Write-Host "Supported RAW Formats:" -ForegroundColor Cyan
    Write-Host "  Canon: .cr2, .cr3, .crw" -ForegroundColor White
    Write-Host "  Nikon: .nef, .nrw" -ForegroundColor White
    Write-Host "  Sony: .arw, .srf, .sr2" -ForegroundColor White
    Write-Host "  Olympus: .orf" -ForegroundColor White
    Write-Host "  Panasonic: .rw2" -ForegroundColor White
    Write-Host "  Pentax: .pef" -ForegroundColor White
    Write-Host "  Fujifilm: .raf" -ForegroundColor White
    Write-Host "  Adobe: .dng" -ForegroundColor White
    Write-Host "  And 100+ more RAW formats..." -ForegroundColor White
    Write-Host ""

} catch {
    Write-Host ""
    Write-Host "================================================" -ForegroundColor Red
    Write-Host "  Build Failed!" -ForegroundColor Red
    Write-Host "================================================" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
    Write-Host "Stack Trace:" -ForegroundColor Yellow
    Write-Host $_.ScriptStackTrace -ForegroundColor Gray
    exit 1
} finally {
    Pop-Location
}
