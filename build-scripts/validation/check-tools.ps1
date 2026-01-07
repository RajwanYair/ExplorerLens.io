# Quick Tool Check
Write-Host "=== Development Tools Check ===" -ForegroundColor Cyan
Write-Host ""

# Git
try {
    $gitVer = git --version 2>&1
    Write-Host "[OK] Git: $gitVer" -ForegroundColor Green
} catch {
    Write-Host "[FAIL] Git: Not found" -ForegroundColor Red
}

# CMake
try {
    $cmakeVer = (cmake --version 2>&1 | Select-Object -First 1)
    Write-Host "[OK] CMake: $cmakeVer" -ForegroundColor Green
} catch {
    Write-Host "[FAIL] CMake: Not found" -ForegroundColor Red
}

# Ninja
try {
    $ninjaVer = ninja --version 2>&1
    Write-Host "[OK] Ninja: $ninjaVer" -ForegroundColor Green
} catch {
    Write-Host "[FAIL] Ninja: Not found" -ForegroundColor Red
}

# Python
try {
    $pythonVer = python --version 2>&1
    Write-Host "[OK] Python: $pythonVer" -ForegroundColor Green
} catch {
    Write-Host "[FAIL] Python: Not found" -ForegroundColor Red
}

# Visual Studio Build Tools
$vsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
if (Test-Path $vsPath) {
    Write-Host "[OK] Visual Studio BuildTools 2026: Found at $vsPath" -ForegroundColor Green
} else {
    Write-Host "[FAIL] Visual Studio BuildTools 2026: Not found" -ForegroundColor Red
}

# MSBuild
try {
    $msbuildVer = (msbuild -version 2>&1 | Select-Object -Last 1)
    Write-Host "[OK] MSBuild: $msbuildVer" -ForegroundColor Green
} catch {
    Write-Host "[FAIL] MSBuild: Not found in PATH (requires vcvarsall.bat)" -ForegroundColor Yellow
}

# cl.exe (MSVC Compiler)
try {
    $clPath = (where.exe cl.exe 2>&1 | Select-Object -First 1)
    if ($clPath -and $clPath -notmatch "could not find") {
        Write-Host "[OK] MSVC Compiler (cl.exe): Found at $clPath" -ForegroundColor Green
    } else {
        Write-Host "[FAIL] MSVC Compiler (cl.exe): Not in PATH (requires vcvarsall.bat)" -ForegroundColor Yellow
    }
} catch {
    Write-Host "[FAIL] MSVC Compiler (cl.exe): Not in PATH (requires vcvarsall.bat)" -ForegroundColor Yellow
}

# GCC
try {
    $gccVer = gcc --version 2>&1 | Select-Object -First 1
    Write-Host "[OK] GCC: $gccVer" -ForegroundColor Green
} catch {
    Write-Host "[WARN]  GCC: Not found (optional)" -ForegroundColor Yellow
}

# Scoop
if (Test-Path "C:\Users\ryair\scoop") {
    Write-Host "[OK] Scoop Package Manager: Installed" -ForegroundColor Green
} else {
    Write-Host "[FAIL] Scoop Package Manager: Not found" -ForegroundColor Red
}

Write-Host ""
Write-Host "=== Summary ===" -ForegroundColor Cyan
Write-Host "Core tools (Git, CMake, Ninja, Python, VS BuildTools) should all be [OK]"
Write-Host "MSVC tools (MSBuild, cl.exe) require running vcvarsall.bat or using Developer PowerShell"
Write-Host ""
