# ExplorerLens v39.9.0 - PowerShell Profile Configuration
# Bootstraps the ExplorerLens build environment in a persistent $PROFILE.
#
# Installation:
#   Add to $PROFILE (e.g. C:\Users\YourName\Documents\PowerShell\Microsoft.PowerShell_profile.ps1):
#
#       $ELRoot = "C:\path\to\ExplorerLens.io"
#       . "$ELRoot\.vscode\scripts\Initialize-ExplorerLens.ps1" -WorkspaceRoot $ELRoot
#
#   For a Developer PowerShell that also loads MSVC (vcvars64):
#
#       . "$ELRoot\.vscode\scripts\Initialize-ExplorerLens.ps1" -WorkspaceRoot $ELRoot -LoadMsvcEnv
#
# The init script is idempotent — safe to re-source in the same session.

# ── Set this to your local clone path ────────────────────────────────────
$ELRoot = "$PSScriptRoot\.."   # Resolved relative to this script's location

# ── Bootstrap ExplorerLens environment ───────────────────────────────────
$initScript = Join-Path $ELRoot '.vscode\scripts\Initialize-ExplorerLens.ps1'
if (Test-Path $initScript -PathType Leaf) {
    . $initScript -WorkspaceRoot (Resolve-Path $ELRoot).Path
} else {
    Write-Warning "[EL] Init script not found: $initScript"
    Write-Warning "     Ensure ExplorerLens.io is cloned and \$ELRoot is set correctly."
}

# ── Performance optimizations (interactive profile only) ─────────────────
# Increase PowerShell command history
$MaximumHistoryCount = 10000

# Tab completion — MenuComplete shows a scrollable list
Set-PSReadLineKeyHandler -Key Tab -Function MenuComplete

# Predictive IntelliSense from history (PowerShell 7.2+)
if ($PSVersionTable.PSVersion -ge [version]'7.2') {
    Set-PSReadLineOption -PredictionSource History
    Set-PSReadLineOption -PredictionViewStyle ListView
}
