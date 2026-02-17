#Requires -Version 7.0
# DarkThumbs v7.0 - Build Progress Tracking Module
# Provides real-time build progress tracking with elapsed time, step indicators, and estimates
# Date: February 16, 2026

<#
.SYNOPSIS
    Progress tracking module for DarkThumbs build system

.DESCRIPTION
    Provides functions to track build progress with:
    - Elapsed time tracking
    - Step progress indicators  
    - Estimated completion time
    - Build phase statistics
    - Real-time status updates

.EXAMPLE
    $progress = Start-BuildProgress -TotalSteps 10 -BuildName "External Libraries"
    Update-BuildProgress -Progress $progress -CurrentStep 1 -StepName "Building zlib"
    Complete-BuildProgress -Progress $progress

.NOTES
    Author: DarkThumbs Development Team
    Version: 7.0
#>

# Progress tracking class
class BuildProgress {
    [string]$BuildName
    [int]$TotalSteps
    [int]$CurrentStep
    [datetime]$StartTime
    [hashtable]$StepTimes
    [System.Diagnostics.Stopwatch]$Stopwatch
    
    BuildProgress([string]$name, [int]$steps) {
        $this.BuildName = $name
        $this.TotalSteps = $steps
        $this.CurrentStep = 0
        $this.StartTime = Get-Date
        $this.StepTimes = @{}
        $this.Stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    }
    
    [double] GetElapsedSeconds() {
        return $this.Stopwatch.Elapsed.TotalSeconds
    }
    
    [string] GetElapsedTime() {
        $elapsed = $this.Stopwatch.Elapsed
        return "{0:D2}:{1:D2}:{2:D2}" -f $elapsed.Hours, $elapsed.Minutes, $elapsed.Seconds
    }
    
    [double] GetAverageStepTime() {
        if ($this.CurrentStep -eq 0) { return 0 }
        return $this.GetElapsedSeconds() / $this.CurrentStep
    }
    
    [string] GetEstimatedTimeRemaining() {
        if ($this.CurrentStep -eq 0) { return "Calculating..." }
        
        $avgTime = $this.GetAverageStepTime()
        $remainingSteps = $this.TotalSteps - $this.CurrentStep
        $estimatedSeconds = $avgTime * $remainingSteps
        
        $span = [TimeSpan]::FromSeconds($estimatedSeconds)
        return "{0:D2}:{1:D2}:{2:D2}" -f $span.Hours, $span.Minutes, $span.Seconds
    }
    
    [int] GetPercentComplete() {
        if ($this.TotalSteps -eq 0) { return 0 }
        return [math]::Round(($this.CurrentStep / $this.TotalSteps) * 100)
    }
}

<#
.SYNOPSIS
    Starts a new build progress tracker

.PARAMETER TotalSteps
    Total number of build steps

.PARAMETER BuildName
    Name of the build operation
#>
function Start-BuildProgress {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [int]$TotalSteps,
        
        [Parameter(Mandatory)]
        [string]$BuildName
    )
    
    $progress = [BuildProgress]::new($BuildName, $TotalSteps)
    
    Write-Host "`n" -NoNewline
    Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
    Write-Host " $BuildName - Build Progress Tracker" -ForegroundColor Cyan
    Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
    Write-Host (" Total Steps: {0}" -f $TotalSteps) -ForegroundColor Gray
    Write-Host (" Started: {0:yyyy-MM-dd HH:mm:ss}" -f $progress.StartTime) -ForegroundColor Gray
    Write-Host "───────────────────────────────────────────────────────`n" -ForegroundColor DarkGray
    
    return $progress
}

<#
.SYNOPSIS
    Updates build progress with current step information

.PARAMETER Progress
    BuildProgress object from Start-BuildProgress

.PARAMETER CurrentStep
    Current step number (1-based)

.PARAMETER StepName
    Name/description of current step

.PARAMETER Status
    Optional status message for the step
#>
function Update-BuildProgress {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [BuildProgress]$Progress,
        
        [Parameter(Mandatory)]
        [int]$CurrentStep,
        
        [Parameter(Mandatory)]
        [string]$StepName,
        
        [Parameter()]
        [string]$Status = ""
    )
    
    $stepStart = $Progress.Stopwatch.Elapsed.TotalSeconds
    $Progress.CurrentStep = $CurrentStep
    $Progress.StepTimes[$CurrentStep] = @{
        Name = $StepName
        StartTime = $stepStart
    }
    
    # Progress bar
    $barWidth = 50
    $filledWidth = [math]::Floor(($CurrentStep / $Progress.TotalSteps) * $barWidth)
    $emptyWidth = $barWidth - $filledWidth
    $progressBar = ("[" + ("█" * $filledWidth) + ("░" * $emptyWidth) + "]")
    
    # Statistics
    $elapsed = $Progress.GetElapsedTime()
    $remaining = $Progress.GetEstimatedTimeRemaining()
    $percent = $Progress.GetPercentComplete()
    
    Write-Host (" [{0}/{1}] {2}" -f $CurrentStep, $Progress.TotalSteps, $StepName) -ForegroundColor Yellow
    Write-Host (" $progressBar {0}%" -f $percent) -ForegroundColor Cyan
    Write-Host (" Elapsed: $elapsed │ Estimated Remaining: $remaining") -ForegroundColor Gray
    
    if ($Status) {
        Write-Host (" Status: $Status") -ForegroundColor DarkCyan
    }
    
    Write-Host ""
}

<#
.SYNOPSIS
    Marks a build step as complete

.PARAMETER Progress
    BuildProgress object from Start-BuildProgress

.PARAMETER StepName
    Name of completed step

.PARAMETER Success
    Whether the step succeeded
#>
function Complete-BuildStep {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [BuildProgress]$Progress,
        
        [Parameter(Mandatory)]
        [string]$StepName,
        
        [Parameter()]
        [bool]$Success = $true
    )
    
    $stepEnd = $Progress.Stopwatch.Elapsed.TotalSeconds
    $stepData = $Progress.StepTimes[$Progress.CurrentStep]
    
    if ($stepData) {
        $stepDuration = $stepEnd - $stepData.StartTime
        $stepData.EndTime = $stepEnd
        $stepData.Duration = $stepDuration
        $stepData.Success = $Success
        
        $icon = if ($Success) { "✓" } else { "✗" }
        $color = if ($Success) { "Green" } else { "Red" }
        
        Write-Host (" $icon $StepName completed in {0:F1}s" -f $stepDuration) -ForegroundColor $color
        Write-Host ""
    }
}

<#
.SYNOPSIS
    Completes build progress tracking and shows summary

.PARAMETER Progress
    BuildProgress object from Start-BuildProgress

.PARAMETER Success
    Whether the overall build succeeded
#>
function Complete-BuildProgress {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [BuildProgress]$Progress,
        
        [Parameter()]
        [bool]$Success = $true
    )
    
    $Progress.Stopwatch.Stop()
    $totalTime = $Progress.GetElapsedTime()
    $endTime = Get-Date
    
    Write-Host "───────────────────────────────────────────────────────" -ForegroundColor DarkGray
    Write-Host ""
    
    $icon = if ($Success) { "✓" } else { "✗" }
    $color = if ($Success) { "Green" } else { "Red" }
    $status = if ($Success) { "COMPLETED SUCCESSFULLY" } else { "FAILED" }
    
    Write-Host " $icon BUILD $status" -ForegroundColor $color
    Write-Host ""
    Write-Host (" Build Name: {0}" -f $Progress.BuildName) -ForegroundColor Gray
    Write-Host (" Total Time: $totalTime") -ForegroundColor Gray
    Write-Host (" Started: {0:HH:mm:ss}" -f $Progress.StartTime) -ForegroundColor Gray
    Write-Host (" Finished: {0:HH:mm:ss}" -f $endTime) -ForegroundColor Gray
    Write-Host (" Steps Completed: {0}/{1}" -f $Progress.CurrentStep, $Progress.TotalSteps) -ForegroundColor Gray
    
    # Show step timing breakdown
    if ($Progress.StepTimes.Count -gt 0) {
        Write-Host "`n Step Timing Breakdown:" -ForegroundColor Cyan
        
        $sortedSteps = $Progress.StepTimes.GetEnumerator() | 
            Where-Object { $_.Value.Duration -ne $null } |
            Sort-Object { $_.Value.Duration } -Descending
        
        foreach ($step in $sortedSteps) {
            $stepNum = $step.Key
            $stepData = $step.Value
            $duration = $stepData.Duration
            $success = if ($stepData.Success) { "✓" } else { "✗" }
            $stepColor = if ($stepData.Success) { "Green" } else { "Red" }
            
            Write-Host ("   {0} [{1}] {2} - {3:F1}s" -f $success, $stepNum, $stepData.Name, $duration) -ForegroundColor $stepColor
        }
    }
    
    Write-Host "`n═══════════════════════════════════════════════════════`n" -ForegroundColor Cyan
}

<#
.SYNOPSIS
    Shows a simple progress spinner for long-running operations

.PARAMETER Message
    Message to display with spinner

.PARAMETER ScriptBlock
    Code to execute while showing spinner
#>
function Show-BuildSpinner {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [string]$Message,
        
        [Parameter(Mandatory)]
        [scriptblock]$ScriptBlock
    )
    
    $spinnerChars = @('⠋', '⠙', '⠹', '⠸', '⠼', '⠴', '⠦', '⠧', '⠇', '⠏')
    $spinnerIndex = 0
    
    $job = Start-Job -ScriptBlock $ScriptBlock
    
    try {
        while ($job.State -eq 'Running') {
            Write-Host ("`r {0} {1}..." -f $spinnerChars[$spinnerIndex], $Message) -NoNewline -ForegroundColor Cyan
            $spinnerIndex = ($spinnerIndex + 1) % $spinnerChars.Length
            Start-Sleep -Milliseconds 100
        }
        
        $result = Receive-Job -Job $job -Wait
        Write-Host "`r ✓ $Message - Complete" -ForegroundColor Green
        
        return $result
    }
    catch {
        Write-Host "`r ✗ $Message - Failed" -ForegroundColor Red
        throw
    }
    finally {
        Remove-Job -Job $job -Force
    }
}

# Functions are automatically available when dot-sourced
# No export needed for script files
