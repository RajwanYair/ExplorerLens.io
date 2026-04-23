# ─────────────────────────────────────────────────────────────────────────────
# .env.ps1 — ExplorerLens.io project bootstrap
# ─────────────────────────────────────────────────────────────────────────────
# Loads shared tooling from the MyScripts root, then applies ExplorerLens-only
# helpers and completions.
#
# Usage:
#   . .\.env.ps1                 # from the project root
#   . "$PSScriptRoot\.env.ps1"   # from scripts or tasks
#
# Idempotent — safe to source multiple times in the same session.
# ─────────────────────────────────────────────────────────────────────────────

param(
    [switch]$SkipCommonBootstrap
)

# Guard: only run once per session
if ($env:EXPLORERLENS_ENV_LOADED -eq '1') { return }

# ── Corporate proxy for Go-based CLIs (gh, etc.) ────────────────────────────
if (-not $env:HTTPS_PROXY) {
    $sysProxy = [System.Net.WebRequest]::GetSystemWebProxy()
    $testUri  = [Uri]'https://api.github.com'
    if (-not $sysProxy.IsBypassed($testUri)) {
        $proxyUri = $sysProxy.GetProxy($testUri).ToString()
        $env:HTTP_PROXY  = $proxyUri
        $env:HTTPS_PROXY = $proxyUri
    }
}

if (-not $SkipCommonBootstrap) {
    $commonBootstrap = Join-Path (Split-Path $PSScriptRoot -Parent) '.vscode\scripts\Initialize-CommonTooling.ps1'
    if (Test-Path $commonBootstrap -PathType Leaf) {
        . $commonBootstrap -WorkspaceRoot $PSScriptRoot -SkipWorkspaceBootstrap
    }
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

$env:EXPLORERLENS_ENV_LOADED = '1'

Write-Host "[.env.ps1] ExplorerLens project helpers loaded" -ForegroundColor DarkCyan
Write-Host "  Helpers: Invoke-ELBuild, Invoke-ELCleanBuild, Invoke-ELTest, Invoke-ELPackage, Invoke-ELStatus" -ForegroundColor DarkGray
