# Monitor-Build-Safe.ps1
# Safe build monitoring that doesn't interfere with the build process
# Uses file system watching instead of shell command interruption

param(
    [Parameter(Mandatory = $true)]
    [string]$LogFile,
    
    [Parameter(Mandatory = $false)]
    [int]$TailLines = 30,
    
    [Parameter(Mandatory = $false)]
    [int]$RefreshSeconds = 2
)

$ErrorActionPreference = "Stop"

# Validate log file exists or can be created
$logPath = Resolve-Path $LogFile -ErrorAction SilentlyContinue
if (-not $logPath) {
    $logPath = Join-Path $PWD $LogFile
    if (-not (Test-Path $logPath)) {
        Write-Host "⏳ Waiting for log file to be created: $logPath" -ForegroundColor Yellow
        
        # Wait up to 30 seconds for log file to appear
        $waited = 0
        while (-not (Test-Path $logPath) -and $waited -lt 30) {
            Start-Sleep -Seconds 1
            $waited++
        }
        
        if (-not (Test-Path $logPath)) {
            Write-Error "Log file never created: $logPath"
            exit 1
        }
    }
}

Write-Host "📊 Monitoring Build Log: $logPath" -ForegroundColor Cyan
Write-Host "Press Ctrl+C to stop monitoring (build will continue)" -ForegroundColor Gray
Write-Host ""

$lastSize = 0
$buildComplete = $false

try {
    while (-not $buildComplete) {
        if (Test-Path $logPath) {
            $file = Get-Item $logPath
            
            # Only read if file has grown
            if ($file.Length -gt $lastSize) {
                $content = Get-Content $logPath -Tail $TailLines
                
                # Clear screen and show latest content
                # Clear-Host  # Disabled to keep history
                Write-Host "─────────────────────────────────────────────────────────" -ForegroundColor DarkGray
                Write-Host "Log: $($file.Name) | Size: $([math]::Round($file.Length/1KB, 1)) KB | Last Modified: $($file.LastWriteTime.ToString('HH:mm:ss'))" -ForegroundColor Gray
                Write-Host "─────────────────────────────────────────────────────────" -ForegroundColor DarkGray
                
                foreach ($line in $content) {
                    # Colorize output
                    if ($line -match "error C|Error:|FAILED") {
                        Write-Host $line -ForegroundColor Red
                    } elseif ($line -match "warning C|Warning:") {
                        Write-Host $line -ForegroundColor Yellow
                    } elseif ($line -match "Build succeeded|succeeded") {
                        Write-Host $line -ForegroundColor Green
                    } elseif ($line -match "Compiling|Linking|Generating") {
                        Write-Host $line -ForegroundColor Cyan
                    } else {
                        Write-Host $line
                    }
                }
                
                $lastSize = $file.Length
                
                # Check for build completion
                $lastFewLines = Get-Content $logPath -Tail 5
                if ($lastFewLines -match "Build succeeded|Build FAILED") {
                    $buildComplete = $true
                    Write-Host ""
                    if ($lastFewLines -match "Build succeeded") {
                        Write-Host "✓ BUILD COMPLETED SUCCESSFULLY" -ForegroundColor Green
                    } else {
                        Write-Host "✗ BUILD FAILED" -ForegroundColor Red
                    }
                }
            }
        }
        
        if (-not $buildComplete) {
            Start-Sleep -Seconds $RefreshSeconds
        }
    }
} catch {
    Write-Host "Monitoring stopped: $_" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Log file: $logPath" -ForegroundColor Gray
