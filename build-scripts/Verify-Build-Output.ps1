# Verify-Build-Output.ps1
# Check build outputs without interfering with build process
# Uses file system queries only - safe for slow machines

param(
    [Parameter(Mandatory = $false)]
    [switch]$Detailed
)

$ErrorActionPreference = "Continue"

Write-Host "`n🔍 ExplorerLens Build Output Verification" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════`n" -ForegroundColor Cyan

# Define expected outputs
$outputs = @(
    @{
        Name    = "ExplorerLens Engine Library"
        Paths   = @(
            "Engine\Release\ExplorerLensEngine.lib",
            "Engine\x64\Release\ExplorerLensEngine.lib",
            "Engine\build\Release\ExplorerLensEngine.lib"
        )
        Type    = "Static Library"
        MinSize = 1MB
    },
    @{
        Name    = "LENSShell Extension DLL"
        Paths   = @(
            "LENSShell\x64\Release\LENSShell.dll",
            "x64\Release\LENSShell.dll"
        )
        Type    = "Dynamic Library"
        MinSize = 1MB
    },
    @{
        Name    = "LENSManager Application"
        Paths   = @(
            "LENSManager\x64\Release\LENSManager.exe",
            "x64\Release\LENSManager.exe"
        )
        Type    = "Executable"
        MinSize = 100KB
    }
)

$found = 0
$total = $outputs.Count
$issues = @()

foreach ($output in $outputs) {
    Write-Host "📦 $($output.Name)" -ForegroundColor White
    
    $file = $null
    foreach ($path in $output.Paths) {
        if (Test-Path $path) {
            $file = Get-Item $path
            break
        }
    }
    
    if ($file) {
        $age = (Get-Date) - $file.LastWriteTime
        $sizeStr = "{0:N2} MB" -f ($file.Length / 1MB)
        
        Write-Host "   ✓ Found: $($file.FullName)" -ForegroundColor Green
        Write-Host "   Size: $sizeStr" -ForegroundColor Gray
        Write-Host "   Modified: $($file.LastWriteTime.ToString('yyyy-MM-dd HH:mm:ss'))" -ForegroundColor Gray
        Write-Host "   Age: $([math]::Round($age.TotalMinutes, 1)) minutes ago" -ForegroundColor Gray
        
        # Validate size
        if ($file.Length -lt $output.MinSize) {
            $warning = "⚠ File is smaller than expected ($sizeStr < $($output.MinSize/1MB) MB)"
            Write-Host "   $warning" -ForegroundColor Yellow
            $issues += $warning
        }
        
        # Warn if file is old
        if ($age.TotalMinutes -gt 60) {
            $warning = "⚠ File is old ($([math]::Round($age.TotalHours, 1)) hours) - may need rebuild"
            Write-Host "   $warning" -ForegroundColor Yellow
            $issues += $warning
        }
        
        $found++
        
        # Detailed info
        if ($Detailed) {
            Write-Host "   Type: $($output.Type)" -ForegroundColor DarkGray
            Write-Host "   Attributes: $($file.Attributes)" -ForegroundColor DarkGray
        }
    } else {
        Write-Host "   ✗ NOT FOUND" -ForegroundColor Red
        Write-Host "   Searched paths:" -ForegroundColor DarkGray
        foreach ($path in $output.Paths) {
            Write-Host "     - $path" -ForegroundColor DarkGray
        }
        $issues += "$($output.Name) is missing"
    }
    
    Write-Host ""
}

# Summary
Write-Host "═══════════════════════════════════════" -ForegroundColor Cyan
Write-Host "Summary: $found/$total outputs found" -ForegroundColor $(if ($found -eq $total) { "Green" } else { "Yellow" })

if ($issues.Count -gt 0) {
    Write-Host "`nIssues:" -ForegroundColor Yellow
    foreach ($issue in $issues) {
        Write-Host "  • $issue" -ForegroundColor Yellow
    }
}

# Check for recent build logs
Write-Host "`n📋 Recent Build Logs:" -ForegroundColor Cyan
$logs = Get-ChildItem -Path . -Filter "*.log" -Recurse -ErrorAction SilentlyContinue | 
Where-Object { $_.LastWriteTime -gt (Get-Date).AddHours(-2) } |
Sort-Object LastWriteTime -Descending |
Select-Object -First 5

if ($logs) {
    foreach ($log in $logs) {
        $age = (Get-Date) - $log.LastWriteTime
        Write-Host "  • $($log.Name) - $([math]::Round($age.TotalMinutes)) min ago" -ForegroundColor Gray
        Write-Host "    $($log.FullName)" -ForegroundColor DarkGray
    }
} else {
    Write-Host "  No recent build logs found" -ForegroundColor DarkGray
}

# Exit code
if ($found -eq $total -and $issues.Count -eq 0) {
    Write-Host "`n✓ All builds verified successfully!" -ForegroundColor Green
    exit 0
} elseif ($found -eq $total) {
    Write-Host "`n⚠ All files found but with warnings" -ForegroundColor Yellow
    exit 1
} else {
    Write-Host "`n✗ Build verification failed" -ForegroundColor Red
    exit 2
}

