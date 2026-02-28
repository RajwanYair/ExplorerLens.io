# Build-LENSShell-MSBuild.ps1 — Build LENSShell DLL with MSBuild
param([switch]$Clean)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot

Write-Host "`n[LENSShell MSBuild]" -ForegroundColor Cyan

# Source vcvars
$vcvars = 'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat'
$vcvarsArgs = '-vcvars_ver=14.50.35717'

# Build command
$target = if ($Clean) { '/t:Rebuild' } else { '' }
$vcxproj = Join-Path $root 'LENSShell\LENSShell.vcxproj'

$buildCmd = "call `"$vcvars`" $vcvarsArgs >nul 2>&1 & msbuild `"$vcxproj`" /p:Configuration=Release /p:Platform=x64 $target /m /v:normal"

Write-Host "  Running: msbuild LENSShell.vcxproj /p:Configuration=Release /p:Platform=x64"
$output = cmd /c $buildCmd 2>&1
$exitCode = $LASTEXITCODE

$output | ForEach-Object { Write-Host $_ }

if ($exitCode -ne 0) {
    Write-Host "`n  Build FAILED (exit code: $exitCode)" -ForegroundColor Red
    exit 1
} else {
    Write-Host "`n  Build SUCCEEDED" -ForegroundColor Green
    exit 0
}
