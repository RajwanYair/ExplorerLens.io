Write-Host "=== Quick Library Build Test ===" -ForegroundColor Cyan

# Test LZMA
Write-Host "`n[1/2] Testing LZMA/XZ build..." -ForegroundColor Yellow
try {
    & ".\build-scripts\build-lzma-24.08.ps1" *>&1 | Out-Null
    if (Test-Path "external\compression\xz-5.6.3\build-vs\Release\liblzma.lib") {
        Write-Host "  ✅ LZMA: SUCCESS" -ForegroundColor Green
    } else {
        Write-Host "  ❌ LZMA: FAILED - lib not found" -ForegroundColor Red
    }
} catch {
    Write-Host "  ❌ LZMA: ERROR - $($_.Exception.Message)" -ForegroundColor Red
}

# Test LibWebP
Write-Host "`n[2/2] Testing LibWebP build..." -ForegroundColor Yellow
try {
    & ".\build-scripts\Build-LibWebP-NMake.ps1" *>&1 | Out-Null
    if (Test-Path "external\image-libs\libwebp-1.5.0\build-vs\Release\webp.lib") {
        Write-Host "  ✅ LibWebP: SUCCESS" -ForegroundColor Green
    } else {
        Write-Host "  ❌ LibWebP: FAILED - lib not found" -ForegroundColor Red
    }
} catch {
    Write-Host "  ❌ LibWebP: ERROR - $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host "`n=== Test Complete ===" -ForegroundColor Cyan
