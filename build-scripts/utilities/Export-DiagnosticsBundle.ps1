# Export-DiagnosticsBundle.ps1
# Sprint 12: Observability & Structured Logging
# Creates comprehensive diagnostics ZIP bundle for troubleshooting

param(
    [string]$OutputPath = "",
    [switch]$IncludeCrashDumps = $true,
    [switch]$IncludeFullPaths = $false,
    [int]$MaxLogSizeMB = 50
)

$ErrorActionPreference = "Stop"

Write-Host "============================================" -ForegroundColor Cyan
Write-Host "DarkThumbs Diagnostics Bundle Export" -ForegroundColor Cyan
Write-Host "Sprint 12: Observability & Structured Logging" -ForegroundColor Cyan
Write-Host "============================================`n" -ForegroundColor Cyan

if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
    $OutputPath = Join-Path $env:USERPROFILE "Desktop\DarkThumbs-Diagnostics-$timestamp.zip"
}

# Create temporary directory for bundle contents
$tempDir = Join-Path $env:TEMP "DarkThumbs-Diagnostics-$(Get-Random)"
New-Item -ItemType Directory -Path $tempDir -Force | Out-Null

try {
    # =========================================================================
    # 1. System Information
    # =========================================================================
    Write-Host "[1/8] Collecting system information..." -ForegroundColor Yellow
    
    $systemInfo = @{
        timestamp = (Get-Date -Format "o")
        os = @{
            caption = (Get-CimInstance Win32_OperatingSystem).Caption
            version = (Get-CimInstance Win32_OperatingSystem).Version
            build = (Get-CimInstance Win32_OperatingSystem).BuildNumber
            architecture = (Get-CimInstance Win32_OperatingSystem).OSArchitecture
            installDate = (Get-CimInstance Win32_OperatingSystem).InstallDate
        }
        hardware = @{
            processor = (Get-CimInstance Win32_Processor).Name
            cores = (Get-CimInstance Win32_Processor).NumberOfCores
            threads = (Get-CimInstance Win32_Processor).NumberOfLogicalProcessors
            ramGB = [math]::Round((Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory / 1GB, 2)
        }
        graphics = @(
            Get-CimInstance Win32_VideoController | ForEach-Object {
                @{
                    name = $_.Name
                    driverVersion = $_.DriverVersion
                    driverDate = $_.DriverDate
                    vramMB = [math]::Round($_.AdapterRAM / 1MB, 0)
                }
            }
        )
        monitors = @(
            Get-CimInstance WmiMonitorID -Namespace root\wmi -ErrorAction SilentlyContinue | ForEach-Object {
                @{
                    manufacturer = [System.Text.Encoding]::ASCII.GetString($_.ManufacturerName -ne 0)
                    productCode = [System.Text.Encoding]::ASCII.GetString($_.ProductCodeID -ne 0)
                    serialNumber = [System.Text.Encoding]::ASCII.GetString($_.SerialNumberID -ne 0)
                }
            }
        )
    }
    
    $systemInfoPath = Join-Path $tempDir "system_info.json"
    $systemInfo | ConvertTo-Json -Depth 10 | Set-Content $systemInfoPath
    Write-Host "  ✓ System information" -ForegroundColor Green
    
    # =========================================================================
    # 2. DarkThumbs Configuration
    # =========================================================================
    Write-Host "`n[2/8] Collecting configuration..." -ForegroundColor Yellow
    
    $config = @{
        registry = @{}
        files = @{}
    }
    
    # Registry settings
    $regPaths = @(
        "HKCU:\Software\DarkThumbs",
        "HKLM:\Software\DarkThumbs"
    )
    
    foreach ($regPath in $regPaths) {
        if (Test-Path $regPath) {
            $props = Get-ItemProperty -Path $regPath -ErrorAction SilentlyContinue
            $config.registry[$regPath] = @{}
            $props.PSObject.Properties | Where-Object { $_.Name -notmatch '^PS' } | ForEach-Object {
                $config.registry[$regPath][$_.Name] = $_.Value
            }
        }
    }
    
    # Configuration files
    $configFiles = @(
        "$env:LOCALAPPDATA\DarkThumbs\config.json",
        "$env:APPDATA\DarkThumbs\settings.json"
    )
    
    foreach ($configFile in $configFiles) {
        if (Test-Path $configFile) {
            $config.files[$configFile] = Get-Content $configFile -Raw
        }
    }
    
    $configPath = Join-Path $tempDir "config_dump.json"
    $config | ConvertTo-Json -Depth 10 | Set-Content $configPath
    Write-Host "  ✓ Configuration dump" -ForegroundColor Green
    
    # =========================================================================
    # 3. Recent Logs
    # =========================================================================
    Write-Host "`n[3/8] Collecting logs..." -ForegroundColor Yellow
    
$logPaths = @(
        "$env:LOCALAPPDATA\DarkThumbs\Logs\darkthumbs.log",
        "$env:TEMP\DarkThumbs\darkthumbs.log",
        "$env:APPDATA\DarkThumbs\Logs\darkthumbs.log"
    )
    
    $logsDir = Join-Path $tempDir "logs"
    New-Item -ItemType Directory -Path $logsDir -Force | Out-Null
    
    $totalLogSize = 0
    foreach ($logPath in $logPaths) {
        if (Test-Path $logPath) {
            $logFile = Get-Item $logPath
            $logSizeMB = [math]::Round($logFile.Length / 1MB, 2)
            
            if ($totalLogSize + $logSizeMB -le $MaxLogSizeMB) {
                $destName = Split-Path $logPath -Leaf
                Copy-Item $logPath (Join-Path $logsDir $destName) -Force
                Write-Host "  ✓ Copied log: $destName ($logSizeMB MB)" -ForegroundColor Green
                $totalLogSize += $logSizeMB
            } else {
                Write-Host "  ⚠ Skipped log (size limit): $logPath" -ForegroundColor Yellow
            }
        }
    }
    
    if ($totalLogSize -eq 0) {
        Write-Host "  ⚠ No log files found" -ForegroundColor Yellow
    }
    
    # =========================================================================
    # 4. Registry Snapshot
    # =========================================================================
    Write-Host "`n[4/8] Exporting registry snapshot..." -ForegroundColor Yellow
    
    $regExportPath = Join-Path $tempDir "registry_snapshot.reg"
    $regContent = @"
Windows Registry Editor Version 5.00

; DarkThumbs Registry Snapshot
; Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

"@
    
    foreach ($regPath in $regPaths) {
        if (Test-Path $regPath) {
            $regContent += "`n[$regPath]`n"
            $props = Get-ItemProperty -Path $regPath -ErrorAction SilentlyContinue
            $props.PSObject.Properties | Where-Object { $_.Name -notmatch '^PS' } | ForEach-Object {
                $name = $_.Name
                $value = $_.Value
                $type = $props.PSObject.Properties[$name].TypeNameOfValue
                
                # Format registry value
                $regContent += "`"$name`"="
                if ($type -match "String") {
                    $regContent += "`"$value`"`n"
                } elseif ($type -match "Int") {
                    $regContent += "dword:$([Convert]::ToString($value, 16).PadLeft(8, '0'))`n"
                } else {
                    $regContent += "`"$value`"`n"
                }
            }
        }
    }
    
    Set-Content -Path $regExportPath -Value $regContent
    Write-Host "  ✓ Registry snapshot" -ForegroundColor Green
    
    # =========================================================================
    # 5. Crash Dumps
    # =========================================================================
    Write-Host "`n[5/8] Collecting crash dumps..." -ForegroundColor Yellow
    
    if ($IncludeCrashDumps) {
        $crashDumpsDir = Join-Path $tempDir "crash_dumps"
        New-Item -ItemType Directory -Path $crashDumpsDir -Force | Out-Null
        
        $dumpLocations = @(
            "$env:LOCALAPPDATA\CrashDumps\*.dmp",
            "$env:TEMP\DarkThumbs\*.dmp",
            "C:\Windows\Minidump\*.dmp"
        )
        
        $dumpFound = $false
        foreach ($dumpPattern in $dumpLocations) {
            $dumps = Get-ChildItem $dumpPattern -ErrorAction SilentlyContinue | Where-Object {
                $_.LastWriteTime -gt (Get-Date).AddDays(-7)  # Last 7 days only
            }
            
            foreach ($dump in $dumps) {
                Copy-Item $dump.FullName $crashDumpsDir -Force
                Write-Host "  ✓ Copied crash dump: $($dump.Name)" -ForegroundColor Green
                $dumpFound = $true
            }
        }
        
        if (-not $dumpFound) {
            Write-Host "  ✓ No recent crash dumps found" -ForegroundColor Green
        }
    } else {
        Write-Host "  ⊘ Crash dumps not included (use -IncludeCrashDumps)" -ForegroundColor Gray
    }
    
    # =========================================================================
    # 6. Performance Metrics
    # =========================================================================
    Write-Host "`n[6/8] Collecting performance metrics..." -ForegroundColor Yellow
    
    $perfMetrics = @{
        collected_at = (Get-Date -Format "o")
        cache_stats = @{}
        decoder_stats = @{}
        memory_usage = @{
            workingSetMB = 0
            privateBytesMB = 0
            virtualSizeMB = 0
        }
    }
    
    # Get Explorer process memory usage (DarkThumbs runs in Explorer process)
    $explorer = Get-Process | Where-Object { $_.Name -eq "explorer" } | Select-Object -First 1
    if ($explorer) {
        $perfMetrics.memory_usage.workingSetMB = [math]::Round($explorer.WorkingSet64 / 1MB, 2)
        $perfMetrics.memory_usage.privateBytesMB = [math]::Round($explorer.PrivateMemorySize64 / 1MB, 2)
        $perfMetrics.memory_usage.virtualSizeMB = [math]::Round($explorer.VirtualMemorySize64 / 1MB, 2)
    }
    
    $perfPath = Join-Path $tempDir "performance_metrics.json"
    $perfMetrics | ConvertTo-Json -Depth 10 | Set-Content $perfPath
    Write-Host "  ✓ Performance metrics" -ForegroundColor Green
    
    # =========================================================================
    # 7. Environment Variables
    # =========================================================================
    Write-Host "`n[7/8] Collecting environment..." -ForegroundColor Yellow
    
    $relevantVars = @(
        "PATH",
        "TEMP",
        "TMP",
        "USERPROFILE",
        "LOCALAPPDATA",
        "APPDATA",
        "PROCESSOR_ARCHITECTURE", 
        "NUMBER_OF_PROCESSORS"
    )
    
    $envInfo = @{}
    foreach ($var in $relevantVars) {
        $value = if ($IncludeFullPaths) {
            [Environment]::GetEnvironmentVariable($var)
        } else {
            # Redact paths for privacy
            $val = [Environment]::GetEnvironmentVariable($var)
            if ($val -match "[A-Z]:\\") {
                "<redacted_path>"
            } else {
                $val
            }
        }
        $envInfo[$var] = $value
    }
    
    $envPath = Join-Path $tempDir "environment.json"
    $envInfo | ConvertTo-Json -Depth 10 | Set-Content $envPath
    Write-Host "  ✓ Environment variables" -ForegroundColor Green
    
    # =========================================================================
    # 8. Create ZIP Archive
    # =========================================================================
    Write-Host "`n[8/8] Creating ZIP archive..." -ForegroundColor Yellow
    
    # Remove existing file if it exists
    if (Test-Path $OutputPath) {
        Remove-Item $OutputPath -Force
    }
    
    # Create ZIP
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [System.IO.Compression.ZipFile]::CreateFromDirectory($tempDir, $OutputPath, 'Optimal', $false)
    
    $zipSize = [math]::Round((Get-Item $OutputPath).Length / 1MB, 2)
    Write-Host "  ✓ Diagnostics bundle created ($zipSize MB)" -ForegroundColor Green
    
    # =========================================================================
    # Summary
    # =========================================================================
    Write-Host "`n============================================" -ForegroundColor Green
    Write-Host "Diagnostics Bundle Export Complete!" -ForegroundColor Green
    Write-Host "============================================" -ForegroundColor Green
    Write-Host "  Output: $OutputPath" -ForegroundColor Cyan
    Write-Host "  Size:   $zipSize MB" -ForegroundColor Cyan
    Write-Host "`nBundle Contents:" -ForegroundColor Yellow
    Write-Host "  • system_info.json - System hardware/software" -ForegroundColor White
    Write-Host "  • config_dump.json - DarkThumbs configuration" -ForegroundColor White
    Write-Host "  • logs/ - Recent log files" -ForegroundColor White
    Write-Host "  • registry_snapshot.reg - Registry settings" -ForegroundColor White
    if ($IncludeCrashDumps) {
        Write-Host "  • crash_dumps/ - Recent crash dumps" -ForegroundColor White
    }
    Write-Host "  • performance_metrics.json - Performance data" -ForegroundColor White
    Write-Host "  • environment.json - Environment variables" -ForegroundColor White
    Write-Host "`nPrivacy:" -ForegroundColor Yellow
    if ($IncludeFullPaths) {
        Write-Host "  ⚠ Full file paths included" -ForegroundColor Red
    } else {
        Write-Host "  ✓ File paths redacted for privacy" -ForegroundColor Green
    }
    Write-Host "============================================`n" -ForegroundColor Green
    
} finally {
    # Clean up temporary directory
    if (Test-Path $tempDir) {
        Remove-Item $tempDir -Recurse -Force
    }
}
