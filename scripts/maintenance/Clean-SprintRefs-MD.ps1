$ErrorActionPreference = 'SilentlyContinue'
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$cleaned = 0

Get-ChildItem $root -Recurse -Filter *.md | Where-Object {
    $_.FullName -notmatch 'external|build-vcpkg|gtest|packages|node_modules|\.github'
} | ForEach-Object {
    $content = [System.IO.File]::ReadAllText($_.FullName)
    $original = $content
    $content = $content -replace '\(Sprint \d+[\.\d]*[A-Za-z]?\)', ''
    $content = $content -replace 'Sprint \d+[\-\x{2013}]\d+:\s*', ''
    $content = $content -replace 'Sprint \d+[\.\d]*[A-Za-z]?:\s*', ''
    $content = $content -replace '\bSprint \d+[\.\d]*[A-Za-z]?\b', ''
    $content = $content -replace '  +', ' '
    if ($content -ne $original) {
        [System.IO.File]::WriteAllText($_.FullName, $content)
        $cleaned++
        Write-Host "  Cleaned: $($_.Name)" -ForegroundColor Green
    }
}
Write-Host "`nCleaned $cleaned MD files" -ForegroundColor Cyan
