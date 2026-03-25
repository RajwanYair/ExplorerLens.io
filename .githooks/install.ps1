#!/usr/bin/env pwsh
# .githooks/install.ps1 — Install ExplorerLens git hooks
#
# Run once after cloning:
#   .\\.githooks\\install.ps1
#
# This sets git's hooksPath so all hooks in .githooks/ are active.
#
Set-StrictMode -Version Latest

$root = git rev-parse --show-toplevel
git -C $root config core.hooksPath .githooks

Write-Host "Git hooks installed: core.hooksPath = .githooks" -ForegroundColor Green
Write-Host ""
Write-Host "Hooks active:" -ForegroundColor Cyan
Get-ChildItem (Join-Path $root '.githooks') -File |
    Where-Object { $_.Name -notmatch '\.ps1$' } |
    ForEach-Object { Write-Host "  - $($_.Name)" -ForegroundColor Gray }
