#Requires -Version 7.0
# DarkThumbs v7.0 - Build libheif 1.19.5 (HEIF/HEIC Support)
# Refactored to use Build-Library-Core.ps1 module
# Date: February 16, 2026

param(
    [string]$Configuration = "Release",
    [switch]$Clean,
    [string]$Libde265ZipPath = "C:\Users\ryair\Downloads\libde265-master.zip",
    [string]$LibheifZipPath = "",
    [string]$GitProxyUrl = "http://proxy-chain.intel.com:928",
    [bool]$DisableGitSslVerifyForProxy = $true
)

# Import core build module
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$externalDir = Join-Path $rootDir "external"
$imageLibsDir = Join-Path $externalDir "image-libs"
$libheifDir = Join-Path $imageLibsDir "libheif-1.19.5"
$buildDir = Join-Path $libheifDir "build-vs"
$installDir = Join-Path $libheifDir "install"

# Also need libde265 (HEVC decoder dependency)
$libde265Dir = Join-Path $imageLibsDir "libde265-1.0.15"
$libde265BuildDir = Join-Path $libde265Dir "build-vs"
$libde265InstallDir = Join-Path $libde265Dir "install"

Write-BuildHeader "Building libheif 1.19.5 (HEIF/HEIC Support)"

function Invoke-GitClone {
    param(
        [Parameter(Mandatory = $true)][string]$Repository,
        [Parameter(Mandatory = $true)][string]$Branch,
        [Parameter(Mandatory = $true)][string]$Destination,
        [string]$ProxyUrl = "",
        [bool]$DisableSslVerify = $false
    )

    $cloneArgs = @('clone', '--depth', '1', '--branch', $Branch, $Repository, $Destination)

    if ($ProxyUrl) {
        Write-BuildLog "Using Git proxy: $ProxyUrl" -Level Info

        $proxyArgs = @('-c', "http.proxy=$ProxyUrl", '-c', "https.proxy=$ProxyUrl")
        & git @proxyArgs @cloneArgs

        if ($LASTEXITCODE -ne 0 -and $DisableSslVerify) {
            Write-BuildLog "Git clone failed through proxy. Retrying with http.sslVerify=false for corporate TLS proxy compatibility." -Level Warning
            & git @proxyArgs '-c' 'http.sslVerify=false' @cloneArgs
        }
    } else {
        & git @cloneArgs
    }

    if ($LASTEXITCODE -ne 0 -or -not (Test-Path $Destination)) {
        throw "git clone failed for $Repository ($Branch)"
    }
}

function Expand-SourceZip {
    param(
        [Parameter(Mandatory = $true)][string]$ZipPath,
        [Parameter(Mandatory = $true)][string]$DestinationDir,
        [Parameter(Mandatory = $true)][string]$FinalDir,
        [Parameter(Mandatory = $true)][string]$ProjectName
    )

    if (-not (Test-Path $ZipPath)) {
        throw "$ProjectName zip not found: $ZipPath"
    }

    Write-BuildLog "Using local $ProjectName zip: $ZipPath" -Level Info

    $extractTempDir = Join-Path $DestinationDir ("tmp-extract-" + $ProjectName)
    if (Test-Path $extractTempDir) {
        Remove-Item -Recurse -Force $extractTempDir
    }
    New-Item -ItemType Directory -Path $extractTempDir -Force | Out-Null

    Expand-Archive -Path $ZipPath -DestinationPath $extractTempDir -Force

    $firstSubdir = Get-ChildItem -Path $extractTempDir -Directory | Select-Object -First 1
    if (-not $firstSubdir) {
        throw "$ProjectName zip extraction produced no source directory"
    }

    if (Test-Path $FinalDir) {
        Remove-Item -Recurse -Force $FinalDir
    }

    Move-Item -Path $firstSubdir.FullName -Destination $FinalDir
    Remove-Item -Recurse -Force $extractTempDir -ErrorAction SilentlyContinue
}

# ============================================================================
# Step 1: Download libde265 if not present
# ============================================================================

if (-not (Test-Path $libde265Dir)) {
    try {
        if (Test-Path $Libde265ZipPath) {
            Expand-SourceZip `
                -ZipPath $Libde265ZipPath `
                -DestinationDir $imageLibsDir `
                -FinalDir $libde265Dir `
                -ProjectName "libde265"
        } else {
            Write-BuildLog "Downloading libde265 1.0.15 (HEVC decoder)..." -Level Info
            Push-Location $imageLibsDir
            Invoke-GitClone -Repository "https://github.com/strukturag/libde265.git" -Branch "v1.0.15" -Destination "libde265-1.0.15" -ProxyUrl $GitProxyUrl -DisableSslVerify $DisableGitSslVerifyForProxy
            Pop-Location
        }
    } catch {
        Write-BuildLog "Failed to obtain libde265 source: $($_.Exception.Message)" -Level Error
        Write-BuildLog "Manual download: https://github.com/strukturag/libde265/releases or provide -Libde265ZipPath" -Level Warning
        if ((Get-Location).Path -like "*$imageLibsDir*") {
            Pop-Location
        }
        exit 1
    }
}

# ============================================================================
# Step 2: Build libde265
# ============================================================================

Write-BuildLog "Building libde265 1.0.15 (HEVC decoder dependency)..." -Level Info

$de265ImportLibCandidates = @(
    (Join-Path $libde265BuildDir "libde265\Release\de265.lib"),
    (Join-Path $libde265InstallDir "lib\de265.lib"),
    (Join-Path $libde265InstallDir "lib\libde265.lib")
)
$de265RuntimeDllCandidates = @(
    (Join-Path $libde265BuildDir "libde265\Release\libde265.dll"),
    (Join-Path $libde265InstallDir "bin\libde265.dll")
)

$de265ImportLib = $de265ImportLibCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $de265ImportLib) {
    $de265Options = @{
        'CMAKE_BUILD_TYPE'  = 'Release'
        'BUILD_SHARED_LIBS' = 'ON'
        'ENABLE_SDL'        = 'OFF'
        'ENABLE_DECODER'    = 'ON'
        'ENABLE_ENCODER'    = 'OFF'
    }

    try {
        Invoke-CMakeBuild `
            -LibraryName "libde265" `
            -SourceDir $libde265Dir `
            -BuildDir $libde265BuildDir `
            -InstallDir $libde265InstallDir `
            -Configuration $Configuration `
            -CMakeOptions $de265Options `
            -Clean:$Clean

        Write-BuildLog "libde265 built successfully" -Level Success

        $de265ImportLib = $de265ImportLibCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
        if (-not $de265ImportLib) {
            throw "libde265 import library not found after build"
        }
    } catch {
        Write-BuildLog "libde265 build failed: $($_.Exception.Message)" -Level Error
        exit 1
    }
} else {
    Write-BuildLog "libde265 already built (found $de265ImportLib)" -Level Info
}

# ============================================================================
# Step 3: Download libheif if not present
# ============================================================================

if (-not (Test-Path $libheifDir)) {
    try {
        if ($LibheifZipPath -and (Test-Path $LibheifZipPath)) {
            Expand-SourceZip `
                -ZipPath $LibheifZipPath `
                -DestinationDir $imageLibsDir `
                -FinalDir $libheifDir `
                -ProjectName "libheif"
        } else {
            Write-BuildLog "Downloading libheif 1.19.5..." -Level Info
            Push-Location $imageLibsDir
            Invoke-GitClone -Repository "https://github.com/strukturag/libheif.git" -Branch "v1.19.5" -Destination "libheif-1.19.5" -ProxyUrl $GitProxyUrl -DisableSslVerify $DisableGitSslVerifyForProxy
            Pop-Location
        }
    } catch {
        Write-BuildLog "Failed to obtain libheif source: $($_.Exception.Message)" -Level Error
        Write-BuildLog "Manual download: https://github.com/strukturag/libheif/releases or provide -LibheifZipPath" -Level Warning
        if ((Get-Location).Path -like "*$imageLibsDir*") {
            Pop-Location
        }
        exit 1
    }
}

# Clean if requested
if ($Clean -and (Test-Path $buildDir)) {
    Remove-Item -Recurse -Force $buildDir
}

# ============================================================================
# Step 4: Build libheif
# ============================================================================

Write-BuildLog "Source: $libheifDir" -Level Info
Write-BuildLog "Build: $buildDir" -Level Info
Write-BuildLog "Install: $installDir" -Level Info

$libde265IncludeDir = Join-Path $libde265InstallDir 'include'
$libde265LibraryPath = $de265ImportLib

$cmakeOptions = @{
    'CMAKE_BUILD_TYPE'      = 'Release'
    'BUILD_SHARED_LIBS'     = 'OFF'
    'CMAKE_C_FLAGS'         = '/DLIBDE265_STATIC_BUILD'
    'CMAKE_CXX_FLAGS'       = '/DLIBDE265_STATIC_BUILD'
    'BUILD_TESTING'         = 'OFF'
    'WITH_EXAMPLES'         = 'OFF'
    'WITH_GDK_PIXBUF'       = 'OFF'
    'WITH_LIBDE265'         = 'ON'
    'WITH_X265'             = 'OFF'
    'WITH_DAV1D'            = 'OFF'
    'WITH_AOM_DECODER'      = 'OFF'
    'WITH_AOM_ENCODER'      = 'OFF'
    'LIBDE265_STATIC_BUILD' = '1'
    'LIBDE265_INCLUDE_DIR'  = "`"$libde265IncludeDir`""
    'LIBDE265_LIBRARY'      = "`"$libde265LibraryPath`""
}

try {
    Invoke-CMakeBuild `
        -LibraryName "libheif" `
        -SourceDir $libheifDir `
        -BuildDir $buildDir `
        -Configuration $Configuration `
        -CMakeOptions $cmakeOptions `
        -Clean:$Clean

    # Verify output
    $expectedLib = Join-Path $buildDir "libheif\$Configuration\heif.lib"
    if (-not (Test-Path $expectedLib)) {
        $expectedLib = Join-Path $buildDir "libheif\Release\heif.lib"
    }
    if (-not (Test-Path $expectedLib)) {
        $expectedLib = Join-Path $installDir "lib\heif.lib"
    }
    Test-BuildOutput -Files @($expectedLib) -ThrowOnMissing

    # Prepare packaged install artifacts
    $installIncludeDir = Join-Path $installDir "include\libheif"
    $installLibDir = Join-Path $installDir "lib"
    $installBinDir = Join-Path $installDir "bin"
    New-Item -ItemType Directory -Path $installIncludeDir -Force | Out-Null
    New-Item -ItemType Directory -Path $installLibDir -Force | Out-Null
    New-Item -ItemType Directory -Path $installBinDir -Force | Out-Null

    # Ensure heif public headers are packaged
    $heifHeaderSource = Join-Path $libheifDir "libheif\api\libheif"
    if (Test-Path $heifHeaderSource) {
        Copy-Item (Join-Path $heifHeaderSource "*.h") -Destination $installIncludeDir -Force -ErrorAction SilentlyContinue
    }

    # Ensure heif library is placed in install/lib
    Copy-Item $expectedLib -Destination (Join-Path $installLibDir "heif.lib") -Force

    # Package de265 import library and runtime DLL alongside heif
    if ($libde265LibraryPath -and (Test-Path $libde265LibraryPath)) {
        Copy-Item $libde265LibraryPath -Destination (Join-Path $installLibDir "de265.lib") -Force
    }

    $de265RuntimeDll = $de265RuntimeDllCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if ($de265RuntimeDll) {
        Copy-Item $de265RuntimeDll -Destination (Join-Path $installBinDir "libde265.dll") -Force
    }

    Write-BuildLog "libheif 1.19.5 build completed successfully" -Level Success
    Write-BuildLog "Features: HEIF/HEIC decoding (iPhone photos), HDR, burst photos" -Level Info
    Write-BuildLog "" -Level Info
    Write-BuildLog "Integration steps:" -Level Info
    Write-BuildLog "  1. Set -DHAS_LIBHEIF=ON in Engine CMake configuration" -Level Info
    Write-BuildLog "  2. Add libheif include/lib paths to Engine CMakeLists.txt" -Level Info
    Write-BuildLog "  3. Rebuild Engine with: cmake --build build --config Release" -Level Info

} catch {
    Write-BuildLog "Build failed: $($_.Exception.Message)" -Level Error
    exit 1
}
