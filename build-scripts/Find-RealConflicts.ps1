# Find-RealConflicts.ps1 — Find ACTUAL same-namespace duplicate type definitions
# Parses each header to track full namespace scope of each type definition
param(
    [string]$EngineDir = "$PSScriptRoot\..\Engine",
    [string]$OutFile   = "$PSScriptRoot\..\build-logs\real-conflicts.txt"
)

$root = (Resolve-Path $EngineDir).Path

function Get-TypesWithNamespace($filePath) {
    $content = Get-Content $filePath -Raw -ErrorAction SilentlyContinue
    if (-not $content) { return @() }

    $results   = [System.Collections.Generic.List[psobject]]::new()
    $nsStack   = [System.Collections.Generic.Stack[string]]::new()
    $depth     = 0  # brace depth
    $nsDepths  = [System.Collections.Generic.Stack[int]]::new()  # brace depth where each ns was opened

    $lines = $content -split "`n"
    $lineNum = 0
    foreach ($line in $lines) {
        $lineNum++
        $trimmed = $line.TrimStart()

        # Detect namespace declarations (handles both C++17 nested and classic)
        if ($trimmed -match '^namespace\s+([\w:]+)\s*\{') {
            $nsName = $Matches[1]
            # Handle nested namespace A::B::C
            foreach ($part in ($nsName -split '::')) {
                if ($part -match '^\w+$') {
                    $nsStack.Push($part)
                    $nsDepths.Push($depth)
                }
            }
            # Don't count this brace (we count below)
        }

        # Count braces
        $opens  = ($line.ToCharArray() | Where-Object { $_ -eq '{' }).Count
        $closes = ($line.ToCharArray() | Where-Object { $_ -eq '}' }).Count
        $depth += $opens - $closes

        # Pop namespace stack when we close back to its depth
        while ($nsDepths.Count -gt 0 -and $depth -le $nsDepths.Peek()) {
            $nsDepths.Pop() | Out-Null
            $nsStack.Pop() | Out-Null
        }

        # Detect type definitions at this scope
        if ($trimmed -match '^(?:enum class|struct|class)\s+(\w+)') {
            $typeName = $Matches[1]
            if ($typeName -notmatch '^[A-Z]$') {
                $arr = $nsStack.ToArray()
                [System.Array]::Reverse($arr)
                $fullNs = $arr -join '::'
                $results.Add([pscustomobject]@{
                    Type      = $typeName
                    Namespace = $fullNs
                    File      = $filePath.Substring($root.Length + 1)
                    Line      = $lineNum
                })
            }
        }
    }
    return $results
}

Write-Host "Scanning headers for real namespace conflicts..."
$allTypes = [System.Collections.Generic.Dictionary[string, System.Collections.Generic.List[psobject]]]::new()

$headers = Get-ChildItem $root -Recurse -Include "*.h" | Where-Object { $_.FullName -notmatch "EngineTests|gtest" }
Write-Host "Processing $($headers.Count) headers..."

foreach ($hdr in $headers) {
    $types = Get-TypesWithNamespace $hdr.FullName
    foreach ($t in $types) {
        $key = "$($t.Namespace)::$($t.Type)"
        if (-not $allTypes.ContainsKey($key)) {
            $allTypes[$key] = [System.Collections.Generic.List[psobject]]::new()
        }
        $allTypes[$key].Add($t)
    }
}

$conflicts = $allTypes.GetEnumerator() |
    Where-Object { $_.Value.Count -gt 1 } |
    Sort-Object Name

Write-Host "`nFound $($conflicts.Count) REAL same-namespace conflicts:"
$lines = [System.Collections.Generic.List[string]]::new()
foreach ($c in $conflicts) {
    $info = "[$($c.Name)] defined $($c.Value.Count)x:"
    $detail = $c.Value | ForEach-Object { "  $($_.File):$($_.Line)" }
    $block = ($info, ($detail -join "`n")) -join "`n"
    Write-Host $block
    $lines.Add($block)
}

New-Item -ItemType Directory -Path (Split-Path $OutFile) -Force | Out-Null
$lines | Out-File -FilePath $OutFile -Encoding UTF8
Write-Host "`nResults written to $OutFile"
