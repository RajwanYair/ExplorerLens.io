# ExplorerLens Dev Container — Setup Script
# Installs required tools in a Windows devcontainer environment.
# Called by devcontainer.json onCreateCommand.

$ErrorActionPreference = 'Stop'

Write-Host '=== ExplorerLens Dev Container Setup ===' -ForegroundColor Cyan

# ── 1. Install Scoop (if not present) ─────────────────────────────────────────
if (-not (Get-Command scoop -ErrorAction SilentlyContinue)) {
    Write-Host '[1/5] Installing Scoop...' -ForegroundColor Yellow
    Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser -Force
    Invoke-RestMethod -Uri https://get.scoop.sh | Invoke-Expression
} else {
    Write-Host '[1/5] Scoop already installed.' -ForegroundColor Green
}

# ── 2. Install Scoop extras bucket ────────────────────────────────────────────
Write-Host '[2/5] Adding Scoop buckets...' -ForegroundColor Yellow
scoop bucket add extras 2>$null
scoop bucket add versions 2>$null

# ── 3. Install build tools via Scoop ──────────────────────────────────────────
Write-Host '[3/5] Installing build tools...' -ForegroundColor Yellow
$tools = @('cmake', 'ninja', 'nasm', 'git', '7zip', 'nuget', 'meson')
foreach ($tool in $tools) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        scoop install $tool
    } else {
        Write-Host "  $tool already installed." -ForegroundColor Gray
    }
}

# ── 4. Install Visual Studio BuildTools (MSVC v143 for container) ─────────────
Write-Host '[4/5] Installing VS 2022 BuildTools...' -ForegroundColor Yellow
$vsInstaller = 'https://aka.ms/vs/17/release/vs_buildtools.exe'
$vsArgs = @(
    '--quiet', '--wait', '--norestart',
    '--add', 'Microsoft.VisualStudio.Workload.VCTools',
    '--add', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
    '--add', 'Microsoft.VisualStudio.Component.Windows11SDK.26100',
    '--add', 'Microsoft.VisualStudio.Component.VC.CMake.Project'
)
$tempInstaller = Join-Path $env:TEMP 'vs_buildtools.exe'
Invoke-WebRequest -Uri $vsInstaller -OutFile $tempInstaller
Start-Process -FilePath $tempInstaller -ArgumentList $vsArgs -Wait
Remove-Item $tempInstaller -Force -ErrorAction SilentlyContinue

# ── 5. Verify setup ───────────────────────────────────────────────────────────
Write-Host '[5/5] Verifying tools...' -ForegroundColor Yellow
$checks = @{
    'cmake'  = { cmake --version 2>&1 | Select-String 'cmake version' }
    'ninja'  = { ninja --version 2>&1 }
    'nasm'   = { nasm --version 2>&1 | Select-String 'version' }
    'git'    = { git --version 2>&1 }
    '7z'     = { 7z --version 2>&1 | Select-String '7-Zip' }
}
$allOk = $true
foreach ($name in $checks.Keys) {
    $result = & $checks[$name]
    if ($result) {
        Write-Host "  ✓ $name : $result" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $name : NOT FOUND" -ForegroundColor Red
        $allOk = $false
    }
}

if ($allOk) {
    Write-Host ''
    Write-Host '=== Dev container setup complete ===' -ForegroundColor Green
    Write-Host 'Build with: .\build-scripts\Build-MSVC.ps1' -ForegroundColor Cyan
} else {
    Write-Host ''
    Write-Warning 'Some tools failed to install. Check output above.'
    exit 1
}
