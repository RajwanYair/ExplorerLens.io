# Build minizip-ng manually with MSVC and /MD runtime
param(
    [string]$ZlibDir = "..\..\zlib-1.3.1",
    [string]$ZlibBuildDir = "..\..\zlib-1.3.1\x64\Release",
    [string]$ZstdDir = "..\..\zstd-1.5.7\lib",
    [string]$BuildDir = "build-manual"
)

# Initialize MSVC environment
$vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
$tempBat = [System.IO.Path]::GetTempFileName() + ".bat"
$tempOut = [System.IO.Path]::GetTempFileName()

# Create batch file that sets up environment and outputs variables
@"
@echo off
call "$vcvarsPath" >nul 2>&1
set > "$tempOut"
"@ | Set-Content $tempBat

# Run batch and capture environment
cmd /c $tempBat
$envVars = Get-Content $tempOut
foreach ($line in $envVars) {
    if ($line -match '^([^=]+)=(.*)$') {
        [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process')
    }
}

Remove-Item $tempBat, $tempOut -ErrorAction SilentlyContinue

# Create build directory
if (Test-Path $BuildDir) {
    Remove-Item $BuildDir -Recurse -Force
}
New-Item -ItemType Directory -Path $BuildDir | Out-Null
Set-Location $BuildDir

Write-Host "Building minizip-ng with /MD runtime..." -ForegroundColor Cyan

# Core source files
$sourceFiles = @(
    "mz_crypt.c",
    "mz_os.c",
    "mz_strm.c",
    "mz_strm_buf.c",
    "mz_strm_mem.c",
    "mz_strm_split.c",
    "mz_strm_zlib.c",
    "mz_strm_zstd.c",
    "mz_zip.c",
    "mz_zip_rw.c",
    "mz_os_win32.c",
    "mz_strm_os_win32.c",
    "mz_crypt_winvista.c"
)

$objFiles = @()
$failed = $false

foreach ($file in $sourceFiles) {
    Write-Host "Compiling $file..." -ForegroundColor Yellow
    
    $result = & cl /c /MD /O2 /Ob2 /DNDEBUG /D_WINDOWS /DHAVE_ZLIB /DHAVE_ZSTD /DZLIB_COMPAT `
        /I"$ZlibDir" `
        /I"$ZlibBuildDir" `
        /I"$ZstdDir" `
        /I".." `
        "..\$file" 2>&1
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Failed to compile $file" -ForegroundColor Red
        Write-Host $result
        $failed = $true
        break
    }
    
    $objFile = [IO.Path]::GetFileNameWithoutExtension($file) + ".obj"
    if (Test-Path $objFile) {
        $objFiles += $objFile
    }
}

if (-not $failed -and $objFiles.Count -gt 0) {
    Write-Host "`nCreating static library..." -ForegroundColor Yellow
    
    $result = & lib /OUT:minizip.lib $objFiles 2>&1
    
    if ($LASTEXITCODE -eq 0 -and (Test-Path "minizip.lib")) {
        $size = (Get-Item "minizip.lib").Length / 1KB
        Write-Host "`nSUCCESS: minizip.lib created ($([math]::Round($size, 2)) KB)" -ForegroundColor Green
        Write-Host "Compiled $($objFiles.Count) source files" -ForegroundColor Green
    } else {
        Write-Host "`nFailed to create library" -ForegroundColor Red
        Write-Host $result
    }
} else {
    Write-Host "`nBuild failed - no object files created" -ForegroundColor Red
}

Set-Location ..
