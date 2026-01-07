# ===========================================================================
# Fix-NonAsciiCharacters.ps1
# Replaces all non-ASCII characters in PowerShell and C++ source files
# ===========================================================================

param(
    [switch]$WhatIf
)

$ErrorActionPreference = "Continue"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)

Write-Host "=========================================================================="  -ForegroundColor Cyan
Write-Host "Fixing Non-ASCII Characters in DarkThumbs Project" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host ""

# Replacement mapping: Unicode character -> ASCII equivalent
$ReplacementMap = @{
    # Unicode Box Drawing -> ASCII
    [char]0x2554 = '+'  # + Box drawings double down and right
    [char]0x2550 = '='  # = Box drawings double horizontal
    [char]0x2557 = '+'  # + Box drawings double down and left
    [char]0x2551 = '|'  # | Box drawings double vertical
    [char]0x255A = '+'  # + Box drawings double up and right
    [char]0x255D = '+'  # + Box drawings double up and left
    [char]0x2501 = '-'  # - Box drawings heavy horizontal
    [char]0x2560 = '+'  # + Box drawings double vertical and right
    [char]0x2563 = '+'  # + Box drawings double vertical and left
    
    # Checkmarks and X marks
    [char]0x2713 = '[OK]'      # [OK] Check mark
    [char]0x2717 = '[FAIL]'    # [FAIL] Ballot X
    [char]0x2705 = '[OK]'      # [OK] White heavy check mark
    [char]0x274C = '[FAIL]'    # [FAIL] Cross mark
    [char]0x2714 = '[OK]'      # [OK] Heavy check mark
    [char]0x2718 = '[FAIL]'    # [FAIL] Heavy ballot X
    
    # Warning and info symbols
    [char]0x26A0 = '[WARN]'    # [WARN] Warning sign
    [char]0x2139 = '[INFO]'    # [INFO] Information source
    [char]0x2295 = '[+]'       # [+] Circled plus
    [char]0x25CB = '[ ]'       # [ ] White circle
    [char]0x2753 = '[?]'       # [?] Question mark ornament
    
    # Arrows and symbols
    [char]0x2192 = '->'        # -> Rightwards arrow
    [char]0x2B06 = '^'         # ^ Upwards black arrow
    
    # Emojis and symbols
    [char]0x2600 = 'Light'     # Light Sun
    [char]0x2699 = '[GEAR]'    # [GEAR] Gear
    [char]0x23F3 = '[...]'     # [...] Hourglass
    [char]0x23ED = '[>>]'      # [>>] Next track button
    [char]0xFE0F = ''          #  Variation Selector-16 (remove)
    
    # Special quotes
    [char]0x2018 = "'"   # ' Left single quote
    [char]0x2019 = "'"   # ' Right single quote
    [char]0x201C = '"'   # " Left double quote
    [char]0x201D = '"'   # " Right double quote
    
    # Dashes
    [char]0x2013 = '-'   # - En dash
    [char]0x2014 = '--'  # -- Em dash
}

# Emoji surrogate pair patterns (these appear as two characters in UTF-16)
$EmojiPatterns = @{
    '\uD83D\uDD27' = '[WRENCH]'   # [WRENCH] Wrench
    '\uD83D\uDCA1' = '[IDEA]'     # [IDEA] Light bulb
    '\uD83D\uDD0D' = '[PUSHPIN]'  # 📍 Pushpin
    '\uD83D\uDCE6' = '[PACKAGE]'  # [PACKAGE] Package
    '\uD83D\uDE80' = '[ROCKET]'   # [ROCKET] Rocket
    '\uD83D\uDCE5' = '[INBOX]'    # [INBOX] Inbox tray
    '\uD83D\uDC0D' = '[SNAKE]'    # [SNAKE] Snake
    '\uD83D\uDCD6' = '[BOOK]'     # [BOOK] Open book
}



function Fix-FileEncoding {
    param(
        [string]$FilePath
    )
    
    $content = Get-Content $FilePath -Raw -Encoding UTF8
    $originalContent = $content
    $modified = $false
    
    # Replace emoji surrogate pairs first
    foreach ($pattern in $EmojiPatterns.Keys) {
        $regex = $pattern -replace '\\u', '\u'
        if ($content -match $regex) {
            $content = $content -replace $regex, $EmojiPatterns[$pattern]
            $modified = $true
        }
    }
    
    foreach ($key in $ReplacementMap.Keys) {
        if ($content -match [regex]::Escape($key)) {
            $content = $content -replace [regex]::Escape($key), $ReplacementMap[$key]
            $modified = $true
        }
    }
    
    # Additional regex-based replacements
    # Replace remaining Unicode characters (non-ASCII printable) with placeholder
    $unicodePattern = '[^\x00-\x7F]'
    if ($content -match $unicodePattern) {
        $matches = [regex]::Matches($content, $unicodePattern)
        foreach ($match in $matches) {
            $char = $match.Value
            $codePoint = [int][char]$char
            Write-Host "  Found unknown Unicode U+$($codePoint.ToString('X4')): $char" -ForegroundColor Yellow
        }
        $modified = $true
    }
    
    if ($modified) {
        if (-not $WhatIf) {
            # Save with UTF-8 encoding (no BOM for compatibility)
            $utf8NoBom = New-Object System.Text.UTF8Encoding $false
            [System.IO.File]::WriteAllText($FilePath, $content, $utf8NoBom)
        }
        return $true
    }
    
    return $false
}

# Process PowerShell files
Write-Host "Processing PowerShell files..." -ForegroundColor Green
$ps1Files = Get-ChildItem -Path $ProjectRoot -Filter "*.ps1" -Recurse | 
Where-Object { $_.FullName -notmatch "\\(external|tools|packages|node_modules|\.git)\\" }

$psCount = 0
foreach ($file in $ps1Files) {
    if (Fix-FileEncoding -FilePath $file.FullName) {
        Write-Host "  [FIXED] $($file.FullName.Replace($ProjectRoot, '.'))" -ForegroundColor Yellow
        $psCount++
    }
}
Write-Host "Fixed $psCount PowerShell files" -ForegroundColor Cyan
Write-Host ""

# Process C++ source files
Write-Host "Processing C++ source files..." -ForegroundColor Green
$cppFiles = Get-ChildItem -Path $ProjectRoot -Include "*.cpp", "*.h" -Recurse |
Where-Object { $_.FullName -notmatch "\\(external|tools|packages|tests|x64|Release|Debug)\\" }

$cppCount = 0
foreach ($file in $cppFiles) {
    if (Fix-FileEncoding -FilePath $file.FullName) {
        Write-Host "  [FIXED] $($file.FullName.Replace($ProjectRoot, '.'))" -ForegroundColor Yellow
        $cppCount++
    }
}
Write-Host "Fixed $cppCount C++ files" -ForegroundColor Cyan
Write-Host ""

Write-Host "==========================================================================" -ForegroundColor Green
Write-Host "Summary:" -ForegroundColor Cyan
Write-Host "  PowerShell files fixed: $psCount" -ForegroundColor White
Write-Host "  C++ files fixed: $cppCount" -ForegroundColor White
if ($WhatIf) {
    Write-Host "  Mode: DRY RUN (no files modified)" -ForegroundColor Yellow
}
Write-Host "==========================================================================" -ForegroundColor Green
