#Requires -Version 7.0
<#
.SYNOPSIS
    File-based build monitoring utility for slow machines
.DESCRIPTION
    Monitors build output via log files instead of shell output
    Optimized for slow machines where shell buffering causes issues
.PARAMETER BuildScript
    The build script to execute (default: Build-Production.ps1)
.PARAMETER Arguments
    Arguments to pass to the build script
.PARAMETER MonitorInterval
    How often to check the log file (default: 5 seconds)
.PARAMETER ShowProgress
    Show progress updates in the console
.EXAMPLE
    .\Monitor-Build.ps1 -BuildScript "Build-Production.ps1" -Arguments "-Clean"
#>

param(
    [string]$BuildScript = "Build-Production.ps1",
    [string[]]$Arguments = @(),
    [int]$MonitorInterval = 5,
    [switch]$ShowProgress = $true
)

$ErrorActionPreference = "Continue"
$ProgressPreference = 'SilentlyContinue'

$scriptDir = Split-Path -Parent $PSScriptRoot
$projectRoot = Split-Path -Parent $scriptDir
Set-Location $projectRoot

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  File-Based Build Monitor" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Project Root: $projectRoot" -ForegroundColor Gray
Write-Host "Build Script: $BuildScript" -ForegroundColor Gray
Write-Host "Arguments: $($Arguments -join ' ')" -ForegroundColor Gray
Write-Host "Monitor Interval: $MonitorInterval seconds" -ForegroundColor Gray
Write-Host ""

# Ensure build-logs directory exists
$buildLogsDir = Join-Path $projectRoot "build-logs"
if (-not (Test-Path $buildLogsDir)) {
    New-Item -ItemType Directory -Path $buildLogsDir -Force | Out-Null
}

# Generate timestamped log file name
$timestamp = Get-Date -Format 'yyyy-MM-dd_HHmmss'
$logFile = Join-Path $buildLogsDir "monitored-build-$timestamp.log"
$stdoutFile = "$logFile.stdout"
$stderrFile = "$logFile.stderr"

Write-Host "Output will be logged to: $logFile" -ForegroundColor Yellow
Write-Host ""

# Start build process with output redirected to files
$scriptPath = Join-Path $projectRoot $BuildScript
$argumentList = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $scriptPath) + $Arguments

Write-Host "Starting build process..." -ForegroundColor Cyan
$buildProcess = Start-Process -FilePath "pwsh" `
    -ArgumentList $argumentList `
    -NoNewWindow `
    -PassThru `
    -RedirectStandardOutput $stdoutFile `
    -RedirectStandardError $stderrFile

Write-Host "Build process started (PID: $($buildProcess.Id))" -ForegroundColor Green
Write-Host ""

# Monitor the build process
$lastStdoutSize = 0
$lastStderrSize = 0
$lastUpdate = Get-Date
$noChangeCount = 0
$linesPrinted = 0

while (-not $buildProcess.HasExited) {
    Start-Sleep -Seconds $MonitorInterval
    
    $currentTime = Get-Date
    $elapsed = ($currentTime - $buildProcess.StartTime).TotalSeconds
    
    # Check stdout
    if (Test-Path $stdoutFile) {
        $stdoutSize = (Get-Item $stdoutFile).Length
        
        if ($stdoutSize -ne $lastStdoutSize) {
            # New output available
            if ($ShowProgress) {
                $newLines = Get-Content $stdoutFile | Select-Object -Skip $linesPrinted
                foreach ($line in $newLines) {
                    Write-Host $line
                }
                $linesPrinted = (Get-Content $stdoutFile).Count
            }
            
            $lastStdoutSize = $stdoutSize
            $lastUpdate = $currentTime
            $noChangeCount = 0
        } else {
            $noChangeCount++
        }
        
        # Progress indicator
        if ($ShowProgress) {
            $sizeMB = [Math]::Round($stdoutSize / 1MB, 2)
            $elapsedMin = [Math]::Round($elapsed / 60, 1)
            $timeSinceUpdate = [Math]::Round(($currentTime - $lastUpdate).TotalSeconds, 0)
            
            Write-Host "[Monitor] Elapsed: ${elapsedMin}m | Log Size: ${sizeMB}MB | Last Update: ${timeSinceUpdate}s ago" -ForegroundColor DarkGray
            
            # Warn if no changes for a long time
            if ($noChangeCount -gt 12) {
                # 1 minute with 5-second intervals
                Write-Host "[Warning] No log activity for $($noChangeCount * $MonitorInterval) seconds - build may be stuck" -ForegroundColor Yellow
            }
        }
    }
}

# Wait for process to fully exit
$buildProcess.WaitForExit()
$exitCode = $buildProcess.ExitCode
$totalTime = ($buildProcess.ExitTime - $buildProcess.StartTime).TotalSeconds

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Build Process Completed" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Exit Code: $exitCode" -ForegroundColor $(if ($exitCode -eq 0) { "Green" } else { "Red" })
Write-Host "Total Time: $([Math]::Round($totalTime / 60, 1)) minutes" -ForegroundColor Gray
Write-Host ""

# Combine stdout and stderr into final log
Write-Host "Combining log files..." -ForegroundColor Cyan
if (Test-Path $stdoutFile) {
    "=== STDOUT ===" | Out-File $logFile -Encoding UTF8
    Get-Content $stdoutFile | Add-Content $logFile -Encoding UTF8
}

if (Test-Path $stderrFile) {
    $stderrContent = Get-Content $stderrFile -ErrorAction SilentlyContinue
    if ($stderrContent) {
        "`n=== STDERR ===" | Add-Content $logFile -Encoding UTF8
        $stderrContent | Add-Content $logFile -Encoding UTF8
    }
}

Write-Host "Final log saved to: $logFile" -ForegroundColor Green

# Show last 30 lines
Write-Host ""
Write-Host "=== Last 30 lines of output ===" -ForegroundColor Yellow
Get-Content $logFile -Tail 30

# Cleanup temporary files
Remove-Item $stdoutFile -Force -ErrorAction SilentlyContinue
Remove-Item $stderrFile -Force -ErrorAction SilentlyContinue

Write-Host ""
if ($exitCode -eq 0) {
    Write-Host "✅ Build completed successfully!" -ForegroundColor Green
} else {
    Write-Host "❌ Build failed with exit code: $exitCode" -ForegroundColor Red
}

exit $exitCode
