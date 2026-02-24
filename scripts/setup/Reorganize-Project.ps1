# Reorganize-Project.ps1
# Systematically reorganize ExplorerLens project directory structure

$ErrorActionPreference = 'Stop'
$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Set-Location $rootDir

Write-Host "=== ExplorerLens Project Reorganization ===" -ForegroundColor Cyan
Write-Host ""

# 1. Create directory structure
Write-Host "Creating directory structure..." -ForegroundColor Yellow
$directories = @(
    "scripts\build",
    "scripts\install", 
    "scripts\test",
    "scripts\release",
    "scripts\setup",
    "documentation\guides",
    "documentation\status",
    "build-output"
)

foreach ($dir in $directories) {
    $fullPath = Join-Path $rootDir $dir
    if (-not (Test-Path $fullPath)) {
        New-Item -ItemType Directory -Path $fullPath -Force | Out-Null
        Write-Host "  [OK] Created: $dir" -ForegroundColor Green
    } else {
        Write-Host "  [>>] Exists: $dir" -ForegroundColor Gray
    }
}

Write-Host ""

# 2. Move Build Scripts
Write-Host "Moving build scripts..." -ForegroundColor Yellow
$buildScripts = @(
    "Build-Async.ps1",
    "Build-Production.ps1", 
    "Build-Pure64bit.ps1",
    "Build-ReleasePackage.ps1",
    "Quick-Build.ps1",
    "quick-update.ps1",
    "automate-release.cmd",
    "build-release-package.cmd",
    "build-v5.2.0-gpu.cmd",
    "quick-check.cmd"
)

foreach ($file in $buildScripts) {
    if (Test-Path $file) {
        Move-Item -Path $file -Destination ".\scripts\build\" -Force
        Write-Host "  [OK] Moved: $file" -ForegroundColor Green
    }
}

# 3. Move Install Scripts
Write-Host "Moving install/uninstall scripts..." -ForegroundColor Yellow
$installScripts = @(
    "Deploy-ExplorerLens.ps1",
    "install-pure64.cmd",
    "install-x64-fixed.cmd",
    "install-x64.ps1",
    "uninstall-pure64.cmd",
    "uninstall-x64-fixed.cmd",
    "uninstall-x64.ps1",
    "verify-deployment.cmd"
)

foreach ($file in $installScripts) {
    if (Test-Path $file) {
        Move-Item -Path $file -Destination ".\scripts\install\" -Force
        Write-Host "  [OK] Moved: $file" -ForegroundColor Green
    }
}

# 4. Move Test Scripts
Write-Host "Moving test scripts..." -ForegroundColor Yellow
$testScripts = @(
    "Test-ExplorerLens.ps1",
    "run-tests.ps1",
    "build-tests.cmd"
)

foreach ($file in $testScripts) {
    if (Test-Path $file) {
        Move-Item -Path $file -Destination ".\scripts\test\" -Force
        Write-Host "  [OK] Moved: $file" -ForegroundColor Green
    }
}

# 5. Move Release Scripts
Write-Host "Moving release scripts..." -ForegroundColor Yellow
$releaseScripts = @(
    "prepare-release.ps1",
    "validate-release.ps1"
)

foreach ($file in $releaseScripts) {
    if (Test-Path $file) {
        Move-Item -Path $file -Destination ".\scripts\release\" -Force
        Write-Host "  [OK] Moved: $file" -ForegroundColor Green
    }
}

# 6. Move Setup Scripts
Write-Host "Moving setup scripts..." -ForegroundColor Yellow
$setupScripts = @(
    "Initialize-MSVCEnv.ps1",
    "Update-DevTools.ps1",
    "verify-tools.ps1",
    "fix-profile.ps1",
    "setup-msvc-env.ps1"
)

foreach ($file in $setupScripts) {
    if (Test-Path $file) {
        Move-Item -Path $file -Destination ".\scripts\setup\" -Force
        Write-Host "  [OK] Moved: $file" -ForegroundColor Green
    }
}

# 7. Move Status Documentation
Write-Host "Moving status documentation..." -ForegroundColor Yellow
$statusDocs = @(
    "DEPLOYMENT_CHECKLIST.md",
    "DEPLOYMENT_READY.md",
    "ENHANCEMENT_SESSION_2025-11-24.md",
    "EXTERNAL_LIBRARY_INTEGRATION_STATUS.md",
    "FEATURE_REENABLE_STATUS.md",
    "FEATURE_STATUS_COMPLETE.md",
    "FINAL_BUILD_SUMMARY.md",
    "PRODUCTION_BUILD_STATUS.md",
    "PROFILE_SETUP_COMPLETE.md",
    "PURE_64BIT_BUILD_COMPLETE.md",
    "RAR_REENABLE_SUCCESS.md",
    "SESSION_SUMMARY_2025-11-24.md",
    "V5.2.0_STATUS.md",
    "validation-report-2025-11-24-103159.md",
    "validation-report-2025-11-24-103204.md"
)

foreach ($file in $statusDocs) {
    if (Test-Path $file) {
        Move-Item -Path $file -Destination ".\documentation\status\" -Force
        Write-Host "  [OK] Moved: $file" -ForegroundColor Green
    }
}

# 8. Move Guide Documentation
Write-Host "Moving guide documentation..." -ForegroundColor Yellow
$guideDocs = @(
    "LIBRARY_BUILD_GUIDE.md",
    "POWERSHELL_PROFILE_SETUP.md",
    "POWERSHELL_SCRIPTS_README.md",
    "QUICK_START_UPDATED.md",
    "RELEASE_DOCUMENTATION.md",
    "RELEASE_NOTES_v5.1.0.md",
    "TESTING_QUICK_START.md",
    "TOOL_INSTALLATION_GUIDE.md",
    "TOOL_REQUIREMENTS.md",
    "WHATS_NEW.md"
)

foreach ($file in $guideDocs) {
    if (Test-Path $file) {
        Move-Item -Path $file -Destination ".\documentation\guides\" -Force
        Write-Host "  [OK] Moved: $file" -ForegroundColor Green
    }
}

# 9. Move Build Logs and Backups
Write-Host "Moving build logs and backups..." -ForegroundColor Yellow
$buildOutputFiles = @(
    "build.log",
    "build_full.log",
    "profile-backup-20251125-160543.ps1",
    "profile-backup-20251125-160740.ps1"
)

foreach ($file in $buildOutputFiles) {
    if (Test-Path $file) {
        Move-Item -Path $file -Destination ".\build-output\" -Force
        Write-Host "  [OK] Moved: $file" -ForegroundColor Green
    }
}

# 10. Summary
Write-Host ""
Write-Host "=== Reorganization Complete ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "Files remaining in root directory:" -ForegroundColor Yellow
Get-ChildItem -Path . -File | Select-Object -ExpandProperty Name | ForEach-Object { Write-Host "  $_" }
Write-Host ""
Write-Host "Root directory file count: $((Get-ChildItem -Path . -File).Count)" -ForegroundColor Cyan

