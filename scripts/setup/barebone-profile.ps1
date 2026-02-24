# ============================================================================
# ExplorerLens - Barebone PowerShell Profile (Optimized for Slow Machines)
# Version: 4.0 - 2026-01-06
# ============================================================================
# This is a minimal profile designed for maximum performance
# No fancy prompts, no modules, no unnecessary features

# Suppress verbose output
$ErrorActionPreference = 'SilentlyContinue'
$ProgressPreference = 'SilentlyContinue'

# Intel Proxy (if needed)
# [System.Net.Http.HttpClient]::DefaultProxy = New-Object System.Net.WebProxy('http://proxy-dmz.intel.com:912', $true)

# Simple prompt - just path
function prompt {
    "PS $(Get-Location)> "
}

# Essential ExplorerLens variables
$global:ExplorerLensPath = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)

# Quick navigation aliases
function dt { Set-Location $global:ExplorerLensPath }
function bs { Set-Location "$global:ExplorerLensPath\build-scripts" }
function src { Set-Location "$global:ExplorerLensPath\LENSShell" }

# Essential build functions (no MSVC environment loading)
function Build-Quick {
    param([switch]$Clean)
    Push-Location $global:ExplorerLensPath
    if ($Clean) {
        & .\Build-Production.ps1 -Clean
    } else {
        & .\Build-Production.ps1 -SkipLibraries
    }
    Pop-Location
}

function Build-Monitor {
    param([string]$LogFile = "build-logs\production-build-*.log")
    $logs = Get-ChildItem "$global:ExplorerLensPath\$LogFile" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending
    if ($logs) {
        $latest = $logs[0]
        Write-Host "Monitoring: $($latest.Name)" -ForegroundColor Cyan
        Get-Content $latest.FullName -Wait -Tail 20
    } else {
        Write-Host "No build logs found matching: $LogFile" -ForegroundColor Yellow
    }
}

function Build-Status {
    $logDir = "$global:ExplorerLensPath\build-logs"
    if (Test-Path $logDir) {
        Write-Host "Recent builds:" -ForegroundColor Cyan
        Get-ChildItem $logDir -Filter "*.log" | 
        Sort-Object LastWriteTime -Descending | 
        Select-Object -First 5 |
        ForEach-Object {
            $status = "?"
            $content = Get-Content $_.FullName -Tail 5
            if ($content -match "BUILD SUCCESSFUL") { $status = "✅" }
            elseif ($content -match "BUILD FAILED") { $status = "❌" }
            Write-Host "  $status $($_.Name) - $($_.LastWriteTime.ToString('yyyy-MM-dd HH:mm:ss'))" -ForegroundColor Gray
        }
    } else {
        Write-Host "No build logs directory found" -ForegroundColor Yellow
    }
}

# File-based build monitor
function Start-BuildMonitor {
    param(
        [string]$LogPattern = "production-build-*.log",
        [int]$RefreshSeconds = 10
    )
    
    $logDir = "$global:ExplorerLensPath\build-logs"
    Write-Host "Starting file-based build monitor..." -ForegroundColor Cyan
    Write-Host "Log directory: $logDir" -ForegroundColor Gray
    Write-Host "Refresh interval: $RefreshSeconds seconds" -ForegroundColor Gray
    Write-Host "Press Ctrl+C to stop" -ForegroundColor Yellow
    Write-Host ""
    
    $lastSize = 0
    $lastUpdateTime = Get-Date
    
    while ($true) {
        $logs = Get-ChildItem "$logDir\$LogPattern" -ErrorAction SilentlyContinue | 
        Sort-Object LastWriteTime -Descending | 
        Select-Object -First 1
        
        if ($logs) {
            $log = $logs[0]
            $currentSize = $log.Length
            $elapsed = (Get-Date) - $log.LastWriteTime
            
            if ($currentSize -ne $lastSize) {
                Clear-Host
                Write-Host "=== Build Monitor ===" -ForegroundColor Cyan
                Write-Host "Log: $($log.Name)" -ForegroundColor White
                Write-Host "Size: $([Math]::Round($currentSize/1KB, 1)) KB" -ForegroundColor Gray
                Write-Host "Last update: $($elapsed.TotalSeconds.ToString('F0')) seconds ago" -ForegroundColor Gray
                Write-Host ""
                Write-Host "=== Last 20 lines ===" -ForegroundColor Yellow
                Get-Content $log.FullName -Tail 20
                Write-Host ""
                $lastSize = $currentSize
                $lastUpdateTime = Get-Date
            } else {
                $noChangeFor = (Get-Date) - $lastUpdateTime
                Write-Host "[$(Get-Date -Format 'HH:mm:ss')] No changes for $($noChangeFor.TotalSeconds.ToString('F0'))s..." -ForegroundColor DarkGray
            }
        } else {
            Write-Host "[$(Get-Date -Format 'HH:mm:ss')] Waiting for build log..." -ForegroundColor DarkGray
        }
        
        Start-Sleep -Seconds $RefreshSeconds
    }
}

# Display help on startup (optional - comment out for even faster startup)
# Write-Host "ExplorerLens Barebone Profile Loaded" -ForegroundColor Green
# Write-Host "Commands: dt, bs, src, Build-Quick, Build-Monitor, Build-Status, Start-BuildMonitor" -ForegroundColor Gray

