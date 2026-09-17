# Build-LENSShell-MSBuild.ps1 — Build LENSShell DLL with MSBuild
param([switch]$Clean)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot

Write-Host "`n[LENSShell MSBuild]" -ForegroundColor Cyan

# Source vcvars
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path -LiteralPath $vswhere)) {
    throw 'Visual Studio Installer discovery tool (vswhere.exe) is missing.'
}
$vsRoot = & $vswhere -latest -products '*' -version '[18.0,19.0)' `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsRoot) {
    throw 'Install Visual Studio 2026 Build Tools with the C++ workload and MSVC v145.'
}
$vcvars = Join-Path $vsRoot 'VC\Auxiliary\Build\vcvars64.bat'
$toolset = Get-ChildItem -LiteralPath (Join-Path $vsRoot 'VC\Tools\MSVC') -Directory |
    Where-Object Name -Match '^14\.5\d\.' |
    Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1
if (-not $toolset -or -not (Test-Path -LiteralPath $vcvars)) {
    throw 'The Visual Studio 2026 MSVC v145 installation is incomplete.'
}
$vcvarsArgs = "-vcvars_ver=$($toolset.Name)"

# Build command
$target = if ($Clean) { '/t:Rebuild' } else { '' }
$vcxproj = Join-Path $root 'LENSShell\LENSShell.vcxproj'

$buildCmd = "call `"$vcvars`" $vcvarsArgs >nul 2>&1 && msbuild `"$vcxproj`" /p:Configuration=Release /p:Platform=x64 $target /m /v:normal"

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
