#Requires -Version 7.0
<#
.SYNOPSIS
    Monitor build logs in real-time with color-coded output

.DESCRIPTION
    Monitors the build-logs directory for new or updated log files and
    displays the last 50 lines with color coding for status messages.

.PARAMETER LogPattern
    Glob pattern for log files to monitor (default: build-logs\*.log)

.PARAMETER RefreshSeconds
    How often to check for updates (default: 5 seconds)

.PARAMETER TailLines
    Number of lines to display from end of file (default: 50)

.EXAMPLE
    .\Monitor-Build-Logs.ps1

.EXAMPLE
    .\Monitor-Build-Logs.ps1 -LogPattern "build-logs\lzma-*.log" -RefreshSeconds 10

.EXAMPLE
    .\Monitor-Build-Logs.ps1 -TailLines 100
#>

param(
    [string]$LogPattern = "",
    [int]$RefreshSeconds = 5,
    [int]$TailLines = 50
)

# Default log pattern: prefer TEMP logs, fall back to project build-logs
if (-not $LogPattern) {
    $LogPattern = Join-Path $env:TEMP "ExplorerLens-logs\*.log"
}

$ErrorActionPreference = "SilentlyContinue"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Build Log Monitor" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Monitoring: $LogPattern" -ForegroundColor Gray
Write-Host "Refresh: Every $RefreshSeconds seconds" -ForegroundColor Gray
Write-Host "Display: Last $TailLines lines" -ForegroundColor Gray
Write-Host ""
Write-Host "Press Ctrl+C to stop" -ForegroundColor Yellow
Write-Host ""

$lastCheck = Get-Date
$lastLogFile = $null

while ($true) {
    # Find most recent log file
    $latestLog = Get-ChildItem $LogPattern -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

    if ($latestLog) {
        # Check if file changed or we're monitoring a new file
        $fileChanged = ($latestLog.LastWriteTime -gt $lastCheck) -or
        ($lastLogFile -ne $latestLog.FullName)

        if ($fileChanged) {
            Clear-Host

            Write-Host "========================================" -ForegroundColor Cyan
            Write-Host "  $($latestLog.Name)" -ForegroundColor Cyan
            Write-Host "========================================" -ForegroundColor Cyan
            Write-Host "Last Updated: $($latestLog.LastWriteTime.ToString('yyyy-MM-dd HH:mm:ss'))" -ForegroundColor Gray
            Write-Host "Size: $([math]::Round($latestLog.Length / 1KB, 1)) KB" -ForegroundColor Gray
            Write-Host ""

            # Read and display with color coding
            $content = Get-Content $latestLog.FullName -Tail $TailLines -ErrorAction Continue

            foreach ($line in $content) {
                $color = 'White'

                # Color code based on content
                if ($line -match '✅|SUCCESS|success|succeeded|Complete|completed|Built') {
                    $color = 'Green'
                } elseif ($line -match '❌|ERROR|error|FAILED|failed|Fatal|FATAL') {
                    $color = 'Red'
                } elseif ($line -match '⚠️|WARNING|warning|WARN') {
                    $color = 'Yellow'
                } elseif ($line -match '\[.*?\]|Building|Compiling') {
                    $color = 'Cyan'
                } elseif ($line -match '^\s*$') {
                    # Skip empty lines
                    continue
                }

                Write-Host $line -ForegroundColor $color
            }

            Write-Host ""
            Write-Host "========================================" -ForegroundColor Cyan
            Write-Host "Monitoring... (Ctrl+C to stop)" -ForegroundColor Gray

            $lastCheck = $latestLog.LastWriteTime
            $lastLogFile = $latestLog.FullName
        }
    } else {
        Write-Host "No log files found matching: $LogPattern" -ForegroundColor Yellow
        Write-Host "Waiting for logs..." -ForegroundColor Gray
    }

    Start-Sleep -Seconds $RefreshSeconds
}
