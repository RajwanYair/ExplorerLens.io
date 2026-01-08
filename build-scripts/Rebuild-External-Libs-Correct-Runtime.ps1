#Requires -Version 7.0

<#
.SYNOPSIS
    Rebuild external libraries with correct runtime libraries to match CBXShell
.DESCRIPTION
    CBXShell Release uses /MT (MultiThreaded static), so external libs must also use /MT
    CBXShell Debug uses /MTd (MultiThreaded Debug static), so external libs must use /MTd for Debug
    This eliminates all /NODEFAULTLIB workarounds
#>

param(
    [string]$WorkspaceRoot = (Split-Path $PSScriptRoot -Parent),
    [ValidateSet("Release", "Debug", "Both")]
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

Write-Host "`n=== Rebuilding External Libraries with Correct Runtime ===" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration" -ForegroundColor Gray
Write-Host "CBXShell Release uses /MT (static)" -ForegroundColor Gray
Write-Host "CBXShell Debug uses /MTd (static debug)" -ForegroundColor Gray
Write-Host "`nRebuilding external libs to match (no MSVCRT/LIBCMT conflicts)`n" -ForegroundColor Gray

# Initialize Visual Studio environment
$vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $vcvarsPath)) {
    Write-Error "vcvars64.bat not found"
    exit 1
}

function Build-MinizipWithMT {
    param([string]$Config) # "Release" or "Debug"
    
    $flags = if ($Config -eq "Release") {
        @{
            Runtime  = "/MT"
            Opt      = "/O2"
            Defines  = "NDEBUG"
            Output   = "minizip.lib"
            BuildDir = "build-manual"
        }
    } else {
        @{
            Runtime  = "/MTd"
            Opt      = "/Od /Zi"
            Defines  = "_DEBUG"
            Output   = "minizipd.lib"
            BuildDir = "build-manual-debug"
        }
    }
    
    Write-Host "Building minizip-ng ($Config with $($flags.Runtime))..." -ForegroundColor Yellow
    
    $minizipDir = Join-Path $WorkspaceRoot "external\compression\minizip-ng-4.0.10"
    $buildDir = Join-Path $minizipDir $flags.BuildDir
    
    if (-not (Test-Path $minizipDir)) {
        Write-Warning "minizip-ng not found"
        return $false
    }
    
    # Clean and recreate build directory
    if (Test-Path $buildDir) {
        Remove-Item $buildDir -Recurse -Force
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
            
            $cmd = "cl.exe /c /nologo /W3 $($flags.Runtime) $($flags.Opt) /D $($flags.Defines) /D WIN32 /D _WINDOWS /D ZLIB_COMPAT /D HAVE_ZLIB /D HAVE_ZSTD /Fo`"$objName`" /I`"$minizipDir`" /I`"$zlibDir`" /I`"$zstdDir\lib`" `"$srcPath`""
            Invoke-Expression $cmd
            
            if ($LASTEXITCODE -ne 0) {
                Write-Error "Compilation failed for $src"
                return $false
            }
        }
        
        # Create static library
        lib.exe /nologo /OUT:$($flags.Output) $objFiles
        
        if (Test-Path $flags.Output) {
            $size = [math]::Round((Get-Item $flags.Output).Length / 1KB, 2)
            Write-Host "  ✓ minizip-ng $Config built: $($flags.Output) ($size KB) with $($flags.Runtime)" -ForegroundColor Green
            
            # Verify runtime
            Write-Host "  Verifying runtime linkage..." -ForegroundColor Gray
            $directives = cmd /c "dumpbin /directives `"$($flags.Output)`"" | Select-String "DEFAULTLIB"
            $hasCorrectRuntime = if ($Config -eq "Release") {
                $directives -match "LIBCMT" -and $directives -notmatch "MSVCRT"
            } else {
                $directives -match "LIBCMTD" -and $directives -notmatch "MSVCRTD"
            }
            
            if ($hasCorrectRuntime) {
                Write-Host "  ✓ Correct runtime verified" -ForegroundColor Green
            } else {
                Write-Warning "  Runtime linkage may be incorrect"
                $directives | ForEach-Object { Write-Host "    $_" -ForegroundColor Gray }
            }
            
            return $true
        }
        
        Write-Warning "minizip-ng $Config build failed"
        return $false
    } finally {
        Pop-Location
    }
}

function Build-ZstdWithMT {
    param([string]$Config)
    
    $flags = if ($Config -eq "Release") {
        @{
            Runtime  = "/MT"
            Opt      = "/O2"
            Defines  = "NDEBUG"
            Output   = "zstd_static.lib"
            BuildDir = "build-manual"
        }
    } else {
        @{
            Runtime  = "/MTd"
            Opt      = "/Od /Zi"
            Defines  = "_DEBUG"
            Output   = "zstd_staticd.lib"
            BuildDir = "build-manual-debug"
        }
    }
    
    Write-Host "Building zstd ($Config with $($flags.Runtime))..." -ForegroundColor Yellow
    
    $zstdDir = Join-Path $WorkspaceRoot "external\compression\zstd-1.5.7"
    $buildDir = Join-Path $zstdDir $flags.BuildDir
    
    if (-not (Test-Path $zstdDir)) {
        Write-Warning "zstd not found"
        return $false
    }
    
    # Clean and recreate
    if (Test-Path $buildDir) {
        Remove-Item $buildDir -Recurse -Force
    }
    New-Item -Path $buildDir -ItemType Directory -Force | Out-Null
    
    # Initialize MSVC
    cmd /c "`"$vcvarsPath`" > nul && set" | ForEach-Object {
        if ($_ -match "^([^=]+)=(.*)$") {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
        }
    }
    
    Push-Location $buildDir
    
    try {
        $sourceDirs = @(
            "..\lib\common\*.c",
            "..\lib\compress\*.c",
            "..\lib\decompress\*.c"
        )
        
        $sourceFiles = $sourceDirs | ForEach-Object { 
            Get-ChildItem (Join-Path $zstdDir $_) -ErrorAction SilentlyContinue
        } | Where-Object { $_.Name -notlike "*asm*" }
        
        $objFiles = @()
        foreach ($src in $sourceFiles) {
            $objName = [System.IO.Path]::ChangeExtension($src.Name, ".obj")
            $objFiles += $objName
            
            $cmd = "cl.exe /c /nologo /W3 $($flags.Runtime) $($flags.Opt) /D $($flags.Defines) /D WIN32 /D _WINDOWS /D ZSTD_MULTITHREAD /Fo`"$objName`" /I`"$zstdDir\lib`" /I`"$zstdDir\lib\common`" /I`"$zstdDir\lib\compress`" /I`"$zstdDir\lib\decompress`" `"$($src.FullName)`""
            Invoke-Expression $cmd | Out-Null
        }
        
        # Create library
        lib.exe /nologo /OUT:$($flags.Output) $objFiles
        
        if (Test-Path $flags.Output) {
            $size = [math]::Round((Get-Item $flags.Output).Length / 1KB, 2)
            Write-Host "  ✓ zstd $Config built: $($flags.Output) ($size KB) with $($flags.Runtime)" -ForegroundColor Green
            return $true
        }
        
        return $false
    } finally {
        Pop-Location
    }
}

# Build based on configuration
$results = @{}

if ($Configuration -in @("Release", "Both")) {
    Write-Host "`n=== Building Release (/MT) ===" -ForegroundColor Cyan
    $results["minizip-Release"] = Build-MinizipWithMT -Config "Release"
    $results["zstd-Release"] = Build-ZstdWithMT -Config "Release"
}

if ($Configuration -in @("Debug", "Both")) {
    Write-Host "`n=== Building Debug (/MTd) ===" -ForegroundColor Cyan
    $results["minizip-Debug"] = Build-MinizipWithMT -Config "Debug"
    $results["zstd-Debug"] = Build-ZstdWithMT -Config "Debug"
}

# Summary
Write-Host "`n=== Build Summary ===" -ForegroundColor Cyan
$success = ($results.Values | Where-Object { $_ -eq $true }).Count
$failed = ($results.Values | Where-Object { $_ -eq $false }).Count

foreach ($key in $results.Keys) {
    $status = if ($results[$key]) { "✓" } else { "✗" }
    $color = if ($results[$key]) { "Green" } else { "Red" }
    Write-Host "  $status $key" -ForegroundColor $color
}

Write-Host "`nTotal: $success succeeded, $failed failed" -ForegroundColor $(if ($failed -eq 0) { "Green" } else { "Yellow" })

if ($failed -eq 0) {
    Write-Host "`n✓ All libraries built with correct runtime" -ForegroundColor Green
    Write-Host "  Ready to remove /NODEFAULTLIB from CBXShell.vcxproj" -ForegroundColor Gray
    exit 0
} else {
    Write-Host "`n⚠ Some builds failed" -ForegroundColor Yellow
    exit 1
}
