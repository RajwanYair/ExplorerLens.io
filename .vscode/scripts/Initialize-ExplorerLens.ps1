#Requires -Version 7.0
# .vscode/scripts/Initialize-ExplorerLens.ps1
# ExplorerLens.io workspace init — fast, self-contained, idempotent.
#
# Parameters:
#   -WorkspaceRoot <path>    Project root (auto-detected from script location if omitted)
#   -LoadMsvcEnv             Also import vcvars64 into this session (Developer PowerShell)
#   -SkipWorkspaceBootstrap  Set env vars only; skip helper functions (automation profile)
#
# Performance budget: <200 ms for the default (no -LoadMsvcEnv) path.
# vcvars64 adds ~1-2 s and is intentionally deferred behind -LoadMsvcEnv.

param(
    [string]$WorkspaceRoot,
    [switch]$LoadMsvcEnv,
    [switch]$SkipWorkspaceBootstrap
)

# ── Idempotency guard ──────────────────────────────────────────────────────
# Already bootstrapped? Only allow -LoadMsvcEnv to pass through so you can
# load MSVC env into a pre-bootstrapped session without re-defining functions.
if ($env:EXPLORERLENS_ENV_LOADED -eq '1' -and -not $LoadMsvcEnv) { return }

# ── Resolve workspace root ─────────────────────────────────────────────────
# This script lives at <root>\.vscode\scripts\ — walk up two levels.
$global:ELRoot = if ($WorkspaceRoot -and (Test-Path $WorkspaceRoot -PathType Container)) {
    $WorkspaceRoot
} else {
    Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
}

Set-Location $global:ELRoot

# ── Corporate proxy (one-time, non-blocking) ──────────────────────────────
if (-not $env:HTTPS_PROXY) {
    try {
        $sysProxy = [System.Net.WebRequest]::GetSystemWebProxy()
        $testUri  = [Uri]'https://api.github.com'
        if (-not $sysProxy.IsBypassed($testUri)) {
            $proxyUri = $sysProxy.GetProxy($testUri).ToString()
            # Avoid setting when the proxy returns the original URI (no proxy configured)
            if ($proxyUri -notmatch '^https://api\.github\.com') {
                $env:HTTP_PROXY  = $proxyUri
                $env:HTTPS_PROXY = $proxyUri
            }
        }
    } catch { <# Non-fatal — silently skip on restricted environments #> }
}

# ── vcpkg root (check once, cache in env) ─────────────────────────────────
if (-not $env:VCPKG_ROOT) {
    foreach ($p in @(
        "$global:ELRoot\external\vcpkg",
        "$global:ELRoot\build-vcpkg\vcpkg",
        "$env:USERPROFILE\vcpkg",
        'C:\vcpkg'
    )) {
        if (Test-Path "$p\vcpkg.exe") { $env:VCPKG_ROOT = $p; break }
    }
}

# ── MSVC environment (opt-in: -LoadMsvcEnv) ───────────────────────────────
# Sourcing vcvars64 takes ~1-2 s — never done automatically on terminal open.
if ($LoadMsvcEnv -and $env:MSVC_ENV_LOADED -ne '1') {
    # Locate vswhere — the authoritative VS discovery tool
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        $vswhere = "${env:ProgramFiles}\Microsoft Visual Studio\Installer\vswhere.exe"
    }

    if (Test-Path $vswhere) {
        # -products * matches BuildTools and Community/Professional/Enterprise
        $vsPath = & $vswhere -latest -products * -property installationPath 2>$null
        $vcvars = "$vsPath\VC\Auxiliary\Build\vcvars64.bat"

        if (Test-Path $vcvars) {
            # Import all env vars set by vcvars64.bat into this PowerShell process
            $envDump = cmd /c "`"$vcvars`" >nul 2>&1 && set" 2>$null
            foreach ($line in $envDump) {
                if ($line -match '^([^=]+)=(.*)$') {
                    [System.Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], 'Process')
                }
            }
            $env:MSVC_ENV_LOADED = '1'
            Write-Host "[EL] MSVC env loaded from: $vsPath" -ForegroundColor DarkGray
        } else {
            Write-Warning "[EL] vcvars64.bat not found — expected at: $vcvars"
        }
    } else {
        Write-Warning "[EL] vswhere.exe not found — install Visual Studio Build Tools"
    }
}

if ($SkipWorkspaceBootstrap) { return }

# ── Helper functions ───────────────────────────────────────────────────────
# All functions reference $global:ELRoot so they work correctly after the
# init script exits and its local scope is gone.

function global:Invoke-ELBuild {
    & "$global:ELRoot\build-scripts\Build-MSVC.ps1" @args
}
function global:Invoke-ELCleanBuild {
    & "$global:ELRoot\build-scripts\Build-MSVC.ps1" -Clean @args
}
function global:Invoke-ELTest {
    & "$global:ELRoot\build-scripts\Build-MSVC.ps1" -Clean -Test @args
}
function global:Invoke-ELPackage {
    & "$global:ELRoot\build-scripts\Build-All-And-Package.ps1" @args
}
function global:Invoke-ELStatus {
    & "$global:ELRoot\build-scripts\Check-Build-Status.ps1"
}
function global:Invoke-ELBumpVersion {
    & "$global:ELRoot\build-scripts\Bump-Version.ps1" @args
}

# Short prefix aliases — use el- to avoid collisions with system commands
Set-Alias -Name el-build   -Value Invoke-ELBuild        -Scope Global -Force
Set-Alias -Name el-clean   -Value Invoke-ELCleanBuild   -Scope Global -Force
Set-Alias -Name el-test    -Value Invoke-ELTest          -Scope Global -Force
Set-Alias -Name el-pkg     -Value Invoke-ELPackage       -Scope Global -Force
Set-Alias -Name el-status  -Value Invoke-ELStatus        -Scope Global -Force
Set-Alias -Name el-bump    -Value Invoke-ELBumpVersion   -Scope Global -Force

# ── cmake --preset tab completion ──────────────────────────────────────────
Register-ArgumentCompleter -CommandName cmake -ScriptBlock {
    param($wordToComplete, $commandAst, $cursorPosition)
    $presets = @('default-release', 'default-debug', 'vcpkg-release', 'vcpkg-debug', 'vs2026')
    if ($commandAst.ToString() -match '--preset') {
        $presets | Where-Object { $_ -like "$wordToComplete*" } |
            ForEach-Object {
                [System.Management.Automation.CompletionResult]::new($_, $_, 'ParameterValue', $_)
            }
    }
}

# ── Mark as loaded ─────────────────────────────────────────────────────────
$env:EXPLORERLENS_ENV_LOADED = '1'

Write-Host "[EL] ExplorerLens 39.9.0  el-build · el-clean · el-test · el-pkg · el-status · el-bump" -ForegroundColor DarkCyan
