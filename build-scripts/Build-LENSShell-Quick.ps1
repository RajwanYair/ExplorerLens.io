# Quick build script for LENSShell.dll
$ErrorActionPreference = "Continue"
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ProjectRoot

Write-Host "Building LENSShell.dll..." -ForegroundColor Cyan

$msbuild = "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe"
$project = "LENSShell\LENSShell.vcxproj"

& $msbuild $project /t:Build /p:Configuration=Release /p:Platform=x64 /v:minimal /nologo

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "[SUCCESS] Build completed" -ForegroundColor Green
    $dll = "x64\Release\LENSShell.dll"
    if (Test-Path $dll) {
        $size = (Get-Item $dll).Length
        Write-Host "LENSShell.dll: $size bytes" -ForegroundColor Green
    } else {
        Write-Host "[WARNING] LENSShell.dll not found in x64\Release\" -ForegroundColor Yellow
    }
} else {
    Write-Host ""
    Write-Host "[FAILED] Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
}

