# Performance Metrics System

## Overview
ExplorerLens includes a comprehensive performance metrics system to track thumbnail generation statistics, memory usage, and format-specific performance data.

## Features

### 📊 Tracked Metrics
- **Success/Failure Rates**: Percentage of successful thumbnail generations
- **Format Breakdown**: Per-format statistics (ZIP, RAR, CBZ, CBR, 7Z, WebP, AVIF, JXL)
- **Memory Usage**: Peak memory and total allocated memory tracking
- **Cache Performance**: Hit/miss rates for thumbnail caching
- **Timing Data**: Generation time per thumbnail (via performance profiler)

### 🎯 Use Cases
- Performance regression testing
- Format support validation
- Memory leak detection
- Cache efficiency optimization
- Production monitoring

## Components

### 1. Metrics Collector (`metrics_collector.h`)
Core metrics collection system with:
- Atomic counters for thread-safe tracking
- Format-specific counters
- Memory usage monitoring
- Export to CSV/JSON

### 2. Thumbnail Metrics Scope
RAII helper for automatic metrics recording:
```cpp
ExplorerLens::ThumbnailMetricsScope metricsScope("cbz");
// ... generate thumbnail ...
metricsScope.SetSuccess(true);
// Metrics recorded on scope exit
```

### 3. Performance Profiler (`performance_profiler.h`)
High-resolution timing metrics:
```cpp
PROFILE_FUNCTION(); // Times entire function
PROFILE_SCOPE("decode_image"); // Times specific code block
```

### 4. Metrics Viewer Tool
Command-line tool to view and export metrics.

## Usage

### Enable Profiling
Add registry key to enable performance profiling:
```
HKEY_CURRENT_USER\Software\ExplorerLens\Settings
EnableProfiling = 1 (DWORD)
```

### View Metrics in Real-Time
```powershell
# Display current metrics
.\tools\ExplorerLensMetrics.exe

# Or
.\tools\ExplorerLensMetrics.exe show
```

Output example:
```
============================================================
ExplorerLens Performance Metrics
============================================================

📊 Thumbnail Generation
  Total Attempts:  1243
  Successful:      1189 (95.7%)
  Failed:          54

💾 Cache Performance
  Cache Hits:      876
  Cache Misses:    367
  Hit Rate:        70.5%

📁 Format Breakdown
  ZIP:     342
  CBZ:     487
  7Z:      156
  WebP:    189
  AVIF:     69

💻 Memory Usage
  Peak Memory:     124.45 MB
  Total Allocated: 1874.32 MB
  Current Usage:   42.18 MB
```

### Export Metrics

**CSV Export:**
```powershell
.\tools\ExplorerLensMetrics.exe export csv metrics.csv
```

**JSON Export:**
```powershell
.\tools\ExplorerLensMetrics.exe export json metrics.json
```

JSON format:
```json
{
  "thumbnailGeneration": {
    "totalAttempts": 1243,
    "successCount": 1189,
    "failureCount": 54,
    "successRate": 95.7
  },
  "cache": {
    "hits": 876,
    "misses": 367,
    "hitRate": 70.5
  },
  "formatBreakdown": {
    "zip": 342,
    "cbz": 487,
    "7z": 156,
    "webp": 189,
    "avif": 69
  },
  "memory": {
    "peakMB": 124.45,
    "totalAllocatedMB": 1874.32
  }
}
```

### Reset Metrics
```powershell
.\tools\ExplorerLensMetrics.exe reset
```

## Integration

### Automatic Tracking
Metrics are automatically collected when:
1. Shell extension generates thumbnails (`GetThumbnail()`)
2. Engine adapter processes images
3. Cache lookups occur

### Manual Tracking
Add metrics to custom code:
```cpp
#include "metrics_collector.h"

// Track a thumbnail generation attempt
ExplorerLens::ThumbnailMetricsScope scope("custom_format");
bool success = GenerateCustomThumbnail();
scope.SetSuccess(success);

// Record cache hit/miss
MetricsCollector::Instance().RecordCacheHit(cacheFound);
```

## Performance Impact

### Overhead
- **Metrics Disabled**: Zero overhead (no-op operations)
- **Metrics Enabled**: ~0.01ms per thumbnail (atomic increments)
- **Memory**: ~2KB for metrics storage

### Thread Safety
All metrics operations are thread-safe using:
- `std::atomic` for counters
- `std::mutex` for map access
- Lock-free reads where possible

## Production Monitoring

### Automated Reports
Schedule metrics exports:
```powershell
# Windows Task Scheduler - Daily at 2 AM
$action = New-ScheduledTaskAction -Execute "ExplorerLensMetrics.exe" `
    -Argument "export json C:\Logs\metrics_$(Get-Date -Format 'yyyyMMdd').json"
$trigger = New-ScheduledTaskTrigger -Daily -At 2am
Register-ScheduledTask -TaskName "ExplorerLens Metrics Export" `
    -Action $action -Trigger $trigger
```

### Log Analysis
Parse exported JSON for trends:
```powershell
# Check success rate over time
Get-ChildItem C:\Logs\metrics_*.json | ForEach-Object {
    $data = Get-Content $_ | ConvertFrom-Json
    [PSCustomObject]@{
        Date = $_.BaseName -replace 'metrics_',''
        SuccessRate = $data.thumbnailGeneration.successRate
        CacheHitRate = $data.cache.hitRate
        PeakMemoryMB = $data.memory.peakMB
    }
} | Format-Table -AutoSize
```

## Troubleshooting

### Metrics Not Recording
1. **Check profiling is enabled**:
   ```
   HKEY_CURRENT_USER\Software\ExplorerLens\Settings\EnableProfiling = 1
   ```

2. **Verify shell extension is loaded**:
   - Open test archive in Explorer
   - Check Event Viewer for ExplorerLens events

3. **Restart Explorer**:
   ```powershell
   taskkill /f /im explorer.exe
   start explorer.exe
   ```

### High Memory Usage
If peak memory exceeds expectations:
1. Export metrics: `ExplorerLensMetrics.exe export json report.json`
2. Check `formatBreakdown` for problematic formats
3. Review `totalAllocatedMB` vs `peakMB` ratio
4. Enable detailed logging for memory profiling

### Low Success Rate
If success rate < 90%:
1. Check format breakdown for failing formats
2. Review Windows Event Viewer for errors
3. Test failing formats manually with LENSManager
4. Check build configuration (Debug vs Release)

## Best Practices

### Development
- Enable profiling during testing
- Export metrics after test runs
- Compare metrics before/after changes
- Monitor memory usage for leaks

### Production
- Enable profiling for 1-2 days initially
- Export daily metrics for baseline
- Disable profiling after baseline established
- Re-enable for troubleshooting

### CI/CD Integration
```yaml
# Example: Azure Pipelines
- script: |
    ExplorerLensMetrics.exe export json $(Build.ArtifactStagingDirectory)/metrics.json
  displayName: 'Export Performance Metrics'
  condition: succeededOrFailed()
  
- task: PublishBuildArtifacts@1
  inputs:
    pathToPublish: '$(Build.ArtifactStagingDirectory)/metrics.json'
    artifactName: 'performance-metrics'
```

## Future Enhancements

Planned features:
- [ ] Real-time metrics dashboard (web UI)
- [ ] Histogram for generation times
- [ ] P50/P95/P99 percentile tracking
- [ ] Alerts for threshold violations
- [ ] Integration with Application Insights
- [ ] Per-directory metrics breakdown

---

**See Also:**
- [performance_profiler.h](../LENSShell/performance_profiler.h) - Timing metrics
- [metrics_collector.h](../LENSShell/metrics_collector.h) - Core metrics system
- [ExplorerLensMetrics.cpp](../tools/ExplorerLensMetrics.cpp) - Viewer tool source

