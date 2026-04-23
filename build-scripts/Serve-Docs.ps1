#Requires -Version 7.0
<#
.SYNOPSIS
    Starts a local HTTP server for verifying the ExplorerLens documentation site.

.DESCRIPTION
    Serves the repo root (index.html) and docs/ tree via Python's built-in
    http.server on localhost:8080.  Press Ctrl-C to stop.

    Use the VS Code task "Serve Local Site" to launch this script.

.PARAMETER Port
    TCP port to listen on.  Default: 8080.

.PARAMETER RootDir
    Directory to serve.  Default: repo root (the folder containing this script's parent).

.EXAMPLE
    .\build-scripts\Serve-Docs.ps1
    .\build-scripts\Serve-Docs.ps1 -Port 9000
#>
[CmdletBinding()]
param(
    [ValidateRange(1, 65535)]
    [int]$Port = 8080,

    [string]$RootDir = (Split-Path $PSScriptRoot -Parent)
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ── Validate Python ────────────────────────────────────────────────────────────
$python = Get-Command python3 -ErrorAction SilentlyContinue
if (-not $python) {
    $python = Get-Command python -ErrorAction SilentlyContinue
}
if (-not $python) {
    Write-Error "Python 3 is required but was not found on PATH. Install Python 3.9+ and re-run."
    exit 1
}

$pyVersion = & $python.Source --version 2>&1
Write-Host "Using: $pyVersion" -ForegroundColor Cyan
Write-Host "Root : $RootDir" -ForegroundColor Cyan
Write-Host ""
Write-Host "Serving ExplorerLens docs at  http://localhost:$Port" -ForegroundColor Green
Write-Host "Open index.html               http://localhost:$Port/index.html" -ForegroundColor Green
Write-Host "Open docs/                    http://localhost:$Port/docs/" -ForegroundColor Green
Write-Host ""
Write-Host "Press Ctrl-C to stop." -ForegroundColor Yellow

Push-Location $RootDir
try {
    & $python.Source -m http.server $Port --bind 127.0.0.1
}
finally {
    Pop-Location
}
