# DarkThumbs MSVC Environment Setup for PowerShell
# This script configures Visual Studio Build Tools environment

$VSPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
$VCVarsAll = Join-Path $VSPath "VC\Auxiliary\Build\vcvarsall.bat"

if (Test-Path $VCVarsAll) {
    Write-Host "Setting up Visual Studio 2026 Build Tools (x64)..." -ForegroundColor Cyan
    
    # Import Visual Studio environment variables
    # We use a helper to capture vcvarsall output and set environment variables
    $tempFile = [System.IO.Path]::GetTempFileName()
    
    cmd /c "`"$VCVarsAll`" x64 > nul && set" > $tempFile
    
    Get-Content $tempFile | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') {
            $varName = $matches[1]
            $varValue = $matches[2]
            Set-Item -Path "env:$varName" -Value $varValue -Force
        }
    }
    
    Remove-Item $tempFile
    
    Write-Host "[OK] MSVC environment configured (x64)" -ForegroundColor Green
    Write-Host "  Available tools: cl.exe, msbuild, link.exe, lib.exe" -ForegroundColor Gray
    
    # Verify cl.exe
    if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
        $clVersion = (cmd /c "cl.exe 2>&1" | Select-Object -First 1).Trim()
        Write-Host "  Compiler: $clVersion" -ForegroundColor Gray
    }
} else {
    Write-Warning "Visual Studio Build Tools not found at: $VSPath"
    Write-Warning "Please install Visual Studio Build Tools 2022/2026"
}
