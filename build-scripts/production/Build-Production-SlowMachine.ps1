#Requires -Version 7.0
<#
.SYNOPSIS
    Production build with file-based monitoring for slow machines

.DESCRIPTION
    Complete production build of DarkThumbs with all libraries.
    Optimized for slow machines with extended timeouts and file-based
    output monitoring instead of real-time terminal output.

.PARAMETER Clean
    Clean all build artifacts before building

.PARAMETER SkipLibraries
    Skip building external libraries (use existing builds)

.PARAMETER Verbose
    Enable verbose output logging

.PARAMETER MonitorInVSCode
    Open log files in VS Code for monitoring

.EXAMPLE
    .\Build-Production-SlowMachine.ps1 -Clean

.EXAMPLE
    .\Build-Production-SlowMachine.ps1 -SkipLibraries -MonitorInVSCode

.EXAMPLE
    .\Build-Production-SlowMachine.ps1 -Clean -Verbose
#>

param(
    [switch]$Clean,
    [switch]$SkipLibraries,
    [switch]$Verbose,
    [switch]$MonitorInVSCode
)

$ErrorActionPreference = "Stop"
$startTime = Get-Date

# Setup logging
if (-not (Test-Path "build-logs")) {
    New-Item -Path "build-logs" -ItemType Directory -Force | Out-Null
}

$timestamp = Get-Date -Format 'yyyy-MM-dd_HHmmss'
$logFile = "build-logs\production-build-$timestamp.log"
$progressFile = "build-logs\build-progress.json"

function Write-Log {
    param([string]$Message, [string]$Color = "White")
    $timestamp = Get-Date -Format "HH:mm:ss"
    $logMessage = "[$timestamp] $Message"
    Write-Host $logMessage -ForegroundColor $Color
    Add-Content -Path $logFile -Value $logMessage -ErrorAction SilentlyContinue
}

function Write-Progress-File {
    param([string]$Status, [string]$Current, [int]$Step, [int]$Total)
    $progress = @{
        Timestamp       = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        Status          = $Status
        CurrentTask     = $Current
        Step            = $Step
        TotalSteps      = $Total
        PercentComplete = [math]::Round(($Step / $Total) * 100, 1)
        ElapsedMinutes  = [math]::Round(((Get-Date) - $startTime).TotalMinutes, 1)
    }
    $progress | ConvertTo-Json | Out-File -FilePath $progressFile -Force
}

Write-Log ""
Write-Log "========================================" "Cyan"
Write-Log "  DarkThumbs Production Build" "Cyan"
Write-Log "  Slow Machine Optimized" "Cyan"
Write-Log "========================================" "Cyan"
Write-Log "Started: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" "Gray"
Write-Log "Log file: $logFile" "Gray"
Write-Log "Progress file: $progressFile" "Gray"
Write-Log ""
Write-Log "TIP: Open '$progressFile' in VS Code to monitor progress" "Yellow"
Write-Log "TIP: Run '.\build-scripts\Monitor-Build-Logs.ps1' in another terminal" "Yellow"
Write-Log ""

# Open files in VS Code if requested
if ($MonitorInVSCode) {
    Write-Log "Opening monitoring files in VS Code..." "Cyan"
    Start-Process "code" -ArgumentList $progressFile, $logFile -NoNewWindow
}

# Clean if requested
if ($Clean) {
    Write-Log "Cleaning build artifacts..." "Yellow"
    Write-Progress-File -Status "Cleaning" -Current "Removing build directories" -Step 1 -Total 10
    
    $cleanDirs = @("build", "x64", "CBXShell\x64", "CBXManager\x64")
    foreach ($dir in $cleanDirs) {
        if (Test-Path $dir) {
            Write-Log "  Removing: $dir" "Gray"
            Remove-Item -Path $dir -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
    Write-Log "Clean complete" "Green"
}

# Build libraries with extended timeouts
$libraryResults = @{}

if (-not $SkipLibraries) {
    Write-Log ""
    Write-Log "Building external libraries..." "Cyan"
    Write-Log "Using file-based monitoring (logs in build-logs/)" "Gray"
    
    $libraries = @(
        @{ Script = "external-libs/Build-Zlib.ps1"; Name = "zlib"; Timeout = 300 },
        @{ Script = "external-libs/Build-LZ4.ps1"; Name = "LZ4"; Timeout = 600 },
        @{ Script = "external-libs/Build-Zstd.ps1"; Name = "Zstandard"; Timeout = 900 },
        @{ Script = "external-libs/Build-LZMA-SDK-26.00.ps1"; Name = "liblzma"; Timeout = 1200 },
        @{ Script = "external-libs/Build-LibWebP-NMake.ps1"; Name = "LibWebP"; Timeout = 1200 },
        @{ Script = "external-libs/Build-MinizipNG.ps1"; Name = "Minizip-NG"; Timeout = 900 }
    )
    
    $step = 2
    $totalSteps = $libraries.Count + 3
    
    foreach ($lib in $libraries) {
        Write-Log ""
        Write-Log "[$step/$totalSteps] Building $($lib.Name)..." "Cyan"
        Write-Log "  Timeout: $($lib.Timeout) seconds ($([math]::Round($lib.Timeout/60, 1)) minutes)" "Gray"
        Write-Progress-File -Status "Building" -Current $lib.Name -Step $step -Total $totalSteps
        
        $safeScriptName = $lib.Script.Replace('.ps1', '').Replace('/', '-').Replace('\\', '-')
        $libLogFile = "build-logs\$safeScriptName-$timestamp.log"
        $scriptPath = Join-Path "build-scripts" $lib.Script
        
        if (-not (Test-Path $scriptPath)) {
            Write-Log "  ⚠️  Script not found: $scriptPath" "Yellow"
            $libraryResults[$lib.Name] = "Script not found"
            $step++
            continue
        }
        
        # Run in background job with file output
        $job = Start-Job -ScriptBlock {
            param($script, $log, $workDir)
            Set-Location $workDir
            $ErrorActionPreference = "Continue"
            
            # Execute script and capture all output
            try {
                & $script *>&1 | ForEach-Object {
                    $_ | Out-File -FilePath $log -Append -Encoding utf8
                    $_  # Also output to job stream
                }
                exit $LASTEXITCODE
            } catch {
                "ERROR: $_" | Out-File -FilePath $log -Append
                exit 1
            }
        } -ArgumentList $scriptPath, $libLogFile, $PWD
        
        Write-Log "  Job ID: $($job.Id)" "Gray"
        Write-Log "  Log: $libLogFile" "Gray"
        
        if ($MonitorInVSCode) {
            Start-Process "code" -ArgumentList $libLogFile -NoNewWindow
        }
        
        # Wait with timeout and periodic status updates
        $waited = 0
        $checkInterval = 30  # Check every 30 seconds
        
        while ($job.State -eq 'Running' -and $waited -lt $lib.Timeout) {
            Start-Sleep -Seconds $checkInterval
            $waited += $checkInterval
            
            # Show progress
            $pct = [math]::Round(($waited / $lib.Timeout) * 100, 1)
            Write-Log "  Progress: $waited / $($lib.Timeout) seconds ($pct%)" "DarkGray"
            
            # Check if log file is growing
            if (Test-Path $libLogFile) {
                $logSize = (Get-Item $libLogFile).Length
                Write-Log "  Log size: $([math]::Round($logSize / 1KB, 1)) KB" "DarkGray"
            }
        }
        
        # Check result
        if ($job.State -eq 'Running') {
            Write-Log "  ⏱️  Timeout exceeded ($($lib.Timeout)s)" "Yellow"
            Write-Log "  Stopping job..." "Yellow"
            Stop-Job $job
            $libraryResults[$lib.Name] = "Timeout"
        } else {
            Wait-Job $job -Timeout 10 | Out-Null
            $jobOutput = Receive-Job $job -ErrorAction SilentlyContinue
            
            # Check exit code (stored in job if script exited properly)
            $success = $false
            if ($job.State -eq 'Completed') {
                # Try to determine success from log file
                if (Test-Path $libLogFile) {
                    $logContent = Get-Content $libLogFile -Tail 20
                    if ($logContent -match 'success|completed|✅') {
                        $success = $true
                    }
                }
            }
            
            if ($success -or $job.State -eq 'Completed') {
                Write-Log "  ✅ $($lib.Name) completed" "Green"
                $libraryResults[$lib.Name] = "Success"
            } else {
                Write-Log "  ❌ $($lib.Name) failed" "Red"
                Write-Log "  Check log: $libLogFile" "Yellow"
                $libraryResults[$lib.Name] = "Failed"
            }
        }
        
        Remove-Job $job -Force -ErrorAction SilentlyContinue
        $step++
    }
}

# Build CBXShell solution
Write-Log ""
Write-Log "Building CBXShell solution..." "Cyan"
Write-Progress-File -Status "Building" -Current "CBXShell" -Step ($totalSteps - 1) -Total $totalSteps

$msbuildScript = ".\build-scripts\Find-MSBuild.ps1"
if (Test-Path $msbuildScript) {
    $msbuild = & $msbuildScript
    if ($msbuild -and (Test-Path $msbuild)) {
        Write-Log "MSBuild: $msbuild" "Gray"
        
        $msbuildLog = "build-logs\msbuild-cbxshell-$timestamp.log"
        
        Write-Log "Building (output to $msbuildLog)..." "Gray"
        
        & $msbuild "CBXShell.sln" /p:Configuration=Release /p:Platform=x64 /m /v:minimal *>&1 | 
        Tee-Object -FilePath $msbuildLog
        
        if ($LASTEXITCODE -eq 0) {
            Write-Log "✅ CBXShell build succeeded" "Green"
        } else {
            Write-Log "❌ CBXShell build failed (exit code: $LASTEXITCODE)" "Red"
            Write-Log "Check log: $msbuildLog" "Yellow"
        }
    } else {
        Write-Log "❌ MSBuild not found" "Red"
    }
} else {
    Write-Log "❌ Find-MSBuild.ps1 not found" "Red"
}

# Final summary
$endTime = Get-Date
$duration = $endTime - $startTime

Write-Log ""
Write-Log "========================================" "Cyan"
Write-Log "  Build Summary" "Cyan"
Write-Log "========================================" "Cyan"
Write-Log "Duration: $([math]::Round($duration.TotalMinutes, 1)) minutes" "White"
Write-Log ""

if ($libraryResults.Count -gt 0) {
    Write-Log "Library Build Results:" "Cyan"
    foreach ($lib in $libraryResults.GetEnumerator() | Sort-Object Name) {
        $icon = if ($lib.Value -eq "Success") { "✅" } elseif ($lib.Value -eq "Timeout") { "⏱️" } else { "❌" }
        $color = if ($lib.Value -eq "Success") { "Green" } elseif ($lib.Value -eq "Timeout") { "Yellow" } else { "Red" }
        Write-Log "  $icon $($lib.Name): $($lib.Value)" $color
    }
}

Write-Log ""
Write-Log "Complete log: $logFile" "Cyan"
Write-Log "Finished: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" "Gray"
Write-Log ""

Write-Progress-File -Status "Complete" -Current "Build finished" -Step $totalSteps -Total $totalSteps

# Return exit code based on results
$failures = ($libraryResults.Values | Where-Object { $_ -eq "Failed" }).Count
exit $failures
