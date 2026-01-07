# View-DarkThumbsDiagnostics.ps1
# View diagnostic logs and performance reports

param(
    [switch]$Logs = $false,
    [switch]$Performance = $false,
    [switch]$Memory = $false,
    [switch]$Cache = $false,
    [switch]$Latest = $false,
    [switch]$All = $false
)

$ErrorActionPreference = "Continue"

Write-Host "`n================================" -ForegroundColor Cyan
Write-Host "DarkThumbs Diagnostics Viewer" -ForegroundColor Cyan
Write-Host "================================`n" -ForegroundColor Cyan

$logPath = "$env:LOCALAPPDATA\DarkThumbs\logs"
$cachePath = "$env:LOCALAPPDATA\DarkThumbs\cache"

if ($All) {
    $Logs = $true
    $Performance = $true
    $Memory = $true
    $Cache = $true
}

# Check if diagnostics are enabled
$settingsPath = "HKCU:\Software\DarkThumbs\Settings"

Write-Host "Diagnostic Status:" -ForegroundColor Yellow
if (Test-Path $settingsPath) {
    $debugLogging = Get-ItemProperty -Path $settingsPath -Name "DebugLogging" -ErrorAction SilentlyContinue
    $profiling = Get-ItemProperty -Path $settingsPath -Name "EnableProfiling" -ErrorAction SilentlyContinue
    $memTracking = Get-ItemProperty -Path $settingsPath -Name "EnableMemoryTracking" -ErrorAction SilentlyContinue
    
    $loggingStatus = if ($debugLogging.DebugLogging -eq 1) { "ENABLED" } else { "DISABLED" }
    $profilingStatus = if ($profiling.EnableProfiling -eq 1) { "ENABLED" } else { "DISABLED" }
    $memTrackingStatus = if ($memTracking.EnableMemoryTracking -eq 1) { "ENABLED" } else { "DISABLED" }
    
    Write-Host "  Error Logging:    " -NoNewline
    Write-Host $loggingStatus -ForegroundColor $(if ($debugLogging.DebugLogging -eq 1) { "Green" } else { "Yellow" })
    
    Write-Host "  Profiling:        " -NoNewline
    Write-Host $profilingStatus -ForegroundColor $(if ($profiling.EnableProfiling -eq 1) { "Green" } else { "Yellow" })
    
    Write-Host "  Memory Tracking:  " -NoNewline
    Write-Host $memTrackingStatus -ForegroundColor $(if ($memTracking.EnableMemoryTracking -eq 1) { "Green" } else { "Yellow" })
} else {
    Write-Host "  Not configured (run Enable-DarkThumbsDiagnostics.ps1)" -ForegroundColor Yellow
}

Write-Host ""

# View Logs
if ($Logs -or $Latest) {
    Write-Host "Recent Log Files:" -ForegroundColor Cyan
    if (Test-Path $logPath) {
        $logFiles = Get-ChildItem -Path $logPath -Filter "*.log" | Sort-Object LastWriteTime -Descending
        
        if ($logFiles.Count -gt 0) {
            if ($Latest) {
                $latestLog = $logFiles[0]
                Write-Host "  Latest: $($latestLog.Name)" -ForegroundColor Green
                Write-Host "  Size: $([math]::Round($latestLog.Length / 1KB, 2)) KB" -ForegroundColor Gray
                Write-Host "  Modified: $($latestLog.LastWriteTime)" -ForegroundColor Gray
                Write-Host "`nLast 20 lines:" -ForegroundColor Yellow
                Get-Content $latestLog.FullName -Tail 20 | ForEach-Object {
                    if ($_ -match "\[ERROR\]|\[CRIT\]") {
                        Write-Host "  $_" -ForegroundColor Red
                    } elseif ($_ -match "\[WARN\]") {
                        Write-Host "  $_" -ForegroundColor Yellow
                    } else {
                        Write-Host "  $_" -ForegroundColor Gray
                    }
                }
            } else {
                foreach ($log in $logFiles | Select-Object -First 5) {
                    Write-Host "  $($log.Name) - $([math]::Round($log.Length / 1KB, 2)) KB" -ForegroundColor Gray
                }
            }
        } else {
            Write-Host "  No log files found" -ForegroundColor Yellow
        }
    } else {
        Write-Host "  Log directory not found: $logPath" -ForegroundColor Yellow
    }
    Write-Host ""
}

# View Performance Stats
if ($Performance) {
    Write-Host "Performance Metrics:" -ForegroundColor Cyan
    # Note: Performance stats are in-memory, would need to be exported
    Write-Host "  Performance metrics are tracked in-memory" -ForegroundColor Gray
    Write-Host "  Enable profiling to collect timing data" -ForegroundColor Gray
    Write-Host ""
}

# View Memory Stats
if ($Memory) {
    Write-Host "Memory Usage:" -ForegroundColor Cyan
    # Note: Memory stats are in-memory, would need to be exported
    Write-Host "  Memory tracking is active when enabled" -ForegroundColor Gray
    Write-Host "  Peak usage and allocation counts tracked per category" -ForegroundColor Gray
    Write-Host ""
}

# View Cache Stats
if ($Cache) {
    Write-Host "Cache Statistics:" -ForegroundColor Cyan
    
    if (Test-Path $cachePath) {
        $cacheFiles = Get-ChildItem -Path $cachePath -Filter "*.png" -ErrorAction SilentlyContinue
        $totalSize = ($cacheFiles | Measure-Object -Property Length -Sum).Sum
        
        Write-Host "  Disk Cache Path: $cachePath" -ForegroundColor Gray
        Write-Host "  Cached Thumbnails: $($cacheFiles.Count)" -ForegroundColor Green
        Write-Host "  Total Size: $([math]::Round($totalSize / 1MB, 2)) MB" -ForegroundColor Green
        
        if ($cacheFiles.Count -gt 0) {
            $oldestCache = $cacheFiles | Sort-Object LastAccessTime | Select-Object -First 1
            $newestCache = $cacheFiles | Sort-Object LastAccessTime -Descending | Select-Object -First 1
            
            Write-Host "  Oldest Access: $($oldestCache.LastAccessTime.ToString('yyyy-MM-dd HH:mm:ss'))" -ForegroundColor Gray
            Write-Host "  Newest Access: $($newestCache.LastAccessTime.ToString('yyyy-MM-dd HH:mm:ss'))" -ForegroundColor Gray
        }
    } else {
        Write-Host "  Cache directory not found: $cachePath" -ForegroundColor Yellow
    }
    Write-Host ""
}

# Configuration info
$cacheRegPath = "HKCU:\Software\DarkThumbs\Cache"
if (Test-Path $cacheRegPath) {
    Write-Host "Cache Configuration:" -ForegroundColor Cyan
    $memCacheSize = Get-ItemProperty -Path $cacheRegPath -Name "MemoryCacheSizeMB" -ErrorAction SilentlyContinue
    $diskCacheSize = Get-ItemProperty -Path $cacheRegPath -Name "DiskCacheSizeMB" -ErrorAction SilentlyContinue
    
    if ($memCacheSize) {
        Write-Host "  Memory Cache Limit: $($memCacheSize.MemoryCacheSizeMB) MB" -ForegroundColor Gray
    }
    if ($diskCacheSize) {
        Write-Host "  Disk Cache Limit: $($diskCacheSize.DiskCacheSizeMB) MB" -ForegroundColor Gray
    }
    Write-Host ""
}

Write-Host "Usage Examples:" -ForegroundColor Yellow
Write-Host "  View latest log:       -Latest" -ForegroundColor Gray
Write-Host "  View all logs:         -Logs" -ForegroundColor Gray
Write-Host "  View cache stats:      -Cache" -ForegroundColor Gray
Write-Host "  View everything:       -All" -ForegroundColor Gray
Write-Host ""
