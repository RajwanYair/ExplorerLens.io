<#
.SYNOPSIS
    Verify ExplorerLens project organization compliance

.DESCRIPTION
    Validates that the ExplorerLens project structure conforms to industrial
    open-source standards. Checks directory structure, documentation, and
    file organization.

.NOTES
    Version: 1.0.0
    Date: February 11, 2026
#>

param(
    [switch]$Detailed,
    [switch]$FixIssues
)

$ErrorActionPreference = "Continue"
$WorkspaceRoot = Split-Path -Parent $PSScriptRoot

Write-Host "`n=== ExplorerLens Project Structure Verification ===" -ForegroundColor Cyan
Write-Host "Workspace: $WorkspaceRoot`n" -ForegroundColor Gray

$issues = @()
$checks = 0
$passed = 0

function Test-DirectoryExists {
    param([string]$Path, [string]$Description)

    $script:checks++
    $fullPath = Join-Path $WorkspaceRoot $Path

    if (Test-Path $fullPath -PathType Container) {
        Write-Host "✓ $Description" -ForegroundColor Green
        $script:passed++
        return $true
    } else {
        Write-Host "✗ $Description" -ForegroundColor Red
        $script:issues += "Missing directory: $Path"
        return $false
    }
}

function Test-FileExists {
    param([string]$Path, [string]$Description)

    $script:checks++
    $fullPath = Join-Path $WorkspaceRoot $Path

    if (Test-Path $fullPath -PathType Leaf) {
        Write-Host "✓ $Description" -ForegroundColor Green
        $script:passed++
        return $true
    } else {
        Write-Host "✗ $Description" -ForegroundColor Red
        $script:issues += "Missing file: $Path"
        return $false
    }
}

function Test-FileNotExists {
    param([string]$Path, [string]$Description)

    $script:checks++
    $fullPath = Join-Path $WorkspaceRoot $Path

    if (-not (Test-Path $fullPath)) {
        Write-Host "✓ $Description" -ForegroundColor Green
        $script:passed++
        return $true
    } else {
        Write-Host "⚠ $Description" -ForegroundColor Yellow
        $script:issues += "Obsolete file still exists: $Path"
        return $false
    }
}

# ============================================================================
# 1. Root Directory Structure
# ============================================================================

Write-Host "`n[1/7] Verifying Root Directory Structure..." -ForegroundColor Cyan
Write-Host "────────────────────────────────────────────" -ForegroundColor DarkGray

Test-FileExists "README.md" "Root README.md exists"
Test-FileExists "MASTER_PLAN.md" "MASTER_PLAN.md exists"
Test-FileExists "LICENSE" "LICENSE file exists"
Test-FileExists "docs\architecture\PROJECT_STRUCTURE.md" "PROJECT_STRUCTURE.md exists"
Test-FileExists "CMakeLists.txt" "Root CMakeLists.txt exists"
Test-FileExists "LENSShell.sln" "Visual Studio solution exists"
Test-FileExists ".gitignore" ".gitignore exists"
Test-FileExists ".gitattributes" ".gitattributes exists"

Test-FileNotExists "SPRINT_SUMMARY.md" "Legacy sprint summary removed from root"

# ============================================================================
# 2. .github Directory Structure
# ============================================================================

Write-Host "`n[2/7] Verifying .github Directory..." -ForegroundColor Cyan
Write-Host "────────────────────────────────────────────" -ForegroundColor DarkGray

Test-DirectoryExists ".github" ".github directory exists"
Test-DirectoryExists ".github\workflows" ".github\workflows exists"
Test-DirectoryExists ".github\ISSUE_TEMPLATE" ".github\ISSUE_TEMPLATE exists"
Test-DirectoryExists ".github\standards" ".github\standards exists"

Test-FileExists ".github\CONTRIBUTING.md" "CONTRIBUTING.md exists"
Test-FileExists ".github\SECURITY.md" "SECURITY.md exists"
Test-FileExists ".github\PULL_REQUEST_TEMPLATE.md" "PR template exists"
Test-FileExists ".github\standards\coding-standards.md" "Coding standards exist"

Test-FileExists ".github\workflows\build.yml" "Build workflow exists"

Test-FileNotExists ".github\docs" ".github\docs directory removed"

# ============================================================================
# 3. Documentation Structure
# ============================================================================

Write-Host "`n[3/7] Verifying Documentation Structure..." -ForegroundColor Cyan
Write-Host "────────────────────────────────────────────" -ForegroundColor DarkGray

Test-DirectoryExists "docs" "docs directory exists"
Test-DirectoryExists "docs\development" "docs\development exists"
Test-DirectoryExists "docs\architecture" "docs\architecture exists"
Test-DirectoryExists "docs\build" "docs\build exists"
Test-DirectoryExists "docs\getting-started" "docs\getting-started exists"
Test-DirectoryExists "docs\testing" "docs\testing exists"
Test-DirectoryExists "docs\plugins" "docs\plugins exists"
Test-DirectoryExists "docs\audits" "docs\audits exists"

Test-FileExists "docs\development\README.md" "Development README exists"
Test-FileExists "docs\development\AI_BUILD_INSTRUCTIONS.md" "AI build instructions moved"
Test-FileExists "docs\development\BUILD_QUICK_REFERENCE.md" "Build quick ref moved"
Test-FileExists "docs\development\THIRD_PARTY.md" "Third-party docs moved"
Test-FileExists "docs\INDEX.md" "Documentation index exists"

# ============================================================================
# 4. Build Scripts Structure
# ============================================================================

Write-Host "`n[4/7] Verifying Build Scripts Structure..." -ForegroundColor Cyan
Write-Host "────────────────────────────────────────────" -ForegroundColor DarkGray

Test-DirectoryExists "build-scripts" "build-scripts directory exists"
Test-DirectoryExists "build-scripts\external-libs" "external-libs subdirectory exists"
Test-DirectoryExists "build-scripts\production" "production subdirectory exists"
Test-DirectoryExists "build-scripts\library-builders" "library-builders exists"
Test-DirectoryExists "build-scripts\utilities" "utilities subdirectory exists"
Test-DirectoryExists "build-scripts\validation" "validation subdirectory exists"

Test-FileExists "build-scripts\README.md" "Build scripts README exists"
Test-FileExists "scripts\build.ps1" "Main build script exists"

Test-FileExists "build-scripts\external-libs\Build-LibWebP-NMake.ps1" "LibWebP script moved"
Test-FileExists "build-scripts\external-libs\Build-LibJXL.ps1" "LibJXL script moved"
Test-FileExists "build-scripts\production\Build-Production-SlowMachine.ps1" "Production script moved"

Test-FileNotExists "build-scripts\build-liblzma.bat" "Obsolete .bat script removed"
Test-FileNotExists "build-scripts\Build-External-Libs-Debug.ps1" "Obsolete debug script removed"

# ============================================================================
# 5. External Dependencies
# ============================================================================

Write-Host "`n[5/7] Verifying External Dependencies..." -ForegroundColor Cyan
Write-Host "────────────────────────────────────────────" -ForegroundColor DarkGray

Test-DirectoryExists "external" "external directory exists"
Test-DirectoryExists "external\compression" "compression libs directory exists"
Test-DirectoryExists "external\image-libs" "image-libs directory exists"
Test-DirectoryExists "external\archive-libs" "archive libs directory exists"

Test-FileExists "external\README.md" "External README exists"
Test-FileExists "external\LIBRARY_INVENTORY.md" "Library inventory exists"


# ============================================================================
# 6. Core Components
# ============================================================================

Write-Host "`n[6/7] Verifying Core Components..." -ForegroundColor Cyan
Write-Host "────────────────────────────────────────────" -ForegroundColor DarkGray

Test-DirectoryExists "LENSShell" "LENSShell directory exists"
Test-DirectoryExists "LENSManager" "LENSManager directory exists"
Test-DirectoryExists "Engine" "Engine directory exists"
Test-DirectoryExists "SDK" "SDK directory exists"
Test-DirectoryExists "tests" "tests directory exists"

Test-FileExists "Engine\README.md" "Engine README exists"
Test-FileExists "SDK\README.md" "SDK README exists"
Test-FileExists "tests\README.md" "Tests README exists"

# ============================================================================
# 7. Scripts and Tools
# ============================================================================

Write-Host "`n[7/7] Verifying Scripts and Tools..." -ForegroundColor Cyan
Write-Host "────────────────────────────────────────────" -ForegroundColor DarkGray

Test-DirectoryExists "scripts" "scripts directory exists"
Test-FileExists "scripts\setup\Reorganize-Project.ps1" "Reorganization script exists"

Test-DirectoryExists "scripts\release" "scripts\release exists"
Test-DirectoryExists "tools" "tools directory exists"

# ============================================================================
# Summary
# ============================================================================

Write-Host "`n=== Verification Summary ===" -ForegroundColor Cyan
Write-Host "Total Checks: $checks" -ForegroundColor White
Write-Host "Passed: $passed" -ForegroundColor Green
Write-Host "Failed: $($checks - $passed)" -ForegroundColor $(if ($checks -eq $passed) { "Green" } else { "Red" })

$percentage = [math]::Round(($passed / $checks) * 100, 1)
Write-Host "Compliance: $percentage%" -ForegroundColor $(if ($percentage -ge 95) { "Green" } elseif ($percentage -ge 85) { "Yellow" } else { "Red" })

if ($issues.Count -gt 0) {
    Write-Host "`n=== Issues Found ===" -ForegroundColor Yellow
    foreach ($issue in $issues) {
        Write-Host "  • $issue" -ForegroundColor Yellow
    }

    if ($FixIssues) {
        Write-Host "`nAttempting to fix issues..." -ForegroundColor Cyan
        # Add fix logic here if needed
    }
} else {
    Write-Host "`n✓ Project structure is fully compliant with industrial standards!" -ForegroundColor Green
}

if ($Detailed) {
    Write-Host "`n=== Detailed Structure ===" -ForegroundColor Cyan

    $structure = @{
        "Root Files"  = Get-ChildItem $WorkspaceRoot -File | Where-Object { $_.Name -match "\.md$|LICENSE|\.sln$" } | Select-Object -ExpandProperty Name
        "Directories" = Get-ChildItem $WorkspaceRoot -Directory | Where-Object { $_.Name -notlike ".*" -and $_.Name -notlike "x64" -and $_.Name -notlike "build" } | Select-Object -ExpandProperty Name
    }

    foreach ($category in $structure.Keys) {
        Write-Host "`n$category`:" -ForegroundColor Cyan
        $structure[$category] | ForEach-Object { Write-Host "  • $_" -ForegroundColor Gray }
    }
}

Write-Host "`n" -NoNewline

# Exit code
if ($checks -eq $passed) {
    exit 0
} else {
    exit 1
}
