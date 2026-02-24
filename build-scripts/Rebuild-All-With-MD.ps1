# ============================================================================
# Rebuild-All-With-MD.ps1
# Rebuilds ALL external libraries with /MD (dynamic CRT) flag
# This eliminates LIBCMT conflicts permanently
# ============================================================================

[CmdletBinding()]
param(
    [Parameter(Mandatory = $false)]
    [switch]$DryRun,
    
    [Parameter(Mandatory = $false)]
    [switch]$SkipBuild,
    
    [Parameter(Mandatory = $false)]
    [string[]]$OnlyLibraries
)

$ErrorActionPreference = "Stop"

# ============================================================================
# Configuration
# ============================================================================

$ScriptRoot = $PSScriptRoot
$ExternalRoot = Join-Path $ScriptRoot "..\external"
$SDKRoot = Join-Path $ScriptRoot "..\SDK"
$BuildLogDir = Join-Path $ScriptRoot "..\build-logs"

# Ensure log directory exists
New-Item -ItemType Directory -Path $BuildLogDir -Force | Out-Null

# ============================================================================
# Library Definitions
# ============================================================================

$Libraries = @(
    @{
        Name         = "zlib"
        Version      = "1.3.1"
        SourceDir    = "compression-libs\zlib-1.3.1"
        BuildSystem  = "CMake"
        CMakeOptions = @(
            "-DCMAKE_BUILD_TYPE=Release"
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL"
            "-DBUILD_SHARED_LIBS=ON"
        )
        BuildDir     = "build-md"
        Outputs      = @("zlib.lib")
    },
    @{
        Name         = "zstd"
        Version      = "1.5.7"
        SourceDir    = "compression-libs\zstd-1.5.7\build\cmake"
        BuildSystem  = "CMake"
        CMakeOptions = @(
            "-DCMAKE_BUILD_TYPE=Release"
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL"
            "-DBUILD_SHARED_LIBS=ON"
            "-DZSTD_BUILD_PROGRAMS=ON"
            "-DZSTD_BUILD_TESTS=ON"
        )
        BuildDir     = "build-md"
        Outputs      = @("zstd.lib")
    },
    @{
        Name         = "lz4"
        Version      = "1.10.0"
        SourceDir    = "compression-libs\lz4-1.10.0\build\cmake"
        BuildSystem  = "CMake"
        CMakeOptions = @(
            "-DCMAKE_BUILD_TYPE=Release"
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL"
            "-DBUILD_SHARED_LIBS=ON"
            "-DLZ4_BUILD_CLI=ON"
            "-DLZ4_BUILD_LEGACY_LZ4C=ON"
        )
        BuildDir     = "build-md"
        Outputs      = @("lz4.lib")
    },
    @{
        Name         = "minizip-ng"
        Version      = "4.0.10"
        SourceDir    = "compression-libs\minizip-ng-4.0.10"
        BuildSystem  = "CMake"
        CMakeOptions = @(
            "-DCMAKE_BUILD_TYPE=Release"
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL"
            "-DBUILD_SHARED_LIBS=ON"
            "-DMZ_COMPAT=ON"
            "-DMZ_ZLIB=ON"
            "-DMZ_BZIP2=ON"
            "-DMZ_LZMA=ON"
            "-DMZ_ZSTD=ON"
        )
        BuildDir     = "build-md"
        Outputs      = @("minizip4.lib")
    },
    @{
        Name         = "bzip2"
        Version      = "1.0.8"
        SourceDir    = "compression-libs\bzip2-1.0.8"
        BuildSystem  = "CMake"
        CMakeOptions = @(
            "-DCMAKE_BUILD_TYPE=Release"
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL"
            "-DBUILD_SHARED_LIBS=ON"
        )
        BuildDir     = "build-md"
        Outputs      = @("bzip2.lib")
    },
    @{
        Name         = "xz"
        Version      = "5.6.3"
        SourceDir    = "compression-libs\xz-5.6.3"
        BuildSystem  = "CMake"
        CMakeOptions = @(
            "-DCMAKE_BUILD_TYPE=Release"
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL"
            "-DBUILD_SHARED_LIBS=ON"
        )
        BuildDir     = "build-md"
        Outputs      = @("liblzma.lib")
    },
    @{
        Name         = "libwebp"
        Version      = "1.5.0"
        SourceDir    = "image-libs\libwebp-1.5.0-original"
        BuildSystem  = "CMake"
        CMakeOptions = @(
            "-DCMAKE_BUILD_TYPE=Release"
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL"
            "-DBUILD_SHARED_LIBS=ON"
            "-DWEBP_BUILD_ANIM_UTILS=ON"
            "-DWEBP_BUILD_CWEBP=ON"
            "-DWEBP_BUILD_DWEBP=ON"
            "-DWEBP_BUILD_IMG2WEBP=ON"
            "-DWEBP_BUILD_WEBPINFO=ON"
            "-DWEBP_BUILD_WEBPMUX=ON"
            "-DWEBP_BUILD_EXTRAS=ON"
        )
        BuildDir     = "build-md"
        Outputs      = @("webp.lib", "webpdecoder.lib", "webpdemux.lib", "sharpyuv.lib")
    },
    @{
        Name         = "dav1d"
        Version      = "1.5.1"
        SourceDir    = "image-libs\dav1d-1.5.1"
        BuildSystem  = "Meson"
        MesonOptions = @(
            "--buildtype=release"
            "-Ddefault_library=static"
            "-Db_vscrt=md"
        )
        BuildDir     = "build-md"
        Outputs      = @("dav1d.lib")
    },
    @{
        Name         = "libavif"
        Version      = "1.3.0"
        SourceDir    = "image-libs\libavif-1.3.0"
        BuildSystem  = "CMake"
        CMakeOptions = @(
            "-DCMAKE_BUILD_TYPE=Release"
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL"
            "-DBUILD_SHARED_LIBS=ON"
            "-DAVIF_CODEC_DAV1D=ON"
            "-DAVIF_BUILD_APPS=ON"
            "-DAVIF_BUILD_TESTS=ON"
        )
        BuildDir     = "build-md"
        Outputs      = @("avif.lib")
    }
)

# ============================================================================
# Helper Functions
# ============================================================================

function Write-Step {
    param([string]$Message)
    Write-Host "`n[$(Get-Date -Format 'HH:mm:ss')] $Message" -ForegroundColor Cyan
}

function Write-Success {
    param([string]$Message)
    Write-Host "  ✓ $Message" -ForegroundColor Green
}

function Write-Warning {
    param([string]$Message)
    Write-Host "  ⚠ $Message" -ForegroundColor Yellow
}

function Write-Error {
    param([string]$Message)
    Write-Host "  ✗ $Message" -ForegroundColor Red
}

function Test-CMake {
    $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmake) {
        throw "CMake not found in PATH. Please install CMake 3.20+"
    }
    $version = & cmake --version | Select-Object -First 1
    Write-Success "CMake found: $version"
}

function Test-Ninja {
    $ninja = Get-Command ninja -ErrorAction SilentlyContinue
    if (-not $ninja) {
        throw "Ninja not found in PATH. Please install Ninja build system"
    }
    Write-Success "Ninja found"
}

function Test-Meson {
    $meson = Get-Command meson -ErrorAction SilentlyContinue
    if (-not $meson) {
        Write-Warning "Meson not found - dav1d will be skipped"
        return $false
    }
    Write-Success "Meson found"
    return $true
}

function Build-CMakeLibrary {
    param(
        [hashtable]$Library,
        [string]$SourcePath,
        [string]$BuildPath
    )
    
    Write-Step "Building $($Library.Name) $($Library.Version) with CMake..."
    
    # Create build directory
    if (Test-Path $BuildPath) {
        Remove-Item -Path $BuildPath -Recurse -Force
    }
    New-Item -ItemType Directory -Path $BuildPath -Force | Out-Null
    
    # Configure
    Write-Host "  Configuring..." -ForegroundColor Gray
    $cmakeArgs = @(
        "-S", $SourcePath
        "-B", $BuildPath
        "-G", "Ninja"
    )
    $cmakeArgs += $Library.CMakeOptions
    
    if ($DryRun) {
        Write-Host "  [DRY RUN] cmake $($cmakeArgs -join ' ')" -ForegroundColor Yellow
    } else {
        $logFile = Join-Path $BuildLogDir "build_$($Library.Name)_configure.log"
        & cmake @cmakeArgs 2>&1 | Tee-Object -FilePath $logFile
        
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configuration failed for $($Library.Name)"
        }
    }
    
    # Build
    Write-Host "  Building..." -ForegroundColor Gray
    if ($DryRun) {
        Write-Host "  [DRY RUN] cmake --build $BuildPath --config Release" -ForegroundColor Yellow
    } else {
        $logFile = Join-Path $BuildLogDir "build_$($Library.Name)_build.log"
        & cmake --build $BuildPath --config Release 2>&1 | Tee-Object -FilePath $logFile
        
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed for $($Library.Name)"
        }
    }
    
    Write-Success "$($Library.Name) built successfully"
}

function Build-MesonLibrary {
    param(
        [hashtable]$Library,
        [string]$SourcePath,
        [string]$BuildPath
    )
    
    Write-Step "Building $($Library.Name) $($Library.Version) with Meson..."
    
    # Create build directory
    if (Test-Path $BuildPath) {
        Remove-Item -Path $BuildPath -Recurse -Force
    }
    
    # Setup
    Write-Host "  Setting up..." -ForegroundColor Gray
    $mesonArgs = @($BuildPath) + $Library.MesonOptions
    
    Push-Location $SourcePath
    try {
        if ($DryRun) {
            Write-Host "  [DRY RUN] meson setup $($mesonArgs -join ' ')" -ForegroundColor Yellow
        } else {
            $logFile = Join-Path $BuildLogDir "build_$($Library.Name)_setup.log"
            & meson setup @mesonArgs 2>&1 | Tee-Object -FilePath $logFile
            
            if ($LASTEXITCODE -ne 0) {
                throw "Meson setup failed for $($Library.Name)"
            }
        }
        
        # Compile
        Write-Host "  Compiling..." -ForegroundColor Gray
        if ($DryRun) {
            Write-Host "  [DRY RUN] meson compile -C $BuildPath" -ForegroundColor Yellow
        } else {
            $logFile = Join-Path $BuildLogDir "build_$($Library.Name)_compile.log"
            & meson compile -C $BuildPath 2>&1 | Tee-Object -FilePath $logFile
            
            if ($LASTEXITCODE -ne 0) {
                throw "Meson compile failed for $($Library.Name)"
            }
        }
    } finally {
        Pop-Location
    }
    
    Write-Success "$($Library.Name) built successfully"
}

function Install-Library {
    param(
        [hashtable]$Library,
        [string]$BuildPath
    )
    
    Write-Host "  Installing to SDK..." -ForegroundColor Gray
    
    # Find library files
    $libFiles = @()
    foreach ($output in $Library.Outputs) {
        $found = Get-ChildItem -Path $BuildPath -Filter $output -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($found) {
            $libFiles += $found
        } else {
            Write-Warning "Output file not found: $output"
        }
    }
    
    if ($libFiles.Count -eq 0) {
        throw "No output files found for $($Library.Name)"
    }
    
    # Copy to SDK
    $sdkLibDir = Join-Path $SDKRoot "lib"
    New-Item -ItemType Directory -Path $sdkLibDir -Force | Out-Null
    
    foreach ($libFile in $libFiles) {
        if ($DryRun) {
            Write-Host "  [DRY RUN] Copy $($libFile.FullName) -> $sdkLibDir" -ForegroundColor Yellow
        } else {
            Copy-Item -Path $libFile.FullName -Destination $sdkLibDir -Force
            Write-Success "Installed: $($libFile.Name)"
        }
    }
}

# ============================================================================
# Main Execution
# ============================================================================

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  Rebuild All Libraries with /MD" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# Verify tools
Write-Step "Verifying build tools..."
Test-CMake
Test-Ninja
$hasMeson = Test-Meson

# Filter libraries if requested
if ($OnlyLibraries) {
    $Libraries = $Libraries | Where-Object { $OnlyLibraries -contains $_.Name }
    Write-Host "`nBuilding only: $($OnlyLibraries -join ', ')" -ForegroundColor Yellow
}

# Build each library
$successCount = 0
$failedLibraries = @()

foreach ($lib in $Libraries) {
    try {
        # Skip Meson libraries if Meson not available
        if ($lib.BuildSystem -eq "Meson" -and -not $hasMeson) {
            Write-Warning "Skipping $($lib.Name) (Meson not available)"
            continue
        }
        
        $sourcePath = Join-Path $ExternalRoot $lib.SourceDir
        
        if (-not (Test-Path $sourcePath)) {
            Write-Warning "Source directory not found: $sourcePath"
            $failedLibraries += $lib.Name
            continue
        }
        
        $buildPath = Join-Path $sourcePath $lib.BuildDir
        
        # Build
        if ($lib.BuildSystem -eq "CMake") {
            Build-CMakeLibrary -Library $lib -SourcePath $sourcePath -BuildPath $buildPath
        } elseif ($lib.BuildSystem -eq "Meson") {
            Build-MesonLibrary -Library $lib -SourcePath $sourcePath -BuildPath $buildPath
        } else {
            Write-Warning "Unknown build system: $($lib.BuildSystem)"
            continue
        }
        
        # Install
        if (-not $SkipBuild) {
            Install-Library -Library $lib -BuildPath $buildPath
        }
        
        $successCount++
        
    } catch {
        Write-Error "Failed: $($lib.Name) - $_"
        $failedLibraries += $lib.Name
    }
}

# Summary
Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  Build Summary" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Total libraries: $($Libraries.Count)" -ForegroundColor White
Write-Host "Built successfully: $successCount" -ForegroundColor Green

if ($failedLibraries.Count -gt 0) {
    Write-Host "Failed: $($failedLibraries.Count)" -ForegroundColor Red
    foreach ($failed in $failedLibraries) {
        Write-Host "  - $failed" -ForegroundColor Red
    }
}

Write-Host ""

if ($failedLibraries.Count -eq 0) {
    Write-Host "✓ All libraries rebuilt successfully with /MD!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Cyan
    Write-Host "  1. Remove /NODEFAULTLIB:LIBCMT from Engine/CMakeLists.txt" -ForegroundColor Gray
    Write-Host "  2. Rebuild ExplorerLens Engine" -ForegroundColor Gray
    Write-Host "  3. Rebuild LENSShell solution" -ForegroundColor Gray
    Write-Host "  4. Verify clean build with zero warnings" -ForegroundColor Gray
    exit 0
} else {
    Write-Host "✗ Some libraries failed to build" -ForegroundColor Red
    exit 1
}

