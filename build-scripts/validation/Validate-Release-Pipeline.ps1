# Validate-Release-Pipeline.ps1
# Sprint 10: End-to-end release pipeline validation
# Verifies: Build → Test → Package → Sign → Verify workflow
# Version: 1.0.0

param(
    [string]$Version = "7.0.0",
    [switch]$DryRun,
    [switch]$SkipMSI,
    [switch]$SkipPortableZip,
    [switch]$Help
)

$ErrorActionPreference = "Stop"
$script:Steps = @()

function Add-Step {
    param([string]$Name, [string]$Status, [string]$Detail = "")
    $script:Steps += [PSCustomObject]@{
        Name   = $Name
        Status = $Status
        Detail = $Detail
    }
}

function Write-StepHeader { param([string]$Text) Write-Host "`n═══ $Text ═══" -ForegroundColor Cyan }

$RootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Push-Location $RootDir

try {
    Write-Host "=============================================" -ForegroundColor Cyan
    Write-Host "DarkThumbs v$Version - Release Pipeline Validation" -ForegroundColor Cyan
    Write-Host "=============================================" -ForegroundColor Cyan
    if ($DryRun) { Write-Host "(DRY RUN - no artifacts will be created)" -ForegroundColor Yellow }
    Write-Host ""

    # =========================================================================
    # Step 1: Pre-flight checks
    # =========================================================================
    Write-StepHeader "Step 1: Pre-flight Checks"

    # Check MSBuild
    $msbuild = Get-Command msbuild -ErrorAction SilentlyContinue
    if ($msbuild) {
        Add-Step "MSBuild available" "PASS" $msbuild.Source
        Write-Host "  ✓ MSBuild: $($msbuild.Source)" -ForegroundColor Green
    } else {
        Add-Step "MSBuild available" "WARN" "Not in PATH - run from VS Developer Shell"
        Write-Host "  ⚠ MSBuild not in PATH (run from VS Developer Shell)" -ForegroundColor Yellow
    }

    # Check CMake
    $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmake) {
        $cmakeVer = (cmake --version | Select-Object -First 1) -replace 'cmake version ', ''
        Add-Step "CMake available" "PASS" "v$cmakeVer"
        Write-Host "  ✓ CMake: v$cmakeVer" -ForegroundColor Green
    } else {
        Add-Step "CMake available" "FAIL" "Not found"
        Write-Host "  ✗ CMake not found" -ForegroundColor Red
    }

    # Check SignTool
    $signtool = Get-Command signtool -ErrorAction SilentlyContinue
    if ($signtool) {
        Add-Step "SignTool available" "PASS" $signtool.Source
        Write-Host "  ✓ SignTool: $($signtool.Source)" -ForegroundColor Green
    } else {
        Add-Step "SignTool available" "WARN" "Not found - signing will be skipped"
        Write-Host "  ⚠ SignTool not found (signing will be skipped)" -ForegroundColor Yellow
    }

    # Check WiX
    $candle = Get-Command candle -ErrorAction SilentlyContinue
    $light = Get-Command light -ErrorAction SilentlyContinue
    if ($candle -and $light) {
        Add-Step "WiX Toolset available" "PASS"
        Write-Host "  ✓ WiX Toolset available" -ForegroundColor Green
    } else {
        Add-Step "WiX Toolset available" "WARN" "Not found - MSI build will be skipped"
        Write-Host "  ⚠ WiX Toolset not found (MSI build will be skipped)" -ForegroundColor Yellow
    }

    # =========================================================================
    # Step 2: Build artifact verification
    # =========================================================================
    Write-StepHeader "Step 2: Build Artifact Verification"

    $artifacts = @(
        @{ Path = "x64\Release\CBXShell.dll"; MinMB = 2.5; Desc = "Shell Extension DLL" }
        @{ Path = "x64\Release\CBXManager.exe"; MinMB = 0.3; Desc = "Manager Application" }
        @{ Path = "build\lib\DarkThumbsEngine.lib"; MinMB = 90; Desc = "Engine Static Library" }
    )

    foreach ($art in $artifacts) {
        $fullPath = Join-Path $RootDir $art.Path
        if (Test-Path $fullPath) {
            $sizeMB = [math]::Round((Get-Item $fullPath).Length / 1MB, 2)
            if ($sizeMB -ge $art.MinMB) {
                Add-Step "$($art.Desc)" "PASS" "$sizeMB MB"
                Write-Host "  ✓ $($art.Desc): $sizeMB MB" -ForegroundColor Green
            } else {
                Add-Step "$($art.Desc)" "FAIL" "$sizeMB MB (min: $($art.MinMB) MB)"
                Write-Host "  ✗ $($art.Desc): $sizeMB MB (expected ≥$($art.MinMB) MB)" -ForegroundColor Red
            }
        } else {
            Add-Step "$($art.Desc)" "FAIL" "Not found: $($art.Path)"
            Write-Host "  ✗ $($art.Desc) not found: $($art.Path)" -ForegroundColor Red
        }
    }

    # =========================================================================
    # Step 3: Test suite gate
    # =========================================================================
    Write-StepHeader "Step 3: Test Suite Gate"

    $testExe = Join-Path $RootDir "build\bin\Release\DarkThumbsTests.exe"
    if (Test-Path $testExe) {
        if (-not $DryRun) {
            Write-Host "  Running tests..." -ForegroundColor Gray
            $testOutput = & $testExe --gtest_brief=1 2>&1
            $lastExitCode = $LASTEXITCODE
            if ($lastExitCode -eq 0) {
                Add-Step "Unit tests" "PASS" "All tests passed"
                Write-Host "  ✓ Unit tests: ALL PASSED" -ForegroundColor Green
            } else {
                Add-Step "Unit tests" "FAIL" "Exit code: $lastExitCode"
                Write-Host "  ✗ Unit tests FAILED (exit code: $lastExitCode)" -ForegroundColor Red
            }
        } else {
            Add-Step "Unit tests" "SKIP" "Dry run"
            Write-Host "  ○ Unit tests: SKIPPED (dry run)" -ForegroundColor Gray
        }
    } else {
        Add-Step "Unit tests" "WARN" "Test binary not found"
        Write-Host "  ⚠ Test binary not found: $testExe" -ForegroundColor Yellow
    }

    # =========================================================================
    # Step 4: Version consistency
    # =========================================================================
    Write-StepHeader "Step 4: Version Consistency"

    $versionFiles = @(
        @{ Path = "README.md"; Pattern = "Current Version.*$Version" }
        @{ Path = "MASTER_PLAN.md"; Pattern = "v$Version" }
        @{ Path = "CHANGELOG.md"; Pattern = $Version }
        @{ Path = "packaging\DarkThumbs.wxs"; Pattern = $Version }
        @{ Path = ".github\standards\IMPLEMENTATION_STATUS.md"; Pattern = "v$Version" }
    )

    foreach ($vf in $versionFiles) {
        $fullPath = Join-Path $RootDir $vf.Path
        if (Test-Path $fullPath) {
            $content = Get-Content $fullPath -Raw
            if ($content -match $vf.Pattern) {
                Add-Step "Version in $($vf.Path)" "PASS"
                Write-Host "  ✓ $($vf.Path)" -ForegroundColor Green
            } else {
                Add-Step "Version in $($vf.Path)" "FAIL" "Pattern not found: $($vf.Pattern)"
                Write-Host "  ✗ $($vf.Path): version $Version not found" -ForegroundColor Red
            }
        } else {
            Add-Step "Version in $($vf.Path)" "WARN" "File not found"
            Write-Host "  ⚠ $($vf.Path) not found" -ForegroundColor Yellow
        }
    }

    # =========================================================================
    # Step 5: Packaging validation
    # =========================================================================
    Write-StepHeader "Step 5: Packaging Validation"

    # MSI check
    if (-not $SkipMSI) {
        $msiPath = Join-Path $RootDir "packaging\DarkThumbs-Setup-$Version.msi"
        if (Test-Path $msiPath) {
            $msiSizeMB = [math]::Round((Get-Item $msiPath).Length / 1MB, 2)
            Add-Step "MSI installer" "PASS" "$msiSizeMB MB"
            Write-Host "  ✓ MSI installer: $msiSizeMB MB" -ForegroundColor Green
        } else {
            Add-Step "MSI installer" "WARN" "Not built yet"
            Write-Host "  ⚠ MSI not found: $msiPath" -ForegroundColor Yellow
        }
    }

    # Portable ZIP check
    if (-not $SkipPortableZip) {
        $zipPath = Join-Path $RootDir "packaging\output\DarkThumbs-$Version-Portable.zip"
        if (Test-Path $zipPath) {
            $zipSizeMB = [math]::Round((Get-Item $zipPath).Length / 1MB, 2)
            Add-Step "Portable ZIP" "PASS" "$zipSizeMB MB"
            Write-Host "  ✓ Portable ZIP: $zipSizeMB MB" -ForegroundColor Green
        } else {
            Add-Step "Portable ZIP" "WARN" "Not built yet"
            Write-Host "  ⚠ Portable ZIP not found" -ForegroundColor Yellow
        }
    }

    # =========================================================================
    # Step 6: Code signing verification
    # =========================================================================
    Write-StepHeader "Step 6: Code Signing Verification"

    $dllPath = Join-Path $RootDir "x64\Release\CBXShell.dll"
    if ((Test-Path $dllPath) -and (Get-Command signtool -ErrorAction SilentlyContinue)) {
        $sigResult = signtool verify /pa $dllPath 2>&1
        if ($LASTEXITCODE -eq 0) {
            Add-Step "CBXShell.dll signed" "PASS"
            Write-Host "  ✓ CBXShell.dll: Valid signature" -ForegroundColor Green
        } else {
            Add-Step "CBXShell.dll signed" "WARN" "Not signed"
            Write-Host "  ⚠ CBXShell.dll: Not signed (sign before release)" -ForegroundColor Yellow
        }
    } else {
        Add-Step "Code signing check" "SKIP" "signtool or DLL not available"
        Write-Host "  ○ Code signing check skipped" -ForegroundColor Gray
    }

    # =========================================================================
    # Step 7: CI workflow files validation
    # =========================================================================
    Write-StepHeader "Step 7: CI Pipeline Files"

    $workflows = @(
        ".github\workflows\build.yml"
        ".github\workflows\build-v7.yml"
        ".github\workflows\build-and-test.yml"
        ".github\workflows\release.yml"
        ".github\workflows\code-quality.yml"
        ".github\workflows\performance-regression-gate.yml"
    )

    foreach ($wf in $workflows) {
        $wfPath = Join-Path $RootDir $wf
        if (Test-Path $wfPath) {
            Add-Step "CI: $(Split-Path -Leaf $wf)" "PASS"
            Write-Host "  ✓ $(Split-Path -Leaf $wf)" -ForegroundColor Green
        } else {
            Add-Step "CI: $(Split-Path -Leaf $wf)" "FAIL" "Missing"
            Write-Host "  ✗ $(Split-Path -Leaf $wf) MISSING" -ForegroundColor Red
        }
    }

    # =========================================================================
    # Summary
    # =========================================================================
    Write-Host ""
    Write-Host "=============================================" -ForegroundColor Cyan
    Write-Host "Release Pipeline Validation Summary" -ForegroundColor Cyan
    Write-Host "=============================================" -ForegroundColor Cyan

    $pass = ($script:Steps | Where-Object { $_.Status -eq "PASS" }).Count
    $fail = ($script:Steps | Where-Object { $_.Status -eq "FAIL" }).Count
    $warn = ($script:Steps | Where-Object { $_.Status -eq "WARN" }).Count
    $skip = ($script:Steps | Where-Object { $_.Status -eq "SKIP" }).Count

    Write-Host "  PASS: $pass" -ForegroundColor Green
    Write-Host "  FAIL: $fail" -ForegroundColor $(if ($fail -gt 0) { "Red" } else { "Green" })
    Write-Host "  WARN: $warn" -ForegroundColor $(if ($warn -gt 0) { "Yellow" } else { "Green" })
    Write-Host "  SKIP: $skip" -ForegroundColor Gray

    if ($fail -eq 0) {
        Write-Host "`n✅ Release pipeline validation PASSED" -ForegroundColor Green
        Write-Host "   Ready for v$Version release candidate." -ForegroundColor Green
    } else {
        Write-Host "`n❌ Release pipeline validation FAILED ($fail issues)" -ForegroundColor Red
        Write-Host "   Fix failures before creating release candidate." -ForegroundColor Red
    }

} finally {
    Pop-Location
}
