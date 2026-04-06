# Fix-PCH-Corruption.ps1 - Repair corrupted #include "StdAfx.h\n" lines
param([string]$ProjectDir = "LENSShell")

$fixed = 0
$files = Get-ChildItem "$ProjectDir\*.cpp" | Where-Object { $_.Name -notmatch "^stdafx\.cpp$" -and $_.Name -notmatch "^StdAfx\.cpp$" }

foreach ($f in $files) {
    $text = Get-Content $f.FullName -Raw -Encoding UTF8
    if ($text -match '#include "StdAfx\.h[\r\n]') {
        $fixed_text = [System.Text.RegularExpressions.Regex]::Replace($text, '#include "StdAfx\.h[\r\n]+"', '#include "StdAfx.h"')
        $fixed_text | Set-Content $f.FullName -Encoding UTF8 -NoNewline
        Write-Host "Fixed: $($f.Name)"
        $fixed++
    }
}
Write-Host "Done. Fixed $fixed file(s)."
