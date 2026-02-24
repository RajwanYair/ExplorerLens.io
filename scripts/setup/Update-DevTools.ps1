# Update All Development Tools
# This script updates all development tools to their latest versions

param(
    [switch]$Force,
    [switch]$InstallMissing
)

Write-Host "+===============================================================+" -ForegroundColor Cyan
Write-Host "|      ExplorerLens Development Tools Update Script              |" -ForegroundColor Cyan
Write-Host "+===============================================================+" -ForegroundColor Cyan
Write-Host ""

# Function to check if a command exists
function Test-CommandExists {
    param($Command)
    $null -ne (Get-Command $Command -ErrorAction SilentlyContinue)
}

# 1. Update Scoop itself
Write-Host "[PACKAGE] Updating Scoop package manager..." -ForegroundColor Yellow
scoop update

Write-Host ""
Write-Host "[PUSHPIN] Checking installed packages..." -ForegroundColor Yellow
scoop status

Write-Host ""
Write-Host "^  Updating all Scoop packages..." -ForegroundColor Yellow
scoop update *

# 2. List of essential development tools
$tools = @{
    'git' = 'Version control system'
    'cmake' = 'Build system generator'
    'ninja' = 'Fast build system'
    'python' = 'Python programming language'
    'perl' = 'Perl programming language'
    'mingw' = 'MinGW GCC compiler collection'
    '7zip' = 'Archive utility'
    'make' = 'GNU Make utility'
    'llvm' = 'LLVM compiler toolchain'
    'nasm' = 'Netwide Assembler'
    'yasm' = 'Modular assembler'
}

# 3. Check and optionally install missing tools
Write-Host ""
Write-Host "[PUSHPIN] Checking for missing tools..." -ForegroundColor Yellow

$missing = @()
foreach ($tool in $tools.Keys) {
    $installed = scoop list | Select-String -Pattern "^\s*$tool\s"
    if (-not $installed) {
        $missing += $tool
        Write-Host "  [FAIL] $tool - Not installed ($($tools[$tool]))" -ForegroundColor Red
    } else {
        Write-Host "  [OK] $tool - Installed" -ForegroundColor Green
    }
}

if ($missing.Count -gt 0) {
    Write-Host ""
    if ($InstallMissing) {
        Write-Host "[INBOX] Installing missing tools..." -ForegroundColor Yellow
        foreach ($tool in $missing) {
            Write-Host "  Installing $tool..." -ForegroundColor Cyan
            scoop install $tool
        }
    } else {
        Write-Host "[WARN]  Found $($missing.Count) missing tools. Run with -InstallMissing to install them:" -ForegroundColor Yellow
        foreach ($tool in $missing) {
            Write-Host "  - $tool : $($tools[$tool])" -ForegroundColor Gray
        }
        Write-Host ""
        Write-Host "To install: .\Update-DevTools.ps1 -InstallMissing" -ForegroundColor Cyan
    }
}

# 4. Verify key tools are accessible
Write-Host ""
Write-Host "[PUSHPIN] Verifying tool versions..." -ForegroundColor Yellow

$verifications = @(
    @{Name='Git'; Cmd='git'; Args='--version'},
    @{Name='CMake'; Cmd='cmake'; Args='--version'},
    @{Name='Ninja'; Cmd='ninja'; Args='--version'},
    @{Name='Python'; Cmd='python'; Args='--version'},
    @{Name='Perl'; Cmd='perl'; Args='--version'},
    @{Name='GCC (MinGW)'; Cmd='gcc'; Args='--version'},
    @{Name='G++ (MinGW)'; Cmd='g++'; Args='--version'},
    @{Name='Make'; Cmd='make'; Args='--version'},
    @{Name='7zip'; Cmd='7z'; Args='--help'}
)

foreach ($tool in $verifications) {
    $cmdExists = Test-CommandExists $tool.Cmd
    if ($cmdExists) {
        try {
            $version = & $tool.Cmd $tool.Args 2>&1 | Select-Object -First 1
            Write-Host "  [OK] $($tool.Name): $version" -ForegroundColor Green
        } catch {
            Write-Host "  [WARN]  $($tool.Name): Command exists but version check failed" -ForegroundColor Yellow
        }
    } else {
        Write-Host "  [FAIL] $($tool.Name): Not found in PATH" -ForegroundColor Red
    }
}

# 5. Check Python packages
Write-Host ""
Write-Host "[SNAKE] Checking Python packages..." -ForegroundColor Yellow
$pythonExists = Test-CommandExists 'python'
if ($pythonExists) {
    try {
        $pipVersion = python -m pip --version 2>&1
        Write-Host "  [OK] pip: $pipVersion" -ForegroundColor Green
        
        # Update pip itself
        Write-Host "  ^  Updating pip..." -ForegroundColor Cyan
        python -m pip install --upgrade pip 2>&1 | Out-Null
        
        # List outdated packages
        Write-Host "  [PUSHPIN] Checking for outdated Python packages..." -ForegroundColor Cyan
        $outdated = python -m pip list --outdated 2>&1
        if ($outdated -match "Package") {
            Write-Host $outdated
            Write-Host ""
            Write-Host "  To update all Python packages, run:" -ForegroundColor Yellow
            Write-Host "  python -m pip list --outdated --format=freeze | ForEach-Object {`$_.split('==')[0]} | ForEach-Object {python -m pip install --upgrade `$_}" -ForegroundColor Cyan
        } else {
            Write-Host "  [OK] All Python packages are up to date" -ForegroundColor Green
        }
    } catch {
        Write-Host "  [WARN]  pip not available" -ForegroundColor Yellow
    }
} else {
    Write-Host "  [FAIL] Python not installed" -ForegroundColor Red
}

# 6. Summary
Write-Host ""
Write-Host "+===============================================================+" -ForegroundColor Cyan
Write-Host "|                    Update Complete                            |" -ForegroundColor Cyan
Write-Host "+===============================================================+" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Close and reopen PowerShell to reload PATH" -ForegroundColor Gray
Write-Host "2. Run 'Check-ExplorerLensTools' to verify all tools" -ForegroundColor Gray
Write-Host "3. Run '. `$PROFILE' to reload the development environment" -ForegroundColor Gray
Write-Host ""

