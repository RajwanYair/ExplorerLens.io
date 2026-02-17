# DarkThumbs Upgrade Testing Guide v7.0

## Overview

This guide provides comprehensive procedures for testing upgrades from previous versions of DarkThumbs to v7.0.0. Proper upgrade testing ensures users can smoothly transition without losing configuration or encountering compatibility issues.

---

## Supported Upgrade Paths

### Direct Upgrades (Recommended)
- **v6.2.0 → v7.0.0** ✅ Fully supported, tested
- **v6.1.0 → v7.0.0** ✅ Supported
- **v6.0.0 → v7.0.0** ✅ Supported

### Multi-Step Upgrades (Required)
- **v5.x → v6.2.0 → v7.0.0** ⚠️ Two-step upgrade required
- **v4.x → v6.2.0 → v7.0.0** ⚠️ Two-step upgrade required
- **v3.x and earlier → v6.2.0 → v7.0.0** ⚠️ Not recommended, consider fresh install

---

## Pre-Upgrade Checklist

### System State Capture
Before upgrading, capture the current system state:

```powershell
# Create upgrade test report
$TestReport = @{
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    CurrentVersion = (Get-Item "C:\Program Files\DarkThumbs\CBXShell.dll").VersionInfo.FileVersion
    User = $env:USERNAME
    MachineName = $env:COMPUTERNAME
    WindowsVersion = [System.Environment]::OSVersion.VersionString
}

# Check installed components
$TestReport.InstalledFiles = @(
    Get-Item "C:\Program Files\DarkThumbs\CBXShell.dll" -ErrorAction SilentlyContinue
    Get-Item "C:\Program Files\DarkThumbs\CBXManager.exe" -ErrorAction SilentlyContinue
)

# Check COM registration
$TestReport.COMRegistration = Test-Path "HKCR:\CLSID\{A8394D0D-EE2B-4A00-9FAC-AB8D3B03F078}"

# Check file associations
$TestReport.FileAssociations = @(
    (Get-Item "HKCR:\.cbz\ShellEx\{E357FCCD-A995-4576-B01F-234630154E96}" -ErrorAction SilentlyContinue)
)

# Save report
$TestReport | ConvertTo-Json | Out-File "upgrade-pre-test-v7.json"
```

### Backup Current Installation
```powershell
# Backup current DarkThumbs installation
$BackupDir = "C:\DarkThumbs-Backup-$(Get-Date -Format 'yyyyMMdd-HHmmss')"
New-Item -ItemType Directory -Path $BackupDir -Force

# Backup binaries
Copy-Item "C:\Program Files\DarkThumbs\*" -Destination $BackupDir -Recurse -ErrorAction SilentlyContinue

# Export registry settings
reg export "HKCR\CLSID\{A8394D0D-EE2B-4A00-9FAC-AB8D3B03F078}" "$BackupDir\COM-Registration.reg" /y

Write-Host "Backup created: $BackupDir" -ForegroundColor Green
```

### Test Environment Setup
Recommended test environments:

1. **Windows 10 22H2 VM**
   - Start with v6.2.0 installed
   - Standard user account (non-admin)
   - 100 GB disk space
   - Internet connection for vcpkg (if needed)

2. **Windows 11 24H2 VM**
   - Start with v6.2.0 installed
   - Administrator account
   - Sample files across all formats
   - GPU for accelerated testing

---

## Upgrade Test Scenarios

### Scenario 1: MSI Upgrade (In-Place)
**Objective:** Verify MSI installer properly upgrades existing installation

**Steps:**
1. Install DarkThumbs v6.2.0 using MSI
2. Verify it works (open folder with thumbnails)
3. Run v7.0.0 MSI installer
4. Follow prompts (should detect existing installation)
5. Complete upgrade
6. Verify functionality

**Expected Results:**
- [ ] Installer detects existing v6.2.0
- [ ] Offers "Upgrade" option (not "Repair" or "Remove")
- [ ] Preserves user settings
- [ ] No prompts to manually unregister old version
- [ ] COM registration updated automatically
- [ ] All thumbnails render correctly after upgrade
- [ ] CBXManager.exe shows v7.0.0 in About dialog
- [ ] No leftover v6.2.0 files in installation directory

**Verification:**
```powershell
# Check version
(Get-Item "C:\Program Files\DarkThumbs\CBXShell.dll").VersionInfo.FileVersion
# Expected: 7.0.0.0

# Check COM registration
$CLSID = "{A8394D0D-EE2B-4A00-9FAC-AB8D3B03F078}"
Test-Path "HKCR:\CLSID\$CLSID"
# Expected: True

# Check thumbnail provider registration
Get-Item "HKCR:\.cbz\ShellEx\{E357FCCD-A995-4576-B01F-234630154E96}"
# Expected: Value points to DarkThumbs CLSID
```

### Scenario 2: Inno Setup Upgrade
**Objective:** Test Inno Setup installer upgrade path

**Steps:**
1. Install v6.2.0 using Inno Setup (if available)
2. Run v7.0.0 Inno Setup installer
3. Observe upgrade process
4. Complete installation

**Expected Results:**
- [ ] Installer detects previous version
- [ ] Offers to uninstall old version first (Inno Setup typical behavior)
- [ ] Uninstall completes successfully
- [ ] New version installs cleanly
- [ ] COM re-registration successful
- [ ] Explorer restarts automatically
- [ ] Thumbnails work immediately

### Scenario 3: Manual Upgrade (Portable → MSI)
**Objective:** Upgrade from portable ZIP installation to MSI

**Steps:**
1. Install v6.2.0 from portable ZIP
2. Manually register DLL
3. Run v7.0.0 MSI installer
4. Installer should detect manually registered DLL

**Expected Results:**
- [ ] MSI installer warns about existing manual installation
- [ ] Option to unregister old version presented
- [ ] After unregistering, MSI proceeds normally
- [ ] No duplicate COM registrations

### Scenario 4: Side-by-Side (Not Supported)
**Objective:** Verify multiple versions cannot coexist

**Steps:**
1. Install v6.2.0 to `C:\Program Files\DarkThumbs`
2. Attempt to install v7.0.0 to `C:\Program Files\DarkThumbs-v7`

**Expected Results:**
- [ ] Installer warns that only one version allowed
- [ ] Must uninstall v6.2.0 first
- [ ] COM registration conflict prevented

### Scenario 5: Downgrade Prevention
**Objective:** Prevent accidental downgrade

**Steps:**
1. Install v7.0.0
2. Attempt to install v6.2.0 MSI

**Expected Results:**
- [ ] v6.2.0 installer detects newer version
- [ ] Blocks installation with error message
- [ ] No changes made to v7.0.0 installation

---

## Configuration Migration Testing

### Registry Settings Migration
Test that these settings carry over from v6.2.0:

```powershell
# GPU acceleration preference
$GPUEnabled = Get-ItemProperty -Path "HKCU:\Software\DarkThumbs" -Name "GPUAcceleration" -ErrorAction SilentlyContinue

# Cache settings
$CacheSize = Get-ItemProperty -Path "HKCU:\Software\DarkThumbs" -Name "CacheSize" -ErrorAction SilentlyContinue

# Format preferences
$PreferredFormats = Get-ItemProperty -Path "HKCU:\Software\DarkThumbs\Formats" -ErrorAction SilentlyContinue
```

**Expected:**
- [ ] GPU acceleration setting preserved
- [ ] Cache settings retained
- [ ] Format-specific preferences maintained
- [ ] Custom thumbnail sizes remembered

### File Association Preservation
```powershell
# Check .cbz association
(Get-Item "HKCR:\.cbz").GetValue("")
# Should remain "DarkThumbs.Archive" or similar

# Check thumbnail handler
Get-ItemProperty "HKCR:\.cbz\ShellEx\{E357FCCD-A995-4576-B01F-234630154E96}"
# Should point to v7.0.0 CLSID
```

---

## Regression Testing After Upgrade

### Functional Tests
After upgrade, verify all core functionality:

#### 1. Thumbnail Generation
```powershell
# Test folder with mixed formats
$TestFolder = "C:\Test-Archives"
explorer.exe $TestFolder

# Wait 5 seconds for thumbnails to generate
Start-Sleep -Seconds 5

# Manually verify:
# - CBZ thumbnails render
# - WebP thumbnails render
# - Mixed formats folder shows all thumbnails
# - No broken/generic icons
```

#### 2. Context Menu Integration
- [ ] Right-click .cbz file → Properties shows "DarkThumbs" tab
- [ ] Right-click .webp file → Properties shows format info
- [ ] "Generate Thumbnail" context menu item present (if feature exists)

#### 3. Performance
```powershell
# Benchmark thumbnail generation speed
Measure-Command {
    Get-ChildItem "$TestFolder\*.cbz" | ForEach-Object {
        [System.IO.File]::OpenRead($_.FullName).Close()
    }
}
# Expected: < 100ms per file
```

#### 4. GPU Acceleration (if applicable)
```powershell
# Launch CBXManager
& "C:\Program Files\DarkThumbs\CBXManager.exe"

# Check GPU status in UI
# Expected: "GPU Acceleration: Enabled" if compatible hardware present
```

### Compatibility Tests
Test with various file scenarios:

| Test Case | Description | Expected Result |
|-----------|-------------|-----------------|
| Large CBZ | Open folder with 500 MB+ CBZ | Thumbnails render within 2 seconds |
| Unicode names | Files with Chinese/Japanese/Arabic names | Correct rendering, no encoding issues |
| Network paths | Files on SMB share `\\server\share` | Thumbnails work over network |
| Compressed formats | 7z, RAR5, encrypted archives | All formats supported |

---

## Rollback Procedures

If upgrade fails or introduces issues:

### Method 1: MSI Rollback (Windows Installer)
```powershell
# Find installed product code
$ProductCode = (Get-WmiObject -Class Win32_Product | Where-Object { $_.Name -eq "DarkThumbs" }).IdentifyingNumber

# Uninstall v7.0.0
msiexec /x $ProductCode /qn

# Reinstall v6.2.0 from backup
msiexec /i "C:\Backups\DarkThumbs-Setup-6.2.0.msi" /qn
```

### Method 2: Manual Restore from Backup
```powershell
# Unregister v7.0.0
regsvr32 /u "C:\Program Files\DarkThumbs\CBXShell.dll"

# Stop Explorer
Stop-Process -Name explorer -Force

# Restore v6.2.0 files from backup
Copy-Item "C:\DarkThumbs-Backup-*\*" -Destination "C:\Program Files\DarkThumbs\" -Recurse -Force

# Re-register v6.2.0
regsvr32 "C:\Program Files\DarkThumbs\CBXShell.dll"

# Restart Explorer
Start-Process explorer.exe
```

### Method 3: System Restore (Nuclear Option)
```powershell
# Create restore point before upgrade
Checkpoint-Computer -Description "Before DarkThumbs v7.0 upgrade" -RestorePointType MODIFY_SETTINGS
```

Then if issues occur:
1. Open System Restore: `rstrui.exe`
2. Select restore point created before upgrade
3. Follow wizard to restore

---

## Known Upgrade Issues

### Issue 1: COM Registration Fails
**Symptom:** After upgrade, thumbnails don't appear  
**Cause:** Old COM registration not properly replaced  
**Fix:**
```powershell
# Manually re-register
regsvr32 /u "C:\Program Files\DarkThumbs\CBXShell.dll"
regsvr32 "C:\Program Files\DarkThumbs\CBXShell.dll"

# Clear thumbnail cache
Remove-Item "$env:LOCALAPPDATA\Microsoft\Windows\Explorer\thumbcache_*.db" -Force

# Restart Explorer
Stop-Process -Name explorer -Force; Start-Process explorer.exe
```

### Issue 2: Version Check Reports Wrong Version
**Symptom:** CBXManager shows v6.2.0 after installing v7.0.0  
**Cause:** Cached executable or shadow copy  
**Fix:**
```powershell
# Kill all DarkThumbs processes
Get-Process | Where-Object { $_.Path -like "*DarkThumbs*" } | Stop-Process -Force

# Clear file cache
Clear-RecycleBin -Force

# Reinstall v7.0.0 MSI
```

### Issue 3: Settings Lost After Upgrade
**Symptom:** GPU acceleration disabled, cache settings reset  
**Cause:** Registry path changed between versions  
**Fix:**
```powershell
# Export v6.2.0 settings (pre-upgrade)
reg export "HKCU\Software\DarkThumbs" "settings-v6.reg"

# After upgrade, import if needed
reg import "settings-v6.reg"
```

---

## Upgrade Testing Report Template

Use this template to document upgrade testing results:

```markdown
# DarkThumbs v7.0.0 Upgrade Test Report

**Tested By:** [Your Name]  
**Test Date:** [Date]  
**Environment:** [Windows 10 22H2 / Windows 11 24H2]  
**Prior Version:** [e.g., v6.2.0]  
**Upgrade Method:** [MSI / Inno Setup / Manual]

## Pre-Upgrade State
- DarkThumbs v6.2.0 installed: [ ] Yes [ ] No
- COM registration verified: [ ] Yes [ ] No
- Thumbnails working pre-upgrade: [ ] Yes [ ] No
- Configuration backed up: [ ] Yes [ ] No

## Upgrade Process
- Installer launched successfully: [ ] Yes [ ] No
- Upgrade detected existing installation: [ ] Yes [ ] No
- Installation completed without errors: [ ] Yes [ ] No
- Explorer restarted automatically: [ ] Yes [ ] No

## Post-Upgrade Verification
- CBXShell.dll version = 7.0.0.0: [ ] Yes [ ] No
- COM registration updated: [ ] Yes [ ] No
- Thumbnails render correctly: [ ] Yes [ ] No
- CBXManager launches: [ ] Yes [ ] No
- Settings preserved: [ ] Yes [ ] No
- Performance acceptable: [ ] Yes [ ] No

## Regression Tests
- Mixed format folder (100 files): [ ] Pass [ ] Fail
- Large archive (500+ MB): [ ] Pass [ ] Fail
- Unicode filenames: [ ] Pass [ ] Fail
- Network paths: [ ] Pass [ ] Fail
- GPU acceleration: [ ] Pass [ ] Fail [ ] N/A

## Issues Found
[List any bugs, errors, or unexpected behavior]

## Overall Result
[ ] ✅ PASS - Upgrade successful, all tests passed
[ ] ⚠️ PASS WITH MINOR ISSUES - Upgrade works, minor issues noted
[ ] ❌ FAIL - Critical issues prevent successful upgrade

## Notes
[Additional observations, recommendations, or context]
```

---

## Automation Script

For automated upgrade testing:

```powershell
# Automated Upgrade Test Script
param(
    [string]$OldVersion = "6.2.0",
    [string]$NewVersion = "7.0.0",
    [string]$TestVMName = "DarkThumbs-Test-VM"
)

# 1. Snapshot VM
Write-Host "Creating VM snapshot..." -ForegroundColor Cyan
Checkpoint-VM -Name $TestVMName -SnapshotName "Pre-Upgrade-$NewVersion"

# 2. Install old version
Write-Host "Installing v$OldVersion..." -ForegroundColor Yellow
# ... installation logic ...

# 3. Verify old version works
Write-Host "Testing v$OldVersion functionality..." -ForegroundColor Yellow
# ... run tests ...

# 4. Run upgrade
Write-Host "Upgrading to v$NewVersion..." -ForegroundColor Cyan
# ... upgrade logic ...

# 5. Verify new version
Write-Host "Testing v$NewVersion functionality..." -ForegroundColor Yellow
# ... run tests ...

# 6. Generate report
Write-Host "Generating test report..." -ForegroundColor Green
# ... report generation ...

# 7. Restore VM snapshot for next test
Write-Host "Restoring VM snapshot..." -ForegroundColor Cyan
Restore-VMSnapshot -Name "Pre-Upgrade-$NewVersion" -VMName $TestVMName -Confirm:$false
```

---

## Acceptance Criteria

Upgrade is considered **successful** if:

- ✅ Installer detects and handles existing installation
- ✅ All v7.0.0 binaries correctly replaced v6.2.0
- ✅ COM registration updated without manual intervention
- ✅ User settings and preferences preserved
- ✅ All file format thumbnails render correctly
- ✅ No error dialogs or crashes during upgrade
- ✅ Performance equal to or better than v6.2.0
- ✅ Rollback possible without data loss

---

**Document Version:** 1.0.0  
**Last Updated:** 2026-02-16  
**Maintained By:** DarkThumbs QA Team
