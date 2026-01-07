# Build Pending Features for DarkThumbs v5.2.0+
# Builds libjxl and libarchive, then integrates them into the project
# PowerShell script for automated feature completion

param(
    [switch]$SkipLibjxl,
    [switch]$SkipLibarchive,
    [switch]$BuildOnly,
    [switch]$IntegrateOnly
)

$ErrorActionPreference = "Stop"

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "DarkThumbs - Pending Features Build" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

# Setup paths
    $ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir = Split-Path -Parent $ScriptDir
$ExternalDir = Join-Path $RootDir "external"
$ImageLibsDir = Join-Path $ExternalDir "image-libs"
$CompressionDir = Join-Path $ExternalDir "compression"# Build libjxl (JPEG XL)
if (-not $SkipLibjxl -and -not $IntegrateOnly) {
    Write-Host "[1/2] Building libjxl (JPEG XL Support)..." -ForegroundColor Yellow
    Write-Host "========================================" -ForegroundColor Yellow
    
    $LibjxlScript = Join-Path $ScriptDir "build-libjxl.ps1"
    if (Test-Path $LibjxlScript) {
        try {
            & $LibjxlScript
            Write-Host "[OK] libjxl build complete" -ForegroundColor Green
            Write-Host ""
        }
        catch {
            Write-Host "[ERROR] libjxl build failed: $($_.Exception.Message)" -ForegroundColor Red
            Write-Host "Continuing with other features..." -ForegroundColor Yellow
            Write-Host ""
        }
    }
    else {
        Write-Host "[WARNING] build-libjxl.ps1 not found, skipping" -ForegroundColor Yellow
        Write-Host ""
    }
}

# Build libarchive (TAR/ISO/CPIO support)
if (-not $SkipLibarchive -and -not $IntegrateOnly) {
    Write-Host "[2/2] Building libarchive (Archive Format Support)..." -ForegroundColor Yellow
    Write-Host "======================================================" -ForegroundColor Yellow
    
    # Check if libarchive source exists
    $LibarchiveDir = Join-Path $CompressionDir "libarchive-3.7.6"
    
    if (Test-Path $LibarchiveDir) {
        $BuildDir = Join-Path $LibarchiveDir "build-msvc"
        
        # Find CMake
        $CMake = Get-Command cmake -ErrorAction SilentlyContinue
        if (-not $CMake) {
            Write-Host "[ERROR] CMake not found in PATH" -ForegroundColor Red
            Write-Host "Please install CMake or add it to PATH" -ForegroundColor Yellow
        }
        else {
            # Create build directory
            if (Test-Path $BuildDir) {
                Write-Host "Cleaning existing build directory..." -ForegroundColor Yellow
                Remove-Item -Recurse -Force $BuildDir
            }
            New-Item -ItemType Directory -Path $BuildDir | Out-Null
            
            # Configure with CMake
            Write-Host "Configuring libarchive with CMake..." -ForegroundColor Yellow
            Push-Location $BuildDir
            
            try {
                & cmake .. `
                    -G "Ninja" `
                    -DCMAKE_BUILD_TYPE=Release `
                    -DBUILD_SHARED_LIBS=OFF `
                    -DENABLE_TEST=OFF `
                    -DENABLE_TAR=ON `
                    -DENABLE_CPIO=ON `
                    -DENABLE_CAT=OFF `
                    -DENABLE_INSTALL=OFF `
                    -DENABLE_WERROR=OFF
                
                if ($LASTEXITCODE -eq 0) {
                    Write-Host "[OK] CMake configuration complete" -ForegroundColor Green
                    
                    # Build
                    Write-Host "Building libarchive..." -ForegroundColor Yellow
                    & cmake --build . --config Release
                    
                    if ($LASTEXITCODE -eq 0) {
                        Write-Host "[OK] libarchive build complete" -ForegroundColor Green
                        
                        # Copy output to expected location
                        $OutputDir = Join-Path $LibarchiveDir "x64\Release"
                        New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
                        
                        $LibFile = Join-Path $BuildDir "libarchive\archive.lib"
                        if (Test-Path $LibFile) {
                            Copy-Item $LibFile (Join-Path $OutputDir "libarchive.lib") -Force
                            Write-Host "[OK] Copied archive.lib to $OutputDir" -ForegroundColor Green
                        }
                        else {
                            # Try alternative path
                            $LibFile = Join-Path $BuildDir "libarchive.a"
                            if (Test-Path $LibFile) {
                                Copy-Item $LibFile (Join-Path $OutputDir "libarchive.lib") -Force
                                Write-Host "[OK] Copied libarchive.a to $OutputDir" -ForegroundColor Green
                            }
                            else {
                                Write-Host "[WARNING] Could not find archive.lib/libarchive.a" -ForegroundColor Yellow
                                Write-Host "Searching for library files..." -ForegroundColor Yellow
                                Get-ChildItem $BuildDir -Recurse -Filter "*.lib" | ForEach-Object {
                                    Write-Host "  Found: $($_.FullName)" -ForegroundColor Cyan
                                }
                                Get-ChildItem $BuildDir -Recurse -Filter "*.a" | ForEach-Object {
                                    Write-Host "  Found: $($_.FullName)" -ForegroundColor Cyan
                                }
                            }
                        }
                    }
                    else {
                        Write-Host "[ERROR] libarchive build failed" -ForegroundColor Red
                    }
                }
                else {
                    Write-Host "[ERROR] CMake configuration failed" -ForegroundColor Red
                }
            }
            catch {
                Write-Host "[ERROR] libarchive build failed: $($_.Exception.Message)" -ForegroundColor Red
            }
            finally {
                Pop-Location
            }
        }
        Write-Host ""
    }
    else {
        Write-Host "[WARNING] libarchive source not found at: $LibarchiveDir" -ForegroundColor Yellow
        Write-Host "Skipping libarchive build" -ForegroundColor Yellow
        Write-Host ""
    }
}

# Integration step
if (-not $BuildOnly) {
    Write-Host "==========================================" -ForegroundColor Cyan
    Write-Host "Integration Check" -ForegroundColor Cyan
    Write-Host "==========================================" -ForegroundColor Cyan
    Write-Host ""
    
    # Check which libraries were built
    $LibjxlBuilt = $false
    $LibarchiveBuilt = $false
    
    # Check libjxl
    $LibjxlLib = Join-Path $ImageLibsDir "libjxl-0.11.1\x64\Release\libjxl.lib"
    if (Test-Path $LibjxlLib) {
        $LibjxlBuilt = $true
        $Size = (Get-Item $LibjxlLib).Length
        Write-Host "[OK] libjxl.lib found ($([math]::Round($Size/1MB, 2)) MB)" -ForegroundColor Green
    }
    else {
        Write-Host "[INFO] libjxl.lib not found - feature will remain disabled" -ForegroundColor Yellow
    }
    
    # Check libarchive
    $LibarchiveLib = Join-Path $CompressionDir "libarchive-3.7.6\x64\Release\libarchive.lib"
    if (Test-Path $LibarchiveLib) {
        $LibarchiveBuilt = $true
        $Size = (Get-Item $LibarchiveLib).Length
        Write-Host "[OK] libarchive.lib found ($([math]::Round($Size/1MB, 2)) MB)" -ForegroundColor Green
    }
    else {
        Write-Host "[INFO] libarchive.lib not found - feature will remain disabled" -ForegroundColor Yellow
    }
    
    Write-Host ""
    
    # Offer to update vcxproj if libraries are built
    if ($LibjxlBuilt -or $LibarchiveBuilt) {
        Write-Host "Library Integration Status:" -ForegroundColor Cyan
        Write-Host "  - JPEG XL (libjxl): $(if($LibjxlBuilt){'READY'}else{'NOT BUILT'})" -ForegroundColor $(if($LibjxlBuilt){'Green'}else{'Yellow'})
        Write-Host "  - Archive Formats (libarchive): $(if($LibarchiveBuilt){'READY'}else{'NOT BUILT'})" -ForegroundColor $(if($LibarchiveBuilt){'Green'}else{'Yellow'})
        Write-Host ""
        Write-Host "To integrate these libraries:" -ForegroundColor Yellow
        Write-Host "  1. Edit CBXShell\CBXShell.vcxproj" -ForegroundColor White
        if ($LibjxlBuilt) {
            Write-Host "     - Uncomment ENABLE_JXL_SUPPORT in PreprocessorDefinitions" -ForegroundColor White
            Write-Host "     - Uncomment jxl_decoder.cpp in ClCompile section" -ForegroundColor White
            Write-Host "     - Add ..\external\image-libs\libjxl-0.11.1\x64\Release to LibraryPath" -ForegroundColor White
            Write-Host "     - Add libjxl.lib to AdditionalDependencies" -ForegroundColor White
        }
        if ($LibarchiveBuilt) {
            Write-Host "     - Uncomment ENABLE_LIBARCHIVE_SUPPORT in PreprocessorDefinitions" -ForegroundColor White
            Write-Host "     - Uncomment libarchive_wrapper.cpp in ClCompile section" -ForegroundColor White
            Write-Host "     - Add ..\external\compression\libarchive-3.7.6\x64\Release to LibraryPath" -ForegroundColor White
            Write-Host "     - Add libarchive.lib to AdditionalDependencies" -ForegroundColor White
        }
        Write-Host "  2. Rebuild CBXShell solution" -ForegroundColor White
        Write-Host "  3. Test new format support" -ForegroundColor White
        Write-Host ""
        Write-Host "Or run: .\build-scripts\integrate-pending-features.ps1" -ForegroundColor Cyan
    }
    else {
        Write-Host "[INFO] No new libraries were built" -ForegroundColor Yellow
        Write-Host "Both libraries require successful CMake builds to integrate" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "Build Complete" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

if (-not $BuildOnly) {
    Write-Host "Next steps:" -ForegroundColor Yellow
    Write-Host "  1. Review build outputs above" -ForegroundColor White
    Write-Host "  2. Run integration script if libraries were built" -ForegroundColor White
    Write-Host "  3. Rebuild DarkThumbs solution" -ForegroundColor White
    Write-Host "  4. Test new formats" -ForegroundColor White
}
