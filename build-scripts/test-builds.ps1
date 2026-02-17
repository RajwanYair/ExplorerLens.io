Write-Host "=== Quick Library Build Test ===" -ForegroundColor Cyan

# Test LZMA SDK
Write-Host "`n[1/2] Testing LZMA SDK 26.00 build..." -ForegroundColor Yellow
try {
    & ".\build-scripts\external-libs\Build-LZMA-SDK-26.00.ps1" *>&1 | Out-Null
    if (Test-Path "external\compression-libs\lzma-26.00\build-vs\Release\lzma.lib") {
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
    & ".\build-scripts\external-libs\Build-LibWebP-NMake.ps1" *>&1 | Out-Null
    if (Test-Path "external\image-libs\libwebp-1.5.0-build\build-vs\Release\webp.lib") {
        Write-Host "  ✅ LibWebP: SUCCESS" -ForegroundColor Green
    } else {
        Write-Host "  ❌ LibWebP: FAILED - lib not found" -ForegroundColor Red
    }
} catch {
    Write-Host "  ❌ LibWebP: ERROR - $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host "`n=== Test Complete ===" -ForegroundColor Cyan
