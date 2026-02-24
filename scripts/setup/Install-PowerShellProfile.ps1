# Install-PowerShellProfile.ps1
# Installs the ExplorerLens PowerShell profile for automatic environment setup

[CmdletBinding()]
param(
    [ValidateSet('CurrentUser', 'AllUsers')]
    [string]$Scope = 'CurrentUser',
    
    [switch]$Backup,
    [switch]$Force
)

$ErrorActionPreference = 'Stop'

Write-Host ""
Write-Host "+===============================================================+" -ForegroundColor Cyan
Write-Host "|         ExplorerLens PowerShell Profile Installer               |" -ForegroundColor Cyan
Write-Host "+===============================================================+" -ForegroundColor Cyan
Write-Host ""

# Determine profile path based on scope
$profilePath = if ($Scope -eq 'AllUsers') {
    # Requires admin privileges
    if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        Write-Error "AllUsers scope requires Administrator privileges. Run PowerShell as Administrator or use -Scope CurrentUser"
        exit 1
    }
    $PROFILE.AllUsersAllHosts
} else {
    $PROFILE.CurrentUserAllHosts
}

Write-Host "Target Profile: " -NoNewline
Write-Host $profilePath -ForegroundColor Yellow
Write-Host ""

# Backup existing profile
if ((Test-Path $profilePath) -and $Backup) {
    $backupPath = "$profilePath.backup-$(Get-Date -Format 'yyyyMMdd-HHmmss')"
    Write-Host "[...] Creating backup: $backupPath" -ForegroundColor Yellow
    Copy-Item $profilePath $backupPath -Force
    Write-Host "[OK] Backup created" -ForegroundColor Green
    Write-Host ""
}

# Check if profile exists
if ((Test-Path $profilePath) -and -not $Force) {
    Write-Host "[WARN]  Profile already exists!" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Options:" -ForegroundColor Cyan
    Write-Host "  1. Append ExplorerLens configuration (recommended)"
    Write-Host "  2. Replace entire profile"
    Write-Host "  3. Cancel"
    Write-Host ""
    $choice = Read-Host "Select option (1-3)"
    
    switch ($choice) {
        "1" {
            # Append mode
            $templatePath = Join-Path $PSScriptRoot "PowerShell-Profile-Template.ps1"
            if (-not (Test-Path $templatePath)) {
                Write-Error "Template not found at: $templatePath"
                exit 1
            }
            
            Write-Host "[...] Appending ExplorerLens configuration..." -ForegroundColor Yellow
            
            # Check if already contains ExplorerLens config
            $existingContent = Get-Content $profilePath -Raw
            if ($existingContent -like "*ExplorerLens Development Environment*") {
                Write-Warning "Profile already contains ExplorerLens configuration!"
                $overwrite = Read-Host "Overwrite existing ExplorerLens section? (y/n)"
                if ($overwrite -ne 'y') {
                    Write-Host "[FAIL] Installation cancelled" -ForegroundColor Red
                    exit 0
                }
                
                # Remove old ExplorerLens section
                $lines = Get-Content $profilePath
                $inExplorerLensSection = $false
                $newLines = @()
                
                foreach ($line in $lines) {
                    if ($line -like "*# PowerShell Profile Configuration for ExplorerLens Development*") {
                        $inExplorerLensSection = $true
                    }
                    
                    if (-not $inExplorerLensSection) {
                        $newLines += $line
                    }
                    
                    if ($inExplorerLensSection -and $line.Trim() -eq "" -and $newLines.Count -gt 0) {
                        $inExplorerLensSection = $false
                    }
                }
                
                Set-Content $profilePath -Value $newLines -Force
            }
            
            # Append template
            $templateContent = Get-Content $templatePath -Raw
            Add-Content $profilePath "`n`n# ===============================================================`n# ExplorerLens Development Configuration (Auto-Generated)`n# ===============================================================`n"
            Add-Content $profilePath $templateContent
            
            Write-Host "[OK] ExplorerLens configuration appended to profile" -ForegroundColor Green
        }
        "2" {
            # Replace mode
            $templatePath = Join-Path $PSScriptRoot "PowerShell-Profile-Template.ps1"
            if (-not (Test-Path $templatePath)) {
                Write-Error "Template not found at: $templatePath"
                exit 1
            }
            
            Write-Host "[...] Replacing profile..." -ForegroundColor Yellow
            Copy-Item $templatePath $profilePath -Force
            Write-Host "[OK] Profile replaced" -ForegroundColor Green
        }
        "3" {
            Write-Host "[FAIL] Installation cancelled" -ForegroundColor Red
            exit 0
        }
        default {
            Write-Error "Invalid choice"
            exit 1
        }
    }
} else {
    # New installation
    $templatePath = Join-Path $PSScriptRoot "PowerShell-Profile-Template.ps1"
    if (-not (Test-Path $templatePath)) {
        Write-Error "Template not found at: $templatePath"
        exit 1
    }
    
    # Create profile directory if needed
    $profileDir = Split-Path $profilePath
    if (-not (Test-Path $profileDir)) {
        Write-Host "[...] Creating profile directory: $profileDir" -ForegroundColor Yellow
        New-Item -ItemType Directory -Path $profileDir -Force | Out-Null
        Write-Host "[OK] Directory created" -ForegroundColor Green
    }
    
    Write-Host "[...] Installing profile..." -ForegroundColor Yellow
    Copy-Item $templatePath $profilePath -Force
    Write-Host "[OK] Profile installed" -ForegroundColor Green
}

Write-Host ""
Write-Host "+===============================================================+" -ForegroundColor Green
Write-Host "|         Installation Complete!                                |" -ForegroundColor Green
Write-Host "+===============================================================+" -ForegroundColor Green
Write-Host ""

Write-Host "Profile Location: " -NoNewline
Write-Host $profilePath -ForegroundColor Yellow
Write-Host ""

Write-Host "Next Steps:" -ForegroundColor Cyan
Write-Host "  1. Open a new PowerShell window (or run: " -NoNewline
Write-Host ". `$PROFILE" -NoNewline -ForegroundColor Yellow
Write-Host ")"
Write-Host "  2. Run: " -NoNewline
Write-Host "Set-ExplorerLensEnvironment -InitializeVS" -ForegroundColor Yellow
Write-Host "  3. Run: " -NoNewline
Write-Host "Test-ExplorerLensTools" -ForegroundColor Yellow
Write-Host ""

Write-Host "Available Commands:" -ForegroundColor Cyan
Write-Host "  " -NoNewline
Write-Host "Set-ExplorerLensEnvironment" -NoNewline -ForegroundColor Yellow
Write-Host " - Initialize development environment"
Write-Host "  " -NoNewline
Write-Host "Initialize-VSBuildTools" -NoNewline -ForegroundColor Yellow
Write-Host "    - Load MSVC environment"
Write-Host "  " -NoNewline
Write-Host "Test-ExplorerLensTools" -NoNewline -ForegroundColor Yellow
Write-Host "       - Verify all tools"
Write-Host "  " -NoNewline
Write-Host "Build-ExplorerLens" -NoNewline -ForegroundColor Yellow
Write-Host "          - Build project"
Write-Host "  " -NoNewline
Write-Host "dt" -NoNewline -ForegroundColor Yellow
Write-Host "                        - Navigate to ExplorerLens"
Write-Host "  " -NoNewline
Write-Host "dt-build" -NoNewline -ForegroundColor Yellow
Write-Host ", " -NoNewline
Write-Host "dt-scripts" -NoNewline -ForegroundColor Yellow
Write-Host ", " -NoNewline
Write-Host "dt-docs" -NoNewline -ForegroundColor Yellow
Write-Host "  - Quick navigation"
Write-Host ""

# Test the profile
Write-Host "Testing profile loading..." -ForegroundColor Yellow
try {
    . $profilePath
    Write-Host "[OK] Profile loaded successfully!" -ForegroundColor Green
} catch {
    Write-Warning "[WARN]  Error loading profile: $_"
    Write-Host "Please check the profile syntax: $profilePath" -ForegroundColor Yellow
}

Write-Host ""

