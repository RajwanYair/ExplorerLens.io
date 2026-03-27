# Find-DuplicateTypes.ps1 — Scan all Engine headers for duplicate top-level type definitions
# Usage: .\build-scripts\Find-DuplicateTypes.ps1
param(
    [string]$EngineDir = "$PSScriptRoot\..\Engine",
    [string]$OutFile = "$PSScriptRoot\..\build-logs\duplicate-types.txt"
)

$root = (Resolve-Path $EngineDir).Path
$allDefs = @{}

$headers = Get-ChildItem $root -Recurse -Include "*.h" | Where-Object { $_.FullName -notmatch "EngineTests|gtest" }
Write-Host "Scanning $($headers.Count) headers..."

foreach ($hdr in $headers) {
    $rel = $hdr.FullName.Substring($root.Length + 1)
    $content = Get-Content $hdr.FullName -Raw -ErrorAction SilentlyContinue
    if (-not $content) { continue }

    # Match top-level type defs: lines starting with enum class / struct / class (no leading whitespace)
    $matches = [regex]::Matches($content, '(?m)^(?:enum class|struct|class)\s+(\w+)')
    foreach ($m in $matches) {
        $typename = $m.Groups[1].Value
        # Skip single-char template params and common non-type words
        if ($typename -match '^[A-Z]$') { continue }
        if ($typename -in @('Override','Final','Impl','Base','Tag','Key','Value','Hash','Eq')) { continue }
        if (-not $allDefs.ContainsKey($typename)) {
            $allDefs[$typename] = [System.Collections.Generic.List[string]]::new()
        }
        $allDefs[$typename].Add($rel)
    }
}

$dupes = $allDefs.GetEnumerator() |
    Where-Object { $_.Value.Count -gt 1 } |
    Sort-Object Name

Write-Host "`nFound $($dupes.Count) duplicate type names:"
$lines = [System.Collections.Generic.List[string]]::new()
foreach ($d in $dupes) {
    $line = "$($d.Name) [$($d.Value.Count)]:`n" + ($d.Value | ForEach-Object { "  $_" } | Out-String).TrimEnd()
    Write-Host $line
    $lines.Add($line)
}

New-Item -ItemType Directory -Path (Split-Path $OutFile) -Force | Out-Null
$lines | Out-File -FilePath $OutFile -Encoding UTF8
Write-Host "`nResults written to: $OutFile"
