# Setup MSVC Environment for PowerShell
# Automatically finds and configures the latest Visual Studio Build Tools

$ErrorActionPreference = "Stop"

# Find the latest MSVC version
$VSBasePath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Tools\MSVC"
$LatestMSVC = Get-ChildItem -Path $VSBasePath | Sort-Object Name -Descending | Select-Object -First 1

if (-not $LatestMSVC) {
    Write-Error "Visual Studio Build Tools not found"
    exit 1
}

Write-Host "Found MSVC version: $($LatestMSVC.Name)" -ForegroundColor Green

# Set up paths for x64 build
$MSVCPath = $LatestMSVC.FullName
$BinPath = Join-Path $MSVCPath "bin\Hostx64\x64"
$LibPath = Join-Path $MSVCPath "lib\x64"
$IncludePath = Join-Path $MSVCPath "include"

# Windows SDK paths
$SDKBase = "C:\Program Files (x86)\Windows Kits\10"
$SDKBin = Join-Path $SDKBase "bin\10.0.22621.0\x64"
$SDKInclude = Join-Path $SDKBase "Include\10.0.22621.0"
$SDKLib = Join-Path $SDKBase "Lib\10.0.22621.0"

# Update PATH
$env:PATH = "$BinPath;$SDKBin;$env:PATH"

# Update INCLUDE
$env:INCLUDE = "$IncludePath;$SDKInclude\ucrt;$SDKInclude\um;$SDKInclude\shared"

# Update LIB
$env:LIB = "$LibPath;$SDKLib\ucrt\x64;$SDKLib\um\x64"

# Set Visual Studio environment
$env:VSINSTALLDIR = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\"
$env:VCINSTALLDIR = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\"
$env:VCToolsVersion = $LatestMSVC.Name
$env:WindowsSdkDir = $SDKBase
$env:WindowsSDKVersion = "10.0.22621.0\"

# Platform and configuration
$env:Platform = "x64"
$env:Configuration = "Release"

Write-Host "`nMSVC Environment configured:" -ForegroundColor Cyan
Write-Host "  MSVC Version: $($LatestMSVC.Name)" -ForegroundColor White
Write-Host "  Compiler: $BinPath\cl.exe" -ForegroundColor White
Write-Host "  Platform: x64" -ForegroundColor White
Write-Host "`nYou can now use cl.exe, msbuild, and other build tools." -ForegroundColor Green
Write-Host "To make this permanent, add to your PowerShell profile:" -ForegroundColor Yellow
Write-Host "  notepad `$PROFILE" -ForegroundColor Yellow
Write-Host "  Add: . '$PSCommandPath'" -ForegroundColor Yellow

# Verify cl.exe is available
try {
    $clVersion = & cl.exe 2>&1 | Select-Object -First 1
    Write-Host "`n[OK] cl.exe is available: $clVersion" -ForegroundColor Green
} catch {
    Write-Warning "cl.exe found but may need additional setup"
}
