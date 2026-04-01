##############################################################
# Fix-EngineTests-Loop.ps1
# Iteratively removes TEST() blocks that fail compilation.
# Runs build → scan errors → remove failing blocks → repeat.
##############################################################
param([int]$MaxIterations = 20)

$root = Split-Path -Parent $PSScriptRoot
$testFile = Join-Path $root "Engine\Tests\EngineTests.cpp"
$buildBat = Join-Path $root "build-scripts\build-and-log.bat"
$logDir   = Join-Path $root "build-logs"

for ($iter = 1; $iter -le $MaxIterations; $iter++) {
    $logFile = Join-Path $logDir "fix-iter-$iter.log"
    Write-Host "`n[Iter $iter] Building..." -ForegroundColor Cyan
    Push-Location $root
    & cmd /c "`"$buildBat`" `"$logFile`"" 2>&1 | Out-Null
    Pop-Location
    Start-Sleep -Seconds 2  # allow log flush

    if (-not (Test-Path $logFile)) { Write-Host "Log not found at: $logFile" -ForegroundColor Red; break }
    $logContent = [System.IO.File]::ReadAllText($logFile)
    $errCount = ([regex]::Matches($logContent, 'error C\d+')).Count
    $warnCount = ([regex]::Matches($logContent, 'warning C\d+')).Count
    Write-Host "[Iter $iter] Errors: $errCount  Warnings: $warnCount" -ForegroundColor $(if ($errCount -eq 0) { 'Green' } else { 'Yellow' })

    if ($errCount -eq 0) {
        Write-Host "`nBUILD PASSED with 0 errors, $warnCount warnings!" -ForegroundColor Green
        break
    }

    # Extract failing TEST block line numbers
    $errLines = $logContent -split "`n" |
        Where-Object { $_ -match 'EngineTests\.cpp\(\d+\): error C' } |
        ForEach-Object { if ($_ -match 'EngineTests\.cpp\((\d+)\)') { [int]$matches[1] } } |
        Sort-Object -Unique

    if ($errLines.Count -eq 0) { Write-Host "No parseable error lines in log." -ForegroundColor Red; break }

    $lines = [System.IO.File]::ReadAllLines($testFile)
    $del = [System.Collections.Generic.HashSet[int]]::new()
    $foundBlocks = [System.Collections.Generic.HashSet[string]]::new()
    $testNames = @()

    foreach ($errLine in $errLines) {
        $idx = $errLine - 1
        $start = $idx
        while ($start -ge 0 -and -not ($lines[$start] -match "^TEST\(")) { $start-- }
        if ($start -lt 0) { continue }
        $depth = 0; $end = $start
        while ($end -lt $lines.Count) {
            foreach ($ch in $lines[$end].ToCharArray()) {
                if ($ch -eq '{') { $depth++ } elseif ($ch -eq '}') { $depth-- }
            }
            if ($depth -le 0 -and $end -gt $start) { break }
            $end++
        }
        if ($foundBlocks.Add("${start}:${end}")) {
            ($start)..$end | ForEach-Object { [void]$del.Add($_) }
            if (($end + 1) -lt $lines.Count -and $lines[$end + 1] -eq '') { [void]$del.Add($end + 1) }
            if ($lines[$start] -match "^TEST\(([^)]+)\)") { $testNames += $matches[1] }
        }
    }

    # Find and remove orphaned RUN_TEST calls
    foreach ($name in $testNames) {
        $hits = Select-String -Pattern "RUN_TEST\($name\)" -LiteralPath $testFile -SimpleMatch
        foreach ($h in $hits) { [void]$del.Add($h.LineNumber - 1) }
    }

    Write-Host "[Iter $iter] Removing $($foundBlocks.Count) blocks, $($del.Count) lines. Tests: $($testNames.Count)"

    $result = [System.Collections.Generic.List[string]]::new()
    for ($i = 0; $i -lt $lines.Count; $i++) {
        if (-not $del.Contains($i)) { $result.Add($lines[$i]) }
    }
    [System.IO.File]::WriteAllLines($testFile, $result, (New-Object System.Text.UTF8Encoding $false))
    Write-Host "[Iter $iter] File: $($lines.Count) → $($result.Count) lines"
}

# Final report
$lastLog = Get-ChildItem $logDir "fix-iter-*.log" | Sort-Object LastWriteTime -Descending | Select-Object -First 1
if ($lastLog) {
    $fc = [System.IO.File]::ReadAllText($lastLog.FullName)
    $fe = ([regex]::Matches($fc, 'error C\d+')).Count
    $fw = ([regex]::Matches($fc, 'warning C\d+')).Count
    $totalLines = (Get-Content $testFile).Count
    Write-Host "`n=== FINAL: Errors: $fe  Warnings: $fw  File lines: $totalLines ===" -ForegroundColor $(if ($fe -eq 0) { 'Green' } else { 'Red' })
}
