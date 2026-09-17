# ExplorerLens Dev Container — Setup Script
# Installs required tools in a Windows devcontainer environment.
# Called by devcontainer.json onCreateCommand.

$ErrorActionPreference = 'Stop'

Write-Host '=== ExplorerLens Dev Container Setup ===' -ForegroundColor Cyan

function Test-Administrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]::new($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

if (-not (Test-Administrator)) {
    throw 'Dev container setup must run as ContainerAdministrator so tools are installed machine-wide.'
}

function Install-WingetMachinePackage {
    param(
        [Parameter(Mandatory = $true)][string]$Id,
        [Parameter(Mandatory = $true)][string]$CommandName
    )

    if (Get-Command $CommandName -ErrorAction SilentlyContinue) {
        Write-Host "  $CommandName already installed." -ForegroundColor Gray
        return
    }

    Write-Host "  Installing $Id machine-wide..." -ForegroundColor Cyan
    & winget install --id $Id --exact --scope machine --silent --accept-source-agreements --accept-package-agreements
    if ($LASTEXITCODE -ne 0) {
        throw "winget failed to install $Id (exit code $LASTEXITCODE)"
    } else {
        Write-Host "  $CommandName installed machine-wide." -ForegroundColor Green
    }
}

# ── 1. Install build tools machine-wide ──────────────────────────────────────
Write-Host '[1/3] Installing build tools machine-wide...' -ForegroundColor Yellow
$tools = @(
    @{ Id = 'Kitware.CMake'; Command = 'cmake' },
    @{ Id = 'Ninja-build.Ninja'; Command = 'ninja' },
    @{ Id = 'NASM.NASM'; Command = 'nasm' },
    @{ Id = 'Git.Git'; Command = 'git' },
    @{ Id = '7zip.7zip'; Command = '7z' },
    @{ Id = 'Microsoft.NuGet'; Command = 'nuget' },
    @{ Id = 'MesonBuild.Meson'; Command = 'meson' }
)
if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
    throw 'winget is required to install container tools machine-wide.'
}
foreach ($tool in $tools) {
    Install-WingetMachinePackage -Id $tool.Id -CommandName $tool.Command
}

# ── 2. Install Visual Studio BuildTools (MSVC v145) ──────────────────────────
Write-Host '[2/3] Installing VS 2026 BuildTools...' -ForegroundColor Yellow
$vsInstaller = 'https://aka.ms/vs/18/release/vs_buildtools.exe'
$vsArgs = @(
    '--quiet', '--wait', '--norestart',
    '--add', 'Microsoft.VisualStudio.Workload.VCTools',
    '--add', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
    '--add', 'Microsoft.VisualStudio.Component.VC.ATL',
    '--add', 'Microsoft.VisualStudio.Component.Windows11SDK.26100',
    '--add', 'Microsoft.VisualStudio.Component.VC.CMake.Project'
)
$tempInstaller = Join-Path $env:TEMP 'explorerlens-vs_buildtools.exe'
Invoke-WebRequest -Uri $vsInstaller -OutFile $tempInstaller
Start-Process -FilePath $tempInstaller -ArgumentList $vsArgs -Wait
Remove-Item $tempInstaller -Force -ErrorAction SilentlyContinue

# ── 3. Verify setup ───────────────────────────────────────────────────────────
Write-Host '[3/3] Verifying tools...' -ForegroundColor Yellow
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
