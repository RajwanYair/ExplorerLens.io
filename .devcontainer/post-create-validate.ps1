<#
.SYNOPSIS
    Dev container post-create validation script for ExplorerLens.

.DESCRIPTION
    Runs after the dev container is created to:
    1. Validate all required build tools are available
    2. Configure git hooks
    3. Run a cmake configure-only dry-run to verify the toolchain
    4. Print actionable fix instructions for any missing components

    ROADMAP reference: §13.1 dev container clone-to-build test

.NOTES
    Called by devcontainer.json postCreateCommand.
    Must be PowerShell-compatible (pwsh 7+).
#>

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Write-Banner {
    param([string]$Text)
    $line = "=" * 60
    Write-Host "" -ForegroundColor White
    Write-Host $line -ForegroundColor DarkCyan
    Write-Host "  $Text" -ForegroundColor Cyan
    Write-Host $line -ForegroundColor DarkCyan
    Write-Host "" -ForegroundColor White
}

function Write-Step {
    param([string]$Number, [string]$Text)
    Write-Host "[$Number] $Text" -ForegroundColor Yellow
}

function Write-OK {
    param([string]$Text)
    Write-Host "  OK  $Text" -ForegroundColor Green
}

function Write-Fail {
    param([string]$Text, [string]$Fix = "")
    Write-Host "  FAIL  $Text" -ForegroundColor Red
    if ($Fix) {
        Write-Host "       Fix: $Fix" -ForegroundColor DarkYellow
    }
}

Write-Banner "ExplorerLens Dev Container — Post-Create Validation"
Write-Host "  Version: $(Get-Content VERSION -ErrorAction SilentlyContinue)" -ForegroundColor White
Write-Host "  Date:    $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss UTC' -AsUTC)" -ForegroundColor White
Write-Host ""

$allOk = $true

# ─────────────────────────────────────────────────────────────────
# Step 1 — Validate required build tools
# ─────────────────────────────────────────────────────────────────
Write-Step "1/4" "Validating required build tools"

$requiredTools = @(
    @{ Name = "cmake";   Fix = "Install via: scoop install cmake  OR  winget install Kitware.CMake" },
    @{ Name = "ninja";   Fix = "Install via: scoop install ninja  OR  winget install Ninja-build.Ninja" },
    @{ Name = "ctest";   Fix = "Comes with cmake — re-install cmake if missing" },
    @{ Name = "pwsh";    Fix = "Install via: winget install Microsoft.PowerShell" }
)

$optionalTools = @(
    @{ Name = "cl";      Fix = "Install VS 2022 BuildTools: winget install Microsoft.VisualStudio.2022.BuildTools" },
    @{ Name = "link";    Fix = "Part of MSVC toolset — install with VS 2022 BuildTools" },
    @{ Name = "nasm";    Fix = "Install via: scoop install nasm  (needed for libAV1/libjpeg-turbo)" },
    @{ Name = "7z";      Fix = "Install via: scoop install 7zip  (needed for extracting external libraries)" },
    @{ Name = "git";     Fix = "Install via: winget install Git.Git" }
)

foreach ($tool in $requiredTools) {
    if (Get-Command $tool.Name -ErrorAction SilentlyContinue) {
        $ver = & $tool.Name --version 2>$null | Select-Object -First 1
        Write-OK "$($tool.Name)  $ver"
    } else {
        Write-Fail "$($tool.Name) — NOT FOUND" $tool.Fix
        $allOk = $false
    }
}

Write-Host ""
Write-Host "  Optional tools:" -ForegroundColor DarkGray
foreach ($tool in $optionalTools) {
    if (Get-Command $tool.Name -ErrorAction SilentlyContinue) {
        Write-OK "$($tool.Name)  (available)"
    } else {
        Write-Host "  WARN  $($tool.Name) — not found (not required for configure-only run)" -ForegroundColor DarkYellow
        Write-Host "        Fix: $($tool.Fix)" -ForegroundColor DarkGray
    }
}

# ─────────────────────────────────────────────────────────────────
# Step 2 — Configure git hooks
# ─────────────────────────────────────────────────────────────────
Write-Host ""
Write-Step "2/4" "Configuring git hooks"

if (Test-Path ".githooks") {
    git config core.hooksPath .githooks 2>&1 | Out-Null
    Write-OK "git hooks configured from .githooks/"
} else {
    Write-Host "  INFO  .githooks/ not present — skipping" -ForegroundColor DarkGray
}

# ─────────────────────────────────────────────────────────────────
# Step 3 — CMake configure-only dry-run (no build)
# ─────────────────────────────────────────────────────────────────
Write-Host ""
Write-Step "3/4" "CMake configure-only validation"

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "  SKIP  cmake not available — skipping configure test" -ForegroundColor DarkYellow
} else {
    try {
        # Use the default-release preset if available; fall back to manual configure
        $presets = cmake --list-presets 2>$null
        $hasPreset = $presets -match "default-release"

        if ($hasPreset) {
            Write-Host "  Using cmake preset: default-release" -ForegroundColor DarkGray
            $result = cmake --preset default-release --log-level=WARNING 2>&1
        } else {
            Write-Host "  Using fallback cmake configure" -ForegroundColor DarkGray
            New-Item -ItemType Directory -Path "build" -Force | Out-Null
            $result = cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release 2>&1
        }

        if ($LASTEXITCODE -eq 0) {
            Write-OK "cmake configure succeeded"
        } else {
            Write-Fail "cmake configure failed (exit code $LASTEXITCODE)" `
                "Check that MSVC BuildTools are installed: winget install Microsoft.VisualStudio.2022.BuildTools"
            $allOk = $false
            Write-Host ""
            Write-Host "  cmake output (last 20 lines):" -ForegroundColor DarkGray
            $result | Select-Object -Last 20 | ForEach-Object { Write-Host "    $_" -ForegroundColor DarkGray }
        }
    } catch {
        Write-Fail "cmake configure threw an exception: $_" ""
        $allOk = $false
    }
}

# ─────────────────────────────────────────────────────────────────
# Step 4 — Print quick-start instructions
# ─────────────────────────────────────────────────────────────────
Write-Host ""
Write-Step "4/4" "Quick-start build instructions"
Write-Host ""
Write-Host "  Full build + tests:" -ForegroundColor White
Write-Host "    .\build-scripts\Build-MSVC.ps1 -Clean -Test" -ForegroundColor Cyan
Write-Host ""
Write-Host "  CMake only:" -ForegroundColor White
Write-Host "    cmake --preset default-release" -ForegroundColor Cyan
Write-Host "    cmake --build build --config Release" -ForegroundColor Cyan
Write-Host ""
Write-Host "  Run tests:" -ForegroundColor White
Write-Host "    ctest --test-dir build -C Release --output-on-failure" -ForegroundColor Cyan
Write-Host ""
Write-Host "  Shell extension (MSBuild):" -ForegroundColor White
Write-Host "    msbuild LENSShell.sln /p:Configuration=Release /p:Platform=x64" -ForegroundColor Cyan
Write-Host ""

# ─────────────────────────────────────────────────────────────────
# Final result
# ─────────────────────────────────────────────────────────────────
Write-Host ""
if ($allOk) {
    Write-Host "  Dev container validation PASSED — ready to build." -ForegroundColor Green
    exit 0
} else {
    Write-Host "  Dev container validation FAILED — see FAIL items above." -ForegroundColor Red
    Write-Host "  Install missing tools and re-run:  .devcontainer\post-create-validate.ps1" -ForegroundColor DarkYellow
    exit 1
}
