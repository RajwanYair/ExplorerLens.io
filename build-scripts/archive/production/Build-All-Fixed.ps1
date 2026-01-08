#Requires -Version 7.0
<#
.SYNOPSIS
    Fixed build script for all DarkThumbs libraries
.DESCRIPTION
    Builds all required libraries with proper error handling and verification
#>

$ErrorActionPreference = "Continue"  # Don't stop on errors, report them all
$startTime = Get-Date

# Create logs directory
$logDir = "build-logs"
if (-not (Test-Path $logDir)) {
    New-Item -Path $logDir -ItemType Directory -Force | Out-Null
}

$logFile = "$logDir\fixed-build-$(Get-Date -Format 'yyyy-MM-dd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Color = "White")
    $timestamp = Get-Date -Format "HH:mm:ss"
    $logMessage = "[$timestamp] $Message"
    Write-Host $logMessage -ForegroundColor $Color
    Add-Content -Path $logFile -Value $logMessage -ErrorAction SilentlyContinue
}

function Test-Library {
    param([string]$Path, [string]$Name)
    
    if (Test-Path $Path) {
        $size = (Get-Item $Path).Length
        $sizeKB = [math]::Round($size / 1KB, 1)
        Write-Log "  ✅ $Name : $sizeKB KB" "Green"
        return $true
    } else {
        Write-Log "  ❌ $Name : NOT FOUND" "Red"
        Write-Log "     Expected: $Path" "Gray"
        return $false
    }
}

Write-Log "================================" "Cyan"
Write-Log " DarkThumbs Library Build" "Cyan"
Write-Log "================================" "Cyan"
Write-Log "Started: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" "Gray"
Write-Log "Log file: $logFile" "Gray"
Write-Log ""

# ============================================================================
# 1. ZLIB - Already built successfully
# ============================================================================
Write-Log "[1/6] ZLIB 1.3.1" "Cyan"
$zlibLib = "external\compression\zlib-1.3.1\x64\Release\zlibstatic.lib"
$zlibOk = Test-Library $zlibLib "zlibstatic.lib"

# ============================================================================
# 2. LZ4 - Already built successfully
# ============================================================================
Write-Log "`n[2/6] LZ4 1.10.0" "Cyan"
$lz4Lib = "external\compression\lz4-1.10.0\build-vs\Release\liblz4_static.lib"
$lz4Ok = Test-Library $lz4Lib "liblz4_static.lib"

# ============================================================================
# 3. ZSTD - Needs to be built with different method
# ============================================================================
Write-Log "`n[3/6] ZSTD 1.5.7" "Cyan"
$zstdLib = "external\compression\zstd-1.5.7\lib\libzstd.a"
Write-Log "  ⚠️  zstd source is missing CMake files" "Yellow"
Write-Log "  SKIP: Will use Makefile or prebuilt library" "Yellow"
$zstdOk = $false

# ============================================================================
# 4. LIBLZMA - Check if already built
# ============================================================================
Write-Log "`n[4/6] LIBLZMA (XZ 5.6.3)" "Cyan"
Write-Log "  Building liblzma..." "White"

try {
    Push-Location "external\compression\xz-5.6.3"
    
    $buildDir = "build-vs"
    if (-not (Test-Path $buildDir)) {
        New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
    }
    
    Push-Location $buildDir
    
    $vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    
    # Run CMake and build
    $cmakeCmd = @"
call "$vcvarsPath" && cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS_RELEASE="/MT /O2 /DNDEBUG" -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2 /DNDEBUG" -DBUILD_SHARED_LIBS=OFF -DENABLE_NLS=OFF && nmake
"@
    
    Write-Log "  Configuring and building with CMake..." "Gray"
    cmd /c $cmakeCmd 2>&1 | Out-File -FilePath "$logFile" -Append
    
    Pop-Location
    Pop-Location
    
    # Check for output
    $lzmaLib = "external\compression\xz-5.6.3\build-vs\lzma.lib"
    $lzmaOk = Test-Library $lzmaLib "lzma.lib"
    
} catch {
    Write-Log "  ❌ Build failed: $($_.Exception.Message)" "Red"
    Pop-Location -ErrorAction SilentlyContinue
    Pop-Location -ErrorAction SilentlyContinue
    $lzmaOk = $false
}

# ============================================================================
# 5. LIBWEBP - Build with nmake
# ============================================================================
Write-Log "`n[5/6] LIBWEBP 1.5.0" "Cyan"
Write-Log "  Building with nmake..." "White"

try {
    Push-Location "external\image-libs\libwebp-1.5.0"
    
    # Clean previous output
    if (Test-Path "output") {
        Remove-Item "output" -Recurse -Force -ErrorAction SilentlyContinue
    }
    
    $vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    
    # Build using nmake
    $buildCmd = @"
call "$vcvarsPath" x64 && nmake /f Makefile.vc CFG=release-static RTLIBCFG=static OBJDIR=output\x64\Release all
"@
    
    Write-Log "  Running nmake..." "Gray"
    $buildOutput = cmd /c $buildCmd 2>&1
    $buildOutput | Out-File -FilePath "$logFile" -Append
    
    # Show last 5 lines of output
    $buildOutput | Select-Object -Last 5 | ForEach-Object { Write-Log "    $_" "Gray" }
    
    Pop-Location
    
    # Check for output libraries
    Write-Log "  Checking output libraries:" "White"
    $webpLibs = @(
        "external\image-libs\libwebp-1.5.0\output\x64\Release\release-static\x64\lib\libwebp.lib",
        "external\image-libs\libwebp-1.5.0\output\x64\Release\release-static\x64\lib\libwebpdecoder.lib",
        "external\image-libs\libwebp-1.5.0\output\x64\Release\release-static\x64\lib\libsharpyuv.lib"
    )
    
    $webpOk = $true
    foreach ($lib in $webpLibs) {
        $libName = Split-Path $lib -Leaf
        if (Test-Path $lib) {
            $size = (Get-Item $lib).Length
            $sizeKB = [math]::Round($size / 1KB, 1)
            Write-Log "    ✅ $libName : $sizeKB KB" "Green"
        } else {
            Write-Log "    ❌ $libName : NOT FOUND" "Red"
            $webpOk = $false
        }
    }
    
} catch {
    Write-Log "  ❌ Build failed: $($_.Exception.Message)" "Red"
    Pop-Location -ErrorAction SilentlyContinue
    $webpOk = $false
}

# ============================================================================
# 6. MINIZIP-NG - Build with CMake
# ============================================================================
Write-Log "`n[6/6] MINIZIP-NG 4.0.10" "Cyan"
Write-Log "  Building with CMake..." "White"

try {
    $minizipDir = "external\compression\minizip-ng-4.0.10"
    
    if (-not (Test-Path $minizipDir)) {
        Write-Log "  ❌ Source directory not found" "Red"
        $minizipOk = $false
    } else {
        Push-Location $minizipDir
        
        $buildDir = "build-vs"
        if (Test-Path $buildDir) {
            Remove-Item $buildDir -Recurse -Force -ErrorAction SilentlyContinue
        }
        New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
        
        Push-Location $buildDir
        
        $vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
        
        # Set dependency paths
        $zlibPath = (Resolve-Path "..\..\zlib-1.3.1").Path
        $lzmaPath = (Resolve-Path "..\..\xz-5.6.3").Path
        $zstdPath = (Resolve-Path "..\..\zstd-1.5.7").Path
        
        # Build with CMake
        $cmakeCmd = @"
call "$vcvarsPath" && cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DMZ_ZLIB=ON -DMZ_LZMA=ON -DMZ_ZSTD=OFF -DZLIB_ROOT="$zlibPath" -DLIBLZMA_ROOT="$lzmaPath" -DCMAKE_C_FLAGS_RELEASE="/MT /O2" && nmake
"@
        
        Write-Log "  Configuring with CMake..." "Gray"
        cmd /c $cmakeCmd 2>&1 | Out-File -FilePath "$logFile" -Append
        
        Pop-Location
        Pop-Location
        
        # Check for output
        $minizipLib = "$minizipDir\build-vs\libminizip.lib"
        $minizipOk = Test-Library $minizipLib "libminizip.lib"
    }
    
} catch {
    Write-Log "  ❌ Build failed: $($_.Exception.Message)" "Red"
    Pop-Location -ErrorAction SilentlyContinue
    Pop-Location -ErrorAction SilentlyContinue
    $minizipOk = $false
}

# ============================================================================
# BUILD SUMMARY
# ============================================================================
$endTime = Get-Date
$duration = $endTime - $startTime

Write-Log "`n================================" "Cyan"
Write-Log " BUILD SUMMARY" "Cyan"
Write-Log "================================" "Cyan"

$results = @(
    @{Name = "ZLIB 1.3.1"; Status = $zlibOk },
    @{Name = "LZ4 1.10.0"; Status = $lz4Ok },
    @{Name = "ZSTD 1.5.7"; Status = $zstdOk },
    @{Name = "LIBLZMA"; Status = $lzmaOk },
    @{Name = "LIBWEBP 1.5.0"; Status = $webpOk },
    @{Name = "MINIZIP-NG 4.0.10"; Status = $minizipOk }
)

foreach ($result in $results) {
    if ($result.Status) {
        Write-Log "  ✅ $($result.Name)" "Green"
    } else {
        Write-Log "  ❌ $($result.Name)" "Red"
    }
}

Write-Log "`nDuration: $($duration.ToString('mm\:ss'))" "Gray"
Write-Log "Completed: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" "Gray"
Write-Log "Full log: $logFile" "Gray"

$successCount = ($results | Where-Object { $_.Status }).Count
$totalCount = $results.Count

if ($successCount -eq $totalCount) {
    Write-Log "`n🎉 ALL LIBRARIES BUILT SUCCESSFULLY!" "Green"
    exit 0
} else {
    Write-Log "`n⚠️  $successCount/$totalCount libraries built successfully" "Yellow"
    Write-Log "Check log file for details: $logFile" "Gray"
    exit 1
}
