# PowerShell Script Naming Convention Analysis
**Date:** February 16, 2026  
**Issue:** Inconsistent naming conventions (PascalCase vs lowercase)

---

## 📋 Naming Convention Violations Found

### Standard: **PascalCase with Hyphens**
**Format:** `Build-LibraryName.ps1`, `Update-Component.ps1`, `Test-Feature.ps1`

**Examples:**
- ✅ `Build-LibWebP-NMake.ps1`
- ✅ `Build-Zlib.ps1`
- ✅ `Build-All-DarkThumbs-V7.ps1`
- ✅ `Monitor-Build-Logs.ps1`

### Files Requiring Renaming (14 total)

#### build-scripts/external-libs/ (5 files)

| Current Name | Correct Name | Reason |
|--------------|--------------|--------|
| `build-dav1d.ps1` | `Build-Dav1d.ps1` | Inconsistent with Build-*.ps1 pattern |
| `build-libavif.ps1` | `Build-LibAVIF.ps1` | Should match Build-LibRaw.ps1 style |
| `build-libjxl.ps1` | `Build-LibJXL.ps1` | Should match Build-LZ4.ps1 style |
| `build-lzma-sdk-26.00.ps1` | `Build-LZMA-SDK-26.00.ps1` | Inconsistent capitalization |
| `build-unrar.ps1` | `Build-UnRAR.ps1` | Should match Build-Zlib.ps1 style |

#### build-scripts/ (6 files)

| Current Name | Correct Name | Reason |
|--------------|--------------|--------|
| `build-cbxshell-quick.ps1` | `Build-CBXShell-Quick.ps1` | Core build scripts use PascalCase |
| `build-image-libs.ps1` | `Build-ImageLibs.ps1` | Should match Build-*.ps1 pattern |
| `download-updates.ps1` | `Download-Updates.ps1` | Should match Download-*.ps1 pattern |
| `msvc.cleanup.ps1` | `MSVC.Cleanup.ps1` or `MSVC-Cleanup.ps1` | Inconsistent with hyphen convention |
| `test-builds.ps1` | `Test-Builds.ps1` | Should match Test-*.ps1 pattern |
| `update-all-libraries.ps1` | `Update-All-Libraries.ps1` | Should match Update-*.ps1 pattern |

#### build-scripts/production/ (1 file)

| Current Name | Correct Name | Reason |
|--------------|--------------|--------|
| `rebuild-compression-libs.ps1` | `Rebuild-Compression-Libs.ps1` | Should match Rebuild-*.ps1 pattern |

#### build-scripts/validation/ (1 file)

| Current Name | Correct Name | Reason |
|--------------|--------------|--------|
| `check-tools.ps1` | `Check-Tools.ps1` | Should match Check-*.ps1 pattern |

#### build-scripts/utilities/ (1 file)

| Current Name | Correct Name | Reason |
|--------------|--------------|--------|
| `darkthumbs.ps1` | `DarkThumbs.ps1` | Project name should be capitalized |

---

## 🔍 Impact Analysis

### Scripts That May Reference These Files

Need to check for references in:
- ✅ `Build-All-DarkThumbs-V7.ps1` - may call external-libs scripts
- ✅ Task definitions in `.vscode/tasks.json`
- ✅ Documentation files (*.md)
- ✅ Other PowerShell scripts (*.ps1)

### Risk Assessment

| Risk Level | Description |
|------------|-------------|
| 🟢 **Low** | Most scripts use relative paths or are standalone |
| 🟡 **Medium** | Task automation may hard-code old names |
| 🔴 **High** | User muscle memory/documentation outdated |

---

## 🔧 Automated Renaming Plan

### Step 1: Create Backup
```powershell
$backupDate = Get-Date -Format "yyyy-MM-dd-HHmmss"
Copy-Item build-scripts -Destination "build-scripts-backup-$backupDate" -Recurse
```

### Step 2: Rename Files
```powershell
$renameMap = @{
    "build-scripts\external-libs\build-dav1d.ps1" = "Build-Dav1d.ps1"
    "build-scripts\external-libs\build-libavif.ps1" = "Build-LibAVIF.ps1"
    "build-scripts\external-libs\build-libjxl.ps1" = "Build-LibJXL.ps1"
    "build-scripts\external-libs\build-lzma-sdk-26.00.ps1" = "Build-LZMA-SDK-26.00.ps1"
    "build-scripts\external-libs\build-unrar.ps1" = "Build-UnRAR.ps1"
    "build-scripts\build-cbxshell-quick.ps1" = "Build-CBXShell-Quick.ps1"
    "build-scripts\build-image-libs.ps1" = "Build-ImageLibs.ps1"
    "build-scripts\download-updates.ps1" = "Download-Updates.ps1"
    "build-scripts\msvc.cleanup.ps1" = "MSVC-Cleanup.ps1"
    "build-scripts\test-builds.ps1" = "Test-Builds.ps1"
    "build-scripts\update-all-libraries.ps1" = "Update-All-Libraries.ps1"
    "build-scripts\production\rebuild-compression-libs.ps1" = "Rebuild-Compression-Libs.ps1"
    "build-scripts\validation\check-tools.ps1" = "Check-Tools.ps1"
    "build-scripts\utilities\darkthumbs.ps1" = "DarkThumbs.ps1"
}

foreach ($old in $renameMap.Keys) {
    $newName = $renameMap[$old]
    $dir = Split-Path $old
    Rename-Item $old -NewName $newName -ErrorAction Stop
    Write-Host "✅ Renamed: $old → $newName" -ForegroundColor Green
}
```

### Step 3: Update References
```powershell
# Search for old names in all scripts and docs
$oldNames = @(
    "build-dav1d", "build-libavif", "build-libjxl", "build-lzma-sdk",
    "build-unrar", "build-cbxshell-quick", "build-image-libs",
    "download-updates", "msvc.cleanup", "test-builds",
    "update-all-libraries", "rebuild-compression-libs",
    "check-tools", "darkthumbs.ps1"
)

Get-ChildItem -Recurse -Include *.ps1,*.md,*.json |
    Where-Object { -not $_.FullName.Contains("backup") } |
    ForEach-Object {
        $content = Get-Content $_.FullName -Raw
        $changed = $false
        
        foreach ($old in $oldNames) {
            if ($content -match [regex]::Escape($old)) {
                Write-Host "⚠️ Found reference in: $($_.FullName)" -ForegroundColor Yellow
                Write-Host "   Pattern: $old"
                $changed = $true
            }
        }
    }
```

### Step 4: Verify No Broken References
```powershell
# Test that all Build-*.ps1 scripts can be loaded
Get-ChildItem build-scripts -Recurse -Filter "Build-*.ps1" |
    ForEach-Object {
        try {
            $null = [System.Management.Automation.PSParser]::Tokenize(
                (Get-Content $_.FullName -Raw), [ref]$null
            )
            Write-Host "✅ Valid: $($_.Name)" -ForegroundColor Green
        } catch {
            Write-Host "❌ Syntax Error: $($_.Name)" -ForegroundColor Red
            Write-Host "   $($_.Exception.Message)"
        }
    }
```

---

## 📝 Why This Naming Convention?

### Historical Context
PowerShell community standard uses **PascalCase-With-Hyphens** (aka "kebab-case with capitals"):
- **Verb-Noun** pattern for cmdlets (Get-Process, Set-Item)
- **Multi-word** nouns use hyphens (Get-Child-Item, New-Web-Service-Proxy)
- **Approved verbs:** Get, Set, New, Add, Remove, etc.

### DarkThumbs Convention
Following PowerShell guidelines:
- `Build-LibraryName.ps1` - Build verb + library noun
- `Download-ComponentName.ps1` - Download verb + component noun
- `Test-FeatureName.ps1` - Test verb + feature noun

### Why lowercase is wrong
- ❌ Inconsistent with 75% of existing scripts
- ❌ Violates PowerShell community standards
- ❌ Harder to read (buildcbxshellquick vs Build-CBXShell-Quick)
- ❌ Poor discovery (autocomplete relies on PascalCase)

---

## ✅ Post-Rename Verification Checklist

- [ ] All 14 files renamed
- [ ] No syntax errors in renamed files
- [ ] All script references updated
- [ ] Documentation updated (*.md files)
- [ ] Task definitions updated (.vscode/tasks.json)
- [ ] Git commit with descriptive message
- [ ] Test one script from each category works

---

## 🎯 Recommendation

**Execute renaming immediately.** This is a low-risk, high-value change that improves:
- ✅ Code consistency (100% PascalCase)
- ✅ Developer experience (easier to find scripts)
- ✅ Professional appearance
- ✅ PowerShell best practices compliance

**Total Time:** ~30 minutes (rename + update references + verify)
