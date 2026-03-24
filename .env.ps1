# ─────────────────────────────────────────────────────────────────────────────
# .env.ps1 — ExplorerLens.io project environment bootstrap
# ─────────────────────────────────────────────────────────────────────────────
# Dot-source this file in any PowerShell session to ensure every CLI tool
# available on this machine is reachable without manual $env:PATH editing.
#
# Usage:
#   . .\.env.ps1                 # from the project root
#   . "$PSScriptRoot\.env.ps1"   # from scripts or tasks
#
# Idempotent — safe to source multiple times in the same session.
# ─────────────────────────────────────────────────────────────────────────────

# ── MSBuild reliability: disable cross-build node reuse ─────────────────────
# Without this, persistent MSBuild worker nodes hold file handles on
# %TEMP%\**\*.cache files and cause MSB3492 on the next build.
# Set before idempotency guard so every session benefits.
$env:MSBUILDDISABLENODEREUSE = '1'

# Guard: only run once per session
if ($env:EXPLORERLENS_ENV_LOADED -eq '1') { return }

# ── Helper: append a directory to PATH only if it exists and isn't there yet ─
function Add-PathEntry {
    param([string]$Dir)
    $resolved = [System.Environment]::ExpandEnvironmentVariables($Dir)
    if (-not (Test-Path $resolved -PathType Container)) { return }
    $current = $env:PATH -split ';'
    if ($current -notcontains $resolved) {
        $env:PATH = "$resolved;$env:PATH"
    }
}

# ── Scoop shims (covers cmake, ninja, git, 7zip, nasm, meson, nuget, wix…) ──
Add-PathEntry "$env:USERPROFILE\scoop\shims"

# ── Scoop Git with full Unix tools (required for git commands in scripts) ────
Add-PathEntry "$env:USERPROFILE\scoop\apps\git\current\bin"
Add-PathEntry "$env:USERPROFILE\scoop\apps\git\current\cmd"
Add-PathEntry "$env:USERPROFILE\scoop\apps\git\current\usr\bin"

# ── MSVC v145 / Visual Studio 18 2026 BuildTools ────────────────────────────
Add-PathEntry "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Tools\MSVC\14.50.35717\bin\Hostx64\x64"
Add-PathEntry "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64"

# ── Windows SDK ──────────────────────────────────────────────────────────────
Add-PathEntry "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64"

# ── vcpkg (bundled in BuildTools) ────────────────────────────────────────────
Add-PathEntry "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\vcpkg"

# ── Scoop apps with their own bin directories ────────────────────────────────
Add-PathEntry "$env:USERPROFILE\scoop\apps\cmake\current\bin"
Add-PathEntry "$env:USERPROFILE\scoop\apps\nuget\current"
Add-PathEntry "$env:USERPROFILE\scoop\apps\mingw\current\bin"
Add-PathEntry "$env:USERPROFILE\scoop\apps\llvm\current\bin"
Add-PathEntry "$env:USERPROFILE\scoop\apps\perl\current\perl\bin"

# ── PowerShell 7 ─────────────────────────────────────────────────────────────
Add-PathEntry "$env:PROGRAMFILES\PowerShell\7"

# ── .NET SDK (for WiX Toolset and packaging) ─────────────────────────────────
Add-PathEntry "$env:PROGRAMFILES\dotnet"
Add-PathEntry "$env:USERPROFILE\.dotnet\tools"

# ── Python ───────────────────────────────────────────────────────────────────
Add-PathEntry "$env:LOCALAPPDATA\Python\bin"
Add-PathEntry "$env:LOCALAPPDATA\Programs\Python\Python314\Scripts"
Add-PathEntry "$env:LOCALAPPDATA\Programs\Python\Python313\Scripts"

# ── Node.js / npm ────────────────────────────────────────────────────────────
Add-PathEntry "$env:PROGRAMFILES\nodejs"
Add-PathEntry "$env:APPDATA\npm"

# ── WinGet ───────────────────────────────────────────────────────────────────
Add-PathEntry "$env:LOCALAPPDATA\Microsoft\WindowsApps"

# ── Chocolatey ───────────────────────────────────────────────────────────────
Add-PathEntry "$env:PROGRAMDATA\chocolatey\bin"

# ── GitHub CLI ───────────────────────────────────────────────────────────────
Add-PathEntry "$env:PROGRAMFILES\GitHub CLI"

# ── Docker Desktop ───────────────────────────────────────────────────────────
Add-PathEntry "$env:PROGRAMFILES\Docker\Docker\resources\bin"

# ── Rust / Cargo ─────────────────────────────────────────────────────────────
Add-PathEntry "$env:USERPROFILE\.cargo\bin"

# ── VS Code CLI ──────────────────────────────────────────────────────────────
Add-PathEntry "$env:PROGRAMFILES\Microsoft VS Code"

# ── Inno Setup (packaging) ───────────────────────────────────────────────────
Add-PathEntry "$env:USERPROFILE\scoop\apps\inno-setup\current"

# ── Proxy configuration for git and HTTPS tools ──────────────────────────────
# Uses the machine-configured Intel proxy (proxy-dmz.intel.com:912)
$_proxy = "http://proxy-dmz.intel.com:912"
$env:HTTP_PROXY  = $_proxy
$env:HTTPS_PROXY = $_proxy
$env:http_proxy  = $_proxy
$env:https_proxy = $_proxy
# NO_PROXY: skip proxy for localhost and local OneDrive paths
$env:NO_PROXY    = "localhost,127.0.0.1,::1"

# ── Mark as loaded ───────────────────────────────────────────────────────────
$env:EXPLORERLENS_ENV_LOADED = '1'

# ── Git shortcut pointing to scoop git (workaround for PATH aliasing issue) ──
$_gitExe = "$env:USERPROFILE\scoop\apps\git\current\bin\git.exe"
if (Test-Path $_gitExe) {
    function global:git { & $_gitExe @args }
}

# ── ExplorerLens project helpers ─────────────────────────────────────────────
function Invoke-ELBuild        { & "$PSScriptRoot\build-scripts\Build-MSVC.ps1" @args }
function Invoke-ELCleanBuild   { & "$PSScriptRoot\build-scripts\Build-MSVC.ps1" -Clean @args }
function Invoke-ELTest         { & "$PSScriptRoot\build-scripts\Build-MSVC.ps1" -Clean -Test @args }
function Invoke-ELPackage      { & "$PSScriptRoot\build-scripts\Build-All-And-Package.ps1" @args }
function Invoke-ELStatus       { & "$PSScriptRoot\build-scripts\Check-Build-Status.ps1" }

# ── Tab completion for build presets ─────────────────────────────────────────
$_elPresets = @('default-release','default-debug','vcpkg-release','vcpkg-debug','vs2026')
Register-ArgumentCompleter -CommandName 'cmake' -ScriptBlock {
    param($wordToComplete, $commandAst, $cursorPosition)
    $cmdText = $commandAst.ToString()
    if ($cmdText -match 'cmake\s+--preset\s+') {
        $_elPresets | Where-Object { $_ -like "$wordToComplete*" } |
        ForEach-Object { [System.Management.Automation.CompletionResult]::new($_, $_, 'ParameterValue', $_) }
    }
}

# ── PSReadLine enhancements (prediction, history search) ─────────────────────
if (Get-Module -ListAvailable PSReadLine -ErrorAction SilentlyContinue) {
    Set-PSReadLineOption -PredictionSource HistoryAndPlugin -ErrorAction SilentlyContinue
    Set-PSReadLineOption -PredictionViewStyle ListView       -ErrorAction SilentlyContinue
    Set-PSReadLineKeyHandler -Key UpArrow   -Function HistorySearchBackward
    Set-PSReadLineKeyHandler -Key DownArrow -Function HistorySearchForward
    Set-PSReadLineKeyHandler -Key Tab       -Function MenuComplete
}

# ── Custom prompt with git branch + command timing ───────────────────────────
function global:prompt {
    $exitCode = $LASTEXITCODE
    $duration = ''
    if ((Get-History -Count 1 -ErrorAction SilentlyContinue) -is [Microsoft.PowerShell.Commands.HistoryInfo]) {
        $last = Get-History -Count 1
        $ms = ($last.EndExecutionTime - $last.StartExecutionTime).TotalMilliseconds
        if ($ms -ge 1000) { $duration = " ($([math]::Round($ms / 1000, 1))s)" }
        elseif ($ms -ge 100) { $duration = " ($([int]$ms)ms)" }
    }
    $branch = ''
    try {
        $b = & "$env:USERPROFILE\scoop\apps\git\current\bin\git.exe" rev-parse --abbrev-ref HEAD 2>$null
        if ($b) { $branch = " ($b)" }
    } catch {}
    $cwd   = (Get-Location).Path -replace [regex]::Escape($HOME), '~'
    $arrow = if ($exitCode -eq 0 -or $null -eq $exitCode) { "`e[32m❯`e[0m" } else { "`e[31m❯`e[0m" }
    "`e[36m$cwd`e[33m$branch`e[90m$duration`e[0m`n$arrow "
}

Write-Host "[.env.ps1] ExplorerLens.io dev environment loaded — $(($env:PATH -split ';').Count) PATH entries" -ForegroundColor DarkCyan
Write-Host "  Proxy  : $env:HTTPS_PROXY" -ForegroundColor DarkGray
Write-Host "  Remote : https://github.com/RajwanYair/ExplorerLens.io.git" -ForegroundColor DarkGray
Write-Host "  Helpers: Invoke-ELBuild, Invoke-ELCleanBuild, Invoke-ELTest, Invoke-ELPackage, Invoke-ELStatus" -ForegroundColor DarkGray
