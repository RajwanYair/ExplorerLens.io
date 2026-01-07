# =============================================================================
# Validate-Sprint18.ps1 - Sprint 18 Deliverables Verification
# =============================================================================
# Validates all v6.0.0 Release deliverables are complete
# =============================================================================

param(
    [switch]$Detailed
)

$ErrorActionPreference = "Continue"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Sprint 18 Validation - v6.0.0 Launch" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$projectRoot = Split-Path -Parent $PSScriptRoot
$passed = 0
$failed = 0
$warnings = 0

function Test-Deliverable {
    param(
        [string]$Name,
        [string]$Path,
        [string]$Type = "File"
    )
    
    Write-Host "[TEST] $Name" -ForegroundColor Yellow
    
    $fullPath = Join-Path $projectRoot $Path
    $exists = if ($Type -eq "File") { Test-Path $fullPath -PathType Leaf } else { Test-Path $fullPath -PathType Container }
    
    if ($exists) {
        Write-Host "  ✅ PASS" -ForegroundColor Green
        if ($Detailed -and $Type -eq "File") {
            $size = [math]::Round((Get-Item $fullPath).Length / 1KB, 2)
            Write-Host "     Path: $Path" -ForegroundColor Gray
            Write-Host "     Size: $size KB" -ForegroundColor Gray
        }
        $script:passed++
    } else {
        Write-Host "  ❌ FAIL - Not found: $Path" -ForegroundColor Red
        $script:failed++
    }
    Write-Host ""
}

function Test-BuildOutput {
    param([string]$Name, [string]$Path)
    
    Write-Host "[TEST] $Name" -ForegroundColor Yellow
    
    $fullPath = Join-Path $projectRoot $Path
    if (Test-Path $fullPath) {
        $size = [math]::Round((Get-Item $fullPath).Length / 1MB, 2)
        Write-Host "  ✅ PASS - $size MB" -ForegroundColor Green
        $script:passed++
    } else {
        Write-Host "  ⚠️  WARN - Not built yet: $Path" -ForegroundColor Yellow
        Write-Host "     Run Build-Production.ps1 to generate" -ForegroundColor Gray
        $script:warnings++
    }
    Write-Host ""
}

# Task 18.1: SBOM Generation
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "Task 18.1: SBOM Generation" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

Test-Deliverable "SBOM JSON" "SBOM.json"
Test-Deliverable "SBOM Generator Script" "release-scripts\Generate-SBOM.ps1"

# Task 18.2: Code Signing Infrastructure
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "Task 18.2: Code Signing Infrastructure" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

Test-Deliverable "Signing Script" "release-scripts\Sign-Binaries.ps1"
Test-Deliverable "Code Signing Documentation" "docs\CODE_SIGNING.md"

# Task 18.3: Multi-Format Packaging
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "Task 18.3: Multi-Format Packaging" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

Test-Deliverable "Package Creation Script" "release-scripts\Create-Packages.ps1"
Test-Deliverable "MSIX Manifest" "packaging\msix\AppxManifest.xml"

# Task 18.4: Documentation Site
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "Task 18.4: Documentation Site" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

Test-Deliverable "MkDocs Configuration" "mkdocs.yml"
Test-Deliverable "Installation Guide" "docs\getting-started\installation.md"
Test-Deliverable "SDK Guide" "docs\SDK_GUIDE.md"
Test-Deliverable "Format Strategy" "docs\FORMAT_STRATEGY.md"
Test-Deliverable "Compatibility Kit Spec" "docs\COMPATIBILITY_KIT_SPEC.md"

# Task 18.5: Marketplace Beta Registry
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "Task 18.5: Marketplace Beta Registry" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

Test-Deliverable "Marketplace Registry" "marketplace\registry.json"
Test-Deliverable "Marketplace Documentation" "marketplace\README.md"

# Task 18.6: Release Notes & Migration
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "Task 18.6: Release Notes & Migration" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

Test-Deliverable "Release Notes v6.0.0" "RELEASE_NOTES_v6.0.0.md"

# Task 18.7: Build Verification
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "Task 18.7: Build Verification" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

Test-BuildOutput "CBXShell.dll" "x64\Release\CBXShell.dll"
Test-BuildOutput "CBXManager.exe" "x64\Release\CBXManager.exe"

# Architecture Verification
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "Architecture & Structure" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

Test-Deliverable "Engine Module" "src\Engine" "Directory"
Test-Deliverable "ShellHost Module" "src\ShellHost" "Directory"
Test-Deliverable "Worker Module" "src\Worker" "Directory"
Test-Deliverable "SDK Module" "src\SDK" "Directory"
Test-Deliverable "CLI Tools" "src\Tools.CLI" "Directory"
Test-Deliverable "PowerShell Module" "src\Tools.PSModule" "Directory"

# Sprint Documentation
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "Sprint Planning & Documentation" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

Test-Deliverable "Master Plan" "docs\MAKE_IT_GREAT_AGAIN_2026.md"
Test-Deliverable "Build Verification" "BUILD_VERIFICATION_2025-12-10.md"

# Summary
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Validation Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$total = $passed + $failed + $warnings
Write-Host "Total Tests: $total" -ForegroundColor White
Write-Host "  ✅ Passed: $passed" -ForegroundColor Green
Write-Host "  ❌ Failed: $failed" -ForegroundColor $(if ($failed -gt 0) { "Red" } else { "Green" })
Write-Host "  ⚠️  Warnings: $warnings" -ForegroundColor $(if ($warnings -gt 0) { "Yellow" } else { "Green" })
Write-Host ""

# Calculate completion percentage
$completionRate = [math]::Round(($passed / $total) * 100, 1)
Write-Host "Completion Rate: $completionRate%" -ForegroundColor White

if ($failed -eq 0 -and $warnings -eq 0) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "  ✅ Sprint 18 - COMPLETE" -ForegroundColor Green
    Write-Host "  All deliverables verified!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next Steps:" -ForegroundColor Cyan
    Write-Host "  1. Build production binaries: .\Build-Production.ps1" -ForegroundColor Gray
    Write-Host "  2. Sign binaries: .\release-scripts\Sign-Binaries.ps1 -TestMode" -ForegroundColor Gray
    Write-Host "  3. Create packages: .\release-scripts\Create-Packages.ps1" -ForegroundColor Gray
    Write-Host "  4. Update master plan: Mark Sprint 18 as ✅" -ForegroundColor Gray
    exit 0
} elseif ($failed -gt 0) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "  ❌ Sprint 18 - INCOMPLETE" -ForegroundColor Red
    Write-Host "  $failed deliverable(s) missing" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    exit 1
} else {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Yellow
    Write-Host "  ⚠️  Sprint 18 - MOSTLY COMPLETE" -ForegroundColor Yellow
    Write-Host "  $warnings warning(s) - build required" -ForegroundColor Yellow
    Write-Host "========================================" -ForegroundColor Yellow
    exit 0
}
