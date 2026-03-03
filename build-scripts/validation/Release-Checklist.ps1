# Release-Checklist.ps1
# Comprehensive release readiness verification script
# Version: 1.0.0

param(
    [switch]$SkipBuild = $false,
    [switch]$SkipTests = $false,
    [switch]$SkipDocs = $false,
    [switch]$SkipPackaging = $false,
    [string]$Version = "15.0.0"
)

$ErrorActionPreference = "Stop"
$script:ChecksPassed = 0
$script:ChecksFailed = 0
$script:ChecksWarned = 0
$script:StartTime = Get-Date

# Color output helpers
function Write-CheckHeader { param([string]$Text) Write-Host "`n[$Text]" -ForegroundColor Cyan -NoNewline; Write-Host " ..." }
function Write-Pass { param([string]$Text) Write-Host "  ✓ " -ForegroundColor Green -NoNewline; Write-Host $Text; $script:ChecksPassed++ }
function Write-Fail { param([string]$Text) Write-Host "  ✗ " -ForegroundColor Red -NoNewline; Write-Host $Text; $script:ChecksFailed++ }
function Write-Warn { param([string]$Text) Write-Host "  ⚠ " -ForegroundColor Yellow -NoNewline; Write-Host $Text; $script:ChecksWarned++ }

Write-Host "============================================" -ForegroundColor Cyan
Write-Host "ExplorerLens v$Version Release Checklist" -ForegroundColor Cyan
Write-Host ": Release Governance & Packaging" -ForegroundColor Cyan
Write-Host "============================================`n" -ForegroundColor Cyan

$RootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Push-Location $RootDir

try {
    # =============================================================================
    # 1. VERSION CONSISTENCY CHECK
    # =============================================================================
    Write-CheckHeader "1. Version Consistency"

    $VersionFiles = @{
        "MASTER_PLAN.md"               = $Version
        "README.md"                    = $Version
        "CHANGELOG.md"                 = $Version
        "packaging/ExplorerLens.wxs"   = $Version
        "LENSShell/LENSShellClass.cpp" = $Version
    }

    $VersionMismatches = @()
    foreach ($file in $VersionFiles.Keys) {
        $expectedVersion = $VersionFiles[$file]
        $fullPath = Join-Path $RootDir $file

        if (Test-Path $fullPath) {
            $content = Get-Content $fullPath -Raw
            if ($content -match $expectedVersion) {
                Write-Pass "$file contains version $expectedVersion"
            } else {
                Write-Fail "$file missing version $expectedVersion"
                $VersionMismatches += $file
            }
        } else {
            Write-Warn "$file not found"
        }
    }

    # =============================================================================
    # 2. BUILD ARTIFACTS CHECK
    # =============================================================================
    if (-not $SkipBuild) {
        Write-CheckHeader "2. Build Artifacts"

        $BuildArtifacts = @{
            "x64/Release/LENSShell.dll"        = 2800000  # ~2.8 MB minimum
            "x64/Release/LENSManager.exe"      = 350000  # ~350 KB minimum
            "build/lib/ExplorerLensEngine.lib" = 100000000  # ~100 MB minimum
        }

        foreach ($artifact in $BuildArtifacts.Keys) {
            $artifactPath = Join-Path $RootDir $artifact
            $minSize = $BuildArtifacts[$artifact]

            if (Test-Path $artifactPath) {
                $fileInfo = Get-Item $artifactPath
                $sizeMB = [math]::Round($fileInfo.Length / 1MB, 2)

                if ($fileInfo.Length -ge $minSize) {
                    Write-Pass "$artifact exists ($sizeMB MB)"
                } else {
                    Write-Fail "$artifact too small ($sizeMB MB, expected ≥$([math]::Round($minSize/1MB, 2)) MB)"
                }
            } else {
                Write-Fail "$artifact not found"
            }
        }

        # Check for debug symbols
        $pdbFiles = @("x64/Release/LENSShell.pdb", "x64/Release/LENSManager.pdb")
        foreach ($pdb in $pdbFiles) {
            $pdbPath = Join-Path $RootDir $pdb
            if (Test-Path $pdbPath) {
                Write-Pass "Debug symbols: $pdb"
            } else {
                Write-Warn "Debug symbols not found: $pdb"
            }
        }
    }

    # =============================================================================
    # 3. TEST SUITE EXECUTION
    # =============================================================================
    if (-not $SkipTests) {
        Write-CheckHeader "3. Test Suite Execution"

        $testExecutable = Join-Path $RootDir "build\bin\Release\ExplorerLensTests.exe"
        if (Test-Path $testExecutable) {
            Write-Host "  Running test suite..." -ForegroundColor Gray

            try {
                $testOutput = & $testExecutable 2>&1
                $testExitCode = $LASTEXITCODE

                if ($testExitCode -eq 0) {
                    # Parse test results
                    $testResults = $testOutput | Select-String "(\d+) tests? .* (\d+) assertions?"
                    if ($testResults) {
                        Write-Pass "All tests passed: $($testResults.Matches[0].Value)"
                    } else {
                        Write-Pass "Test suite completed successfully"
                    }
                } else {
                    Write-Fail "Test suite failed with exit code $testExitCode"
                }
            } catch {
                Write-Fail "Failed to execute test suite: $_"
            }
        } else {
            Write-Warn "Test executable not found at: $testExecutable"
        }

        # Check for test coverage (if available)
        $coverageFile = Join-Path $RootDir "build\coverage\coverage.xml"
        if (Test-Path $coverageFile) {
            Write-Pass "Code coverage report available"
        } else {
            Write-Warn "Code coverage report not generated"
        }
    }

    # =============================================================================
    # 4. DOCUMENTATION INTEGRITY
    # =============================================================================
    if (-not $SkipDocs) {
        Write-CheckHeader "4. Documentation Integrity"

        # Check required documentation files
        $RequiredDocs = @(
            "README.md",
            "CHANGELOG.md",
            "USER_GUIDE.md",
            "DEVELOPER_GUIDE.md",
            "QUICK_BUILD_REFERENCE.md",
            "KNOWN_ISSUES.md",
            "MASTER_PLAN.md"
        )

        foreach ($doc in $RequiredDocs) {
            $docPath = Join-Path $RootDir $doc
            if (Test-Path $docPath) {
                $content = Get-Content $docPath -Raw

                # Check for broken internal links (basic check)
                $brokenLinks = @()
                $linkPattern = '\[([^\]]+)\]\(([^)]+)\)'
                $matches = [regex]::Matches($content, $linkPattern)

                foreach ($match in $matches) {
                    $linkPath = $match.Groups[2].Value

                    # Skip external links and anchors
                    if ($linkPath -notmatch '^https?://' -and $linkPath -notmatch '^#') {
                        $fullLinkPath = Join-Path (Split-Path $docPath) $linkPath
                        if (-not (Test-Path $fullLinkPath)) {
                            $brokenLinks += $linkPath
                        }
                    }
                }

                if ($brokenLinks.Count -eq 0) {
                    Write-Pass "$doc (no broken links detected)"
                } else {
                    Write-Warn "$doc has $($brokenLinks.Count) potentially broken link(s)"
                }
            } else {
                Write-Fail "$doc not found"
            }
        }

        # Check for release notes
        $releaseNotesPath = Join-Path $RootDir "docs\release-notes\RELEASE_NOTES_v$Version.md"
        if (Test-Path $releaseNotesPath) {
            Write-Pass "Release notes for v$Version exist"
        } else {
            Write-Warn "Release notes not found: RELEASE_NOTES_v$Version.md"
        }
    }

    # =============================================================================
    # 5. PACKAGING READINESS
    # =============================================================================
    if (-not $SkipPackaging) {
        Write-CheckHeader "5. Packaging Readiness"

        # Check MSI installer
        $msiPath = Join-Path $RootDir "packaging\ExplorerLens-Setup-$Version.msi"
        if (Test-Path $msiPath) {
            $msiSize = [math]::Round((Get-Item $msiPath).Length / 1MB, 2)
            Write-Pass "MSI installer exists ($msiSize MB)"
        } else {
            Write-Warn "MSI installer not found at: ExplorerLens-Setup-$Version.msi"
        }

        # Check portable ZIP
        $zipPath = Join-Path $RootDir "packaging\output\ExplorerLens-$Version-Portable.zip"
        if (Test-Path $zipPath) {
            $zipSize = [math]::Round((Get-Item $zipPath).Length / 1MB, 2)
            Write-Pass "Portable ZIP exists ($zipSize MB)"
        } else {
            Write-Warn "Portable ZIP not found"
        }

        # Check for WiX Toolset
        $wixCommand = Get-Command wix -ErrorAction SilentlyContinue
        if ($wixCommand) {
            Write-Pass "WiX Toolset available"
        } else {
            Write-Warn "WiX Toolset not found in PATH"
        }

        # Check for code signing certificate
        $certPath = Join-Path $RootDir "certs\ExplorerLens-codesign.pfx"
        if (Test-Path $certPath) {
            Write-Pass "Code signing certificate available"
        } else {
            Write-Warn "Code signing certificate not configured"
        }
    }

    # =============================================================================
    # 6. GIT REPOSITORY STATUS
    # =============================================================================
    Write-CheckHeader "6. Git Repository Status"

    try {
        # Check if git is available
        $gitCommand = Get-Command git -ErrorAction SilentlyContinue
        if ($gitCommand) {
            # Check for uncommitted changes
            $gitStatus = & git status --porcelain 2>&1
            if ([string]::IsNullOrWhiteSpace($gitStatus)) {
                Write-Pass "Working tree is clean"
            } else {
                Write-Warn "Uncommitted changes detected"
            }

            # Check current branch
            $currentBranch = & git rev-parse --abbrev-ref HEAD 2>&1
            if ($currentBranch -eq "main" -or $currentBranch -eq "master") {
                Write-Pass "On release branch: $currentBranch"
            } else {
                Write-Warn "Not on main/master branch (current: $currentBranch)"
            }

            # Check for version tag
            $versionTag = "v$Version"
            $tagExists = & git tag -l $versionTag 2>&1
            if ($tagExists) {
                Write-Pass "Git tag exists: $versionTag"
            } else {
                Write-Warn "Git tag not created: $versionTag"
            }
        } else {
            Write-Warn "Git command not available"
        }
    } catch {
        Write-Warn "Git checks failed: $_"
    }

    # =============================================================================
    # 7. EXTERNAL DEPENDENCIES
    # =============================================================================
    Write-CheckHeader "7. External Dependencies"

    # Check for required DLLs
    $RequiredDLLs = @(
        "x64/Release/zlib1.dll",
        "x64/Release/libde265.dll"
    )

    foreach ($dll in $RequiredDLLs) {
        $dllPath = Join-Path $RootDir $dll
        if (Test-Path $dllPath) {
            Write-Pass "$dll available"
        } else {
            Write-Warn "$dll not found (may be statically linked)"
        }
    }

    # Check vcpkg manifest
    $vcpkgManifest = Join-Path $RootDir "vcpkg.json"
    if (Test-Path $vcpkgManifest) {
        Write-Pass "vcpkg.json manifest exists"
    } else {
        Write-Warn "vcpkg.json manifest not found"
    }

    # =============================================================================
    # 8. SECURITY CHECKS
    # =============================================================================
    Write-CheckHeader "8. Security Checks"

    # Check for hardcoded credentials (basic scan)
    Write-Host "  Scanning for potential security issues..." -ForegroundColor Gray
    $securityPatterns = @(
        'password\s*=\s*[''"]',
        'api[_-]?key\s*=\s*[''"]',
        'secret\s*=\s*[''"]'
    )

    $securityIssues = @()
    Get-ChildItem -Path $RootDir -Recurse -Include *.cpp, *.h, *.ps1, *.json -ErrorAction SilentlyContinue | ForEach-Object {
        $content = Get-Content $_.FullName -Raw -ErrorAction SilentlyContinue
        foreach ($pattern in $securityPatterns) {
            if ($content -match $pattern) {
                $securityIssues += $_.FullName
                break
            }
        }
    }

    if ($securityIssues.Count -eq 0) {
        Write-Pass "No obvious credentials in source code"
    } else {
        Write-Warn "Potential credentials found in $($securityIssues.Count) file(s)"
    }

    # =============================================================================
    # SUMMARY
    # =============================================================================
    $duration = (Get-Date) - $script:StartTime

    Write-Host "`n============================================" -ForegroundColor Cyan
    Write-Host "Release Checklist Summary" -ForegroundColor Cyan
    Write-Host "============================================" -ForegroundColor Cyan
    Write-Host "  Passed:    " -NoNewline; Write-Host $script:ChecksPassed -ForegroundColor Green
    Write-Host "  Failed:    " -NoNewline; Write-Host $script:ChecksFailed -ForegroundColor Red
    Write-Host "  Warnings:  " -NoNewline; Write-Host $script:ChecksWarned -ForegroundColor Yellow
    Write-Host "  Duration:  $($duration.TotalSeconds.ToString('F2')) seconds" -ForegroundColor Gray
    Write-Host "============================================`n" -ForegroundColor Cyan

    if ($script:ChecksFailed -eq 0) {
        Write-Host "✓ Release checklist PASSED" -ForegroundColor Green
        Write-Host "  Ready to proceed with release" -ForegroundColor Green
        exit 0
    } else {
        Write-Host "✗ Release checklist FAILED" -ForegroundColor Red
        Write-Host "  Address failures before releasing" -ForegroundColor Red
        exit 1
    }

} finally {
    Pop-Location
}
