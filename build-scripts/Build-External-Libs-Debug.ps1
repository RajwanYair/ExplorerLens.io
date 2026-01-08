#Requires -Version 7.0

<#
.SYNOPSIS
    Build Debug versions of all external compression and image libraries with /MTd runtime
.DESCRIPTION
    Builds Debug configurations for zlib, lz4, zstd, minizip-ng, and libwebp
    Uses /MTd (Multi-threaded Debug static runtime) to match CBXShell Debug configuration
    This eliminates the need for runtime library ignores and warnings
#>

param(
    [string]$WorkspaceRoot = (Split-Path $PSScriptRoot -Parent)
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

Write-Host "`n=== Building Debug External Libraries with /MTd ===" -ForegroundColor Cyan
Write-Host "Workspace: $WorkspaceRoot`n" -ForegroundColor Gray

# Initialize Visual Studio environment
$vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $vcvarsPath)) {
    Write-Error "vcvars64.bat not found at: $vcvarsPath"
    exit 1
}

function Build-ZlibDebug {
    Write-Host "`n[1/5] Building zlib 1.3.1 (Debug /MTd)..." -ForegroundColor Yellow
    
    $zlibDir = Join-Path $WorkspaceRoot "external\compression\zlib-1.3.1"
    $buildDir = Join-Path $zlibDir "x64\Debug"
    
    if (-not (Test-Path $zlibDir)) {
        Write-Warning "zlib directory not found: $zlibDir"
        return $false
    }
    
    New-Item -Path $buildDir -ItemType Directory -Force | Out-Null
    Push-Location $buildDir
    
    try {
        # Configure with CMake for Debug /MTd
        cmake ..\.. -G "Visual Studio 18 2026" -A x64 `
            -DCMAKE_BUILD_TYPE=Debug `
            -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreadedDebug"
        
        # Build
        cmake --build . --config Debug --parallel
        
        if (Test-Path "Debug\zlibstaticd.lib") {
            Write-Host "  ✓ zlib Debug built: $(Get-Item 'Debug\zlibstaticd.lib' | ForEach-Object { [math]::Round($_.Length/1KB, 2) }) KB" -ForegroundColor Green
            return $true
        }
        
        Write-Warning "zlib Debug build failed"
        return $false
    } finally {
        Pop-Location
    }
}

function Build-Lz4Debug {
    Write-Host "`n[2/5] Building lz4 1.10.0 (Debug /MTd)..." -ForegroundColor Yellow
    
    $lz4Dir = Join-Path $WorkspaceRoot "external\compression\lz4-1.10.0"
    $buildDir = Join-Path $lz4Dir "build-vs\Debug"
    
    if (-not (Test-Path $lz4Dir)) {
        Write-Warning "lz4 directory not found: $lz4Dir"
        return $false
    }
    
    New-Item -Path $buildDir -ItemType Directory -Force | Out-Null
    Push-Location $buildDir
    
    try {
        cmake ..\..\build\cmake -G "Visual Studio 18 2026" -A x64 `
            -DCMAKE_BUILD_TYPE=Debug `
            -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreadedDebug" `
            -DLZ4_BUILD_CLI=OFF `
            -DLZ4_BUILD_LEGACY_LZ4C=OFF
        
        cmake --build . --config Debug --parallel
        
        if (Test-Path "Debug\lz4_static.lib") {
            Write-Host "  ✓ lz4 Debug built: $(Get-Item 'Debug\lz4_static.lib' | ForEach-Object { [math]::Round($_.Length/1KB, 2) }) KB" -ForegroundColor Green
            return $true
        }
        
        Write-Warning "lz4 Debug build failed"
        return $false
    } finally {
        Pop-Location
    }
}

function Build-ZstdDebug {
    Write-Host "`n[3/5] Building zstd 1.5.7 (Debug /MTd)..." -ForegroundColor Yellow
    
    $zstdDir = Join-Path $WorkspaceRoot "external\compression\zstd-1.5.7"
    $buildDir = Join-Path $zstdDir "build-manual-debug"
    
    if (-not (Test-Path $zstdDir)) {
        Write-Warning "zstd directory not found: $zstdDir"
        return $false
    }
    
    New-Item -Path $buildDir -ItemType Directory -Force | Out-Null
    
    # Initialize MSVC environment in PowerShell
    cmd /c "`"$vcvarsPath`" > nul && set" | ForEach-Object {
        if ($_ -match "^([^=]+)=(.*)$") {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
        }
    }
    
    Push-Location $buildDir
    
    try {
        # Compile all source files with /MTd
        $sources = @(
            "..\lib\common\*.c",
            "..\lib\compress\*.c",
            "..\lib\decompress\*.c"
        )
        
        $sourceFiles = $sources | ForEach-Object { 
            Get-ChildItem (Join-Path $zstdDir $_) -ErrorAction SilentlyContinue
        } | Where-Object { $_.Name -notlike "*asm*" }
        
        $objFiles = @()
        foreach ($src in $sourceFiles) {
            $objName = [System.IO.Path]::ChangeExtension($src.Name, ".obj")
            $objFiles += $objName
            
            cl.exe /c /nologo /W3 /WX- /diagnostics:column /Od /Ob0 /Oi /D _DEBUG /D WIN32 /D _WINDOWS /D ZSTD_MULTITHREAD /Gm- /EHsc /MTd /GS /Gy /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"$objName" /Fd"zstd_staticd.pdb" /external:W3 /Gd /TC /FC /errorReport:queue `
                "/I$zstdDir\lib" "/I$zstdDir\lib\common" "/I$zstdDir\lib\compress" "/I$zstdDir\lib\decompress" `
                $src.FullName
        }
        
        # Create static library
        lib.exe /nologo /OUT:zstd_staticd.lib $objFiles
        
        if (Test-Path "zstd_staticd.lib") {
            Write-Host "  ✓ zstd Debug built: $(Get-Item 'zstd_staticd.lib' | ForEach-Object { [math]::Round($_.Length/1KB, 2) }) KB" -ForegroundColor Green
            return $true
        }
        
        Write-Warning "zstd Debug build failed"
        return $false
    } finally {
        Pop-Location
    }
}

function Build-MinizipDebug {
    Write-Host "`n[4/5] Building minizip-ng 4.0.10 (Debug /MTd)..." -ForegroundColor Yellow
    
    $minizipDir = Join-Path $WorkspaceRoot "external\compression\minizip-ng-4.0.10"
    $buildDir = Join-Path $minizipDir "build-manual-debug"
    
    if (-not (Test-Path $minizipDir)) {
        Write-Warning "minizip-ng directory not found: $minizipDir"
        return $false
    }
    
    New-Item -Path $buildDir -ItemType Directory -Force | Out-Null
    
    # Initialize MSVC environment
    cmd /c "`"$vcvarsPath`" > nul && set" | ForEach-Object {
        if ($_ -match "^([^=]+)=(.*)$") {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
        }
    }
    
    Push-Location $buildDir
    
    try {
        $sources = @(
            "mz_crypt.c", "mz_os.c", "mz_strm.c", "mz_strm_buf.c",
            "mz_strm_mem.c", "mz_strm_split.c", "mz_zip.c", "mz_zip_rw.c",
            "mz_os_win32.c", "mz_strm_os_win32.c", "mz_strm_zlib.c",
            "mz_strm_pkcrypt.c", "mz_strm_wzaes.c"
        )
        
        $zlibDir = Join-Path $WorkspaceRoot "external\compression\zlib-1.3.1"
        $zstdDir = Join-Path $WorkspaceRoot "external\compression\zstd-1.5.7"
        
        $objFiles = @()
        foreach ($src in $sources) {
            $srcPath = Join-Path $minizipDir $src
            if (-not (Test-Path $srcPath)) { continue }
            
            $objName = [System.IO.Path]::ChangeExtension($src, ".obj")
            $objFiles += $objName
            
            cl.exe /c /nologo /W3 /WX- /diagnostics:column /Od /Ob0 /Oi /D _DEBUG /D WIN32 /D _WINDOWS /D ZLIB_COMPAT /D HAVE_ZLIB /D HAVE_ZSTD /Gm- /EHsc /MTd /GS /Gy /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"$objName" /Fd"minizipd.pdb" /external:W3 /Gd /TC /FC /errorReport:queue `
                "/I$minizipDir" "/I$zlibDir" "/I$zstdDir\lib" `
                $srcPath
        }
        
        # Create static library
        lib.exe /nologo /OUT:minizipd.lib $objFiles
        
        if (Test-Path "minizipd.lib") {
            Write-Host "  ✓ minizip-ng Debug built: $(Get-Item 'minizipd.lib' | ForEach-Object { [math]::Round($_.Length/1KB, 2) }) KB" -ForegroundColor Green
            return $true
        }
        
        Write-Warning "minizip-ng Debug build failed"
        return $false
    } finally {
        Pop-Location
    }
}

function Build-LibWebPDebug {
    Write-Host "`n[5/5] Building libwebp 1.5.0 (Debug /MTd)..." -ForegroundColor Yellow
    
    $webpDir = Join-Path $WorkspaceRoot "external\image-libs\libwebp-1.5.0"
    $buildDir = Join-Path $webpDir "build-vs\Debug"
    
    if (-not (Test-Path $webpDir)) {
        Write-Warning "libwebp directory not found: $webpDir"
        return $false
    }
    
    New-Item -Path $buildDir -ItemType Directory -Force | Out-Null
    Push-Location $buildDir
    
    try {
        cmake ..\.. -G "Visual Studio 18 2026" -A x64 `
            -DCMAKE_BUILD_TYPE=Debug `
            -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreadedDebug" `
            -DWEBP_BUILD_ANIM_UTILS=OFF `
            -DWEBP_BUILD_CWEBP=OFF `
            -DWEBP_BUILD_DWEBP=OFF `
            -DWEBP_BUILD_GIF2WEBP=OFF `
            -DWEBP_BUILD_IMG2WEBP=OFF `
            -DWEBP_BUILD_VWEBP=OFF `
            -DWEBP_BUILD_WEBPINFO=OFF `
            -DWEBP_BUILD_WEBPMUX=OFF `
            -DWEBP_BUILD_EXTRAS=OFF
        
        cmake --build . --config Debug --parallel
        
        if ((Test-Path "Debug\webp.lib") -and (Test-Path "Debug\sharpyuv.lib")) {
            $webpSize = Get-Item 'Debug\webp.lib' | ForEach-Object { [math]::Round($_.Length / 1KB, 2) }
            $sharpyuvSize = Get-Item 'Debug\sharpyuv.lib' | ForEach-Object { [math]::Round($_.Length / 1KB, 2) }
            Write-Host "  ✓ libwebp Debug built: webp.lib ($webpSize KB), sharpyuv.lib ($sharpyuvSize KB)" -ForegroundColor Green
            return $true
        }
        
        Write-Warning "libwebp Debug build failed"
        return $false
    } finally {
        Pop-Location
    }
}

# Build all libraries
$results = @{
    "zlib"       = Build-ZlibDebug
    "lz4"        = Build-Lz4Debug
    "zstd"       = Build-ZstdDebug
    "minizip-ng" = Build-MinizipDebug
    "libwebp"    = Build-LibWebPDebug
}

# Summary
Write-Host "`n=== Build Summary ===" -ForegroundColor Cyan
$success = 0
$failed = 0

foreach ($lib in $results.Keys) {
    if ($results[$lib]) {
        Write-Host "  ✓ $lib" -ForegroundColor Green
        $success++
    } else {
        Write-Host "  ✗ $lib" -ForegroundColor Red
        $failed++
    }
}

Write-Host "`nTotal: $success succeeded, $failed failed" -ForegroundColor $(if ($failed -eq 0) { "Green" } else { "Yellow" })

if ($failed -eq 0) {
    Write-Host "`n✓ All Debug libraries built successfully with /MTd runtime" -ForegroundColor Green
    Write-Host "  Ready to remove library ignores from CBXShell.vcxproj" -ForegroundColor Gray
} else {
    Write-Host "`n⚠ Some libraries failed to build" -ForegroundColor Yellow
    exit 1
}
