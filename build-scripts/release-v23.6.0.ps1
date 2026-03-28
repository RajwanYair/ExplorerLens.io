# Release script for v23.6.0 "Vega-W"
# Run from the repository root after all version files are updated.
param(
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

$gitExe = "$env:USERPROFILE\scoop\apps\git\current\bin\git.exe"
if (-not (Test-Path $gitExe)) { $gitExe = 'git' }

function Invoke-Git {
    param([string[]]$Args)
    $result = & $gitExe @Args 2>&1
    $result | ForEach-Object { Write-Host $_ }
    if ($LASTEXITCODE -ne 0) { throw "git $($Args -join ' ') exited $LASTEXITCODE" }
    return $result
}

Write-Host "=== ExplorerLens v23.6.0 Release Script ===" -ForegroundColor Cyan
Write-Host "Repo : $repoRoot"
Write-Host "Mode : $(if ($DryRun) { 'DRY RUN' } else { 'LIVE' })" -ForegroundColor $(if ($DryRun) { 'Yellow' } else { 'Green' })
Write-Host ""

# 1 — Show changed files
Write-Host "--- Changed files ---" -ForegroundColor Yellow
Invoke-Git 'status', '--short'
Write-Host ""

if (-not $DryRun) {
    # 2 — Stage all changes
    Write-Host "--- Staging all changes ---" -ForegroundColor Yellow
    Invoke-Git 'add', '-A'

    # 3 — Commit
    Write-Host "--- Committing ---" -ForegroundColor Yellow
    Invoke-Git 'commit', '-m', 'chore: bump version to v23.6.0 (Vega-W)', '-m', @'
Release v23.6.0 "Vega-W" — Security Hardening v2

Deliverables (Sprints 461-470):
- Engine/Core/ZeroTrustPolicyEngine.h: Zero-trust COM/plugin access policy
- Engine/Core/DecoderSandboxIsolation.h: Job Object decoder isolation
- Engine/Core/RuntimeIntegrityVerifier.h: Runtime WDAC code-integrity check
- Engine/Utils/ExploitMitigationEngine.h: CFG/CET/SEHOP exploit mitigation
- Engine/Core/PrivilegeSeparationBroker.h: Low <-> High IL broker
- Engine/Core/SecureIPCChannel.h: ECDH + AES-GCM IPC channel
- Engine/Utils/AuditTrailEncryptor.h: AES-256-GCM audit trail
- Engine/Utils/AntiTamperDetector.h: Anti-tamper / anti-debug detector

Version bumps:
- VERSION, vcpkg.json, BuildValidation.h, SBOMGenerator.h
- CHANGELOG.md, SBOM.json, baseline.json
- README.md, social-preview.svg, architecture-build.svg
- copilot-instructions.md, tool-versions.md

Sprint plan: Added SPRINT_PLAN_700.md (Sprints 661-760, v26.2.0-v27.3.0)
Test count: 3117 unit tests (was 2938)
'@

    # 4 — Tag
    Write-Host "--- Tagging v23.6.0 ---" -ForegroundColor Yellow
    Invoke-Git 'tag', '-a', 'v23.6.0', '-m', 'Release v23.6.0 (Vega-W) — Security Hardening v2'

    # 5 — Push with tags (triggers release.yml)
    Write-Host "--- Pushing to origin (triggers GitHub Release) ---" -ForegroundColor Yellow
    Invoke-Git 'push', 'origin', 'main', '--tags'

    Write-Host ""
    Write-Host "=== v23.6.0 release pushed. GitHub Actions will publish all artifacts. ===" -ForegroundColor Green
    Write-Host "  Monitor: https://github.com/RajwanYair/ExplorerLens.io/actions" -ForegroundColor Cyan
    Write-Host "  Release: https://github.com/RajwanYair/ExplorerLens.io/releases/tag/v23.6.0" -ForegroundColor Cyan
} else {
    Write-Host "DRY RUN complete — no changes pushed." -ForegroundColor Yellow
}
