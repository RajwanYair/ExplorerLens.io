#Requires -Version 7.0
<#
.SYNOPSIS
    Build wrapper with file-based monitoring for slow machines

.DESCRIPTION
    Wraps any build script to redirect output to a timestamped log file
    in the build-logs directory. Allows monitoring progress from VS Code
    instead of waiting on PowerShell terminal.

.PARAMETER ScriptPath
    Path to the build script to execute

.PARAMETER Watch
    If specified, monitors the log file and displays last 5 lines periodically

.EXAMPLE
    .\Build-With-Monitoring.ps1 -ScriptPath ".\build-scripts\build-lzma-simple.ps1" -Watch

.EXAMPLE
    .\Build-With-Monitoring.ps1 -ScriptPath ".\build-scripts\Build-LibWebP-NMake.ps1"
#>

param(
    [Parameter(Mandatory = $true)]
    [string]$ScriptPath,
    
    [switch]$Watch
)

$ErrorActionPreference = "Continue"
$timestamp = Get-Date -Format "yyyy-MM-dd_HHmmss"
$scriptName = [System.IO.Path]::GetFileNameWithoutExtension($ScriptPath)
$logFile = "build-logs\$scriptName-$timestamp.log"

# Ensure log directory exists
New-Item -ItemType Directory -Path "build-logs" -Force -ErrorAction SilentlyContinue | Out-Null

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Building: $scriptName" -ForegroundColor Cyan
Write-Host "  Log: $logFile" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Build output will be written to: $logFile" -ForegroundColor Yellow
Write-Host "Open this file in VS Code to monitor progress." -ForegroundColor Yellow
Write-Host ""

# Resolve script path
if (-not [System.IO.Path]::IsPathRooted($ScriptPath)) {
    $ScriptPath = Join-Path $PSScriptRoot ".." $ScriptPath
}

if (-not (Test-Path $ScriptPath)) {
    Write-Host "ERROR: Script not found: $ScriptPath" -ForegroundColor Red
    exit 1
}

# Start build in background
$job = Start-Job -ScriptBlock {
    param($script, $log, $workDir)
    Set-Location $workDir
    & $script *>&1 | Tee-Object -FilePath $log
} -ArgumentList $ScriptPath, $logFile, $PWD

Write-Host "Build started in background (Job ID: $($job.Id))" -ForegroundColor Green
Write-Host ""

if ($Watch) {
    Write-Host "Monitoring log file (Ctrl+C to stop watching)..." -ForegroundColor Cyan
    Write-Host "Tip: Open $logFile in VS Code for better viewing" -ForegroundColor Gray
    Write-Host ""
    
    # Monitor with timeout
    $timeout = 3600 # 1 hour
    $elapsed = 0
    
    while ($job.State -eq 'Running' -and $elapsed -lt $timeout) {
        Start-Sleep -Seconds 10
        $elapsed += 10
        
        if (Test-Path $logFile) {
            $lastLines = Get-Content $logFile -Tail 5 -ErrorAction SilentlyContinue
            if ($lastLines) {
                Clear-Host
                Write-Host "=== Last 5 lines of $scriptName ===" -ForegroundColor Cyan
                $lastLines | ForEach-Object { Write-Host $_ -ForegroundColor Gray }
                Write-Host ""
                Write-Host "Elapsed: $elapsed seconds | Job State: $($job.State)" -ForegroundColor DarkGray
            }
        }
    }
    
    if ($elapsed -ge $timeout) {
        Write-Host "WARNING: Build exceeded timeout of $timeout seconds" -ForegroundColor Yellow
        Write-Host "Check log file: $logFile" -ForegroundColor Yellow
    }
}

# Wait for completion
Wait-Job $job -Timeout 7200 | Out-Null # 2 hour max

# Get results
$result = Receive-Job $job
$exitCode = if ($job.State -eq 'Completed') { 0 } else { 1 }

Remove-Job $job -Force

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Build Complete" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Exit Code: $exitCode" -ForegroundColor $(if ($exitCode -eq 0) { 'Green' }else { 'Red' })
Write-Host "Full log: $logFile" -ForegroundColor Cyan
Write-Host ""

exit $exitCode
