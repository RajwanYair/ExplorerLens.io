# Verify All Development Tools
# Run with: pwsh -NoProfile -File verify-tools.ps1

Write-Host "+===============================================================+" -ForegroundColor Cyan
Write-Host "|     DarkThumbs Development Tools Verification                 |" -ForegroundColor Cyan
Write-Host "+===============================================================+" -ForegroundColor Cyan
Write-Host ""

$tools = @(
    @{Name='Git'; Cmd='git'; Args='--version'},
    @{Name='CMake'; Cmd='cmake'; Args='--version'},
    @{Name='Ninja'; Cmd='ninja'; Args='--version'},
    @{Name='Python'; Cmd='python'; Args='--version'},
    @{Name='Perl'; Cmd='perl'; Args='--version'},
    @{Name='GCC'; Cmd='gcc'; Args='--version'},
    @{Name='G++'; Cmd='g++'; Args='--version'},
    @{Name='Make'; Cmd='make'; Args='--version'},
    @{Name='MSVC (cl.exe)'; Cmd='cl'; Args=''},
    @{Name='MSBuild'; Cmd='msbuild'; Args='/version'},
    @{Name='7zip'; Cmd='7z'; Args=''},
    @{Name='Scoop'; Cmd='scoop'; Args='--version'}
)

$installed = @()
$missing = @()

foreach ($tool in $tools) {
    $cmd = Get-Command $tool.Cmd -ErrorAction SilentlyContinue
    if ($cmd) {
        $installed += $tool.Name
        try {
            if ($tool.Args) {
                $version = & $tool.Cmd $tool.Args 2>&1 | Select-Object -First 1
            } else {
                $version = & $tool.Cmd 2>&1 | Select-Object -First 1
            }
            Write-Host "[OK] $($tool.Name): $version" -ForegroundColor Green
        } catch {
            Write-Host "[OK] $($tool.Name): Installed (version check failed)" -ForegroundColor Green
        }
    } else {
        $missing += $tool.Name
        Write-Host "[FAIL] $($tool.Name): Not found" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "===============================================================" -ForegroundColor Cyan
Write-Host "Summary:" -ForegroundColor Yellow
Write-Host "  [OK] Installed: $($installed.Count) tools" -ForegroundColor Green
Write-Host "  [FAIL] Missing: $($missing.Count) tools" -ForegroundColor Red
Write-Host ""

if ($missing.Count -gt 0) {
    Write-Host "Missing tools can be installed with:" -ForegroundColor Yellow
    Write-Host "  scoop install perl python make nasm" -ForegroundColor Cyan
    Write-Host ""
}

Write-Host "To update all tools, run:" -ForegroundColor Yellow
Write-Host "  scoop update *" -ForegroundColor Cyan
Write-Host "  python -m pip install --upgrade pip" -ForegroundColor Cyan
Write-Host ""
