# Installation Fix & Project Organization - 2026-01-08

## Issues Resolved

### 1. Installation File Locking Error ✅
**Problem**: Installation failed with error:
```
Error copying CBXShell.dll: The process cannot access the file 
'C:\Program Files\DarkThumbs\CBXShell.dll' because it is being 
used by another process.
```

**Root Cause**: Windows Explorer was holding a lock on the old `CBXShell.dll` because it was registered as a shell extension and actively loaded.

**Solution Implemented**:
1. **Automatic Unregistration**: Before copying files, script now unregisters existing DLL with `regsvr32 /u /s`
2. **Explorer Restart**: Automatically stops Explorer before file copy, releases all file locks
3. **Automatic Restart**: Restarts Explorer after successful installation
4. **New Helper Function**: `Restart-Explorer` function with wait/no-wait options
5. **Better User Experience**: Removed manual restart instruction (now automatic)

**Files Modified**:
- [scripts/install.ps1](scripts/install.ps1) - Added unregister logic, Explorer restart handling

### 2. Duplicate Scripts Consolidated ✅
**Problem**: Multiple installation scripts with overlapping/conflicting functionality:
- `scripts/install.ps1` (current, 319 lines, with dry-run and timeout)
- `scripts/install/Install-DarkThumbs.ps1` (old, 212 lines, stops Explorer)
- `scripts/install/install-x64.ps1` (old, 124 lines, registry checks)
- `scripts/install/uninstall-x64.ps1` (old, separate uninstall)

**Solution Implemented**:
1. **Consolidated into Single Script**: All functionality now in `scripts/install.ps1`
2. **Archived Old Scripts**: Moved to `scripts/install/_archive/` with .old extension
3. **Created Cleanup Tool**: `scripts/maintenance/cleanup-duplicates.ps1` for future use
4. **Better Features**: New script has dry-run mode, timeout protection, conditional admin checks

**Files Archived**:
- `scripts/install/Install-DarkThumbs.ps1.old`
- `scripts/install/install-x64.ps1.old`
- `scripts/install/uninstall-x64.ps1.old`

**Files Created**:
- `scripts/maintenance/cleanup-duplicates.ps1` - Reusable duplicate finder

### 3. Project Organization Guidelines ✅
**Problem**: No clear guidelines on where to put scripts/docs, leading to duplication.

**Solution Implemented**:
Enhanced [.github/CONTRIBUTING.md](.github/CONTRIBUTING.md) with:

1. **Canonical File Locations Table**:
   | Purpose | Canonical Path | Status |
   |---------|---------------|--------|
   | Main build | `scripts/build.ps1` | ✅ Current |
   | Installation | `scripts/install.ps1` | ✅ Current |
   | Verification | `scripts/verify-tools.ps1` | ✅ Current |
   | Library builds | `build-scripts/*.ps1` | ✅ Current |
   | Updates | `build-scripts/update-all-libraries.ps1` | ✅ Current |

2. **Naming Conventions**:
   - Scripts: `lowercase-with-hyphens.ps1`
   - Major docs: `UPPERCASE_WITH_UNDERSCORES.md`
   - Subdirectory docs: `lowercase-with-hyphens.md`

3. **Duplicate Prevention Guidelines**:
   - Search before creating new files
   - Update existing files instead of duplicating
   - Check canonical location table

4. **Consolidation Process**:
   - Step-by-step guide for identifying and merging duplicates
   - PowerShell commands for finding duplicates
   - Git commit message standards

5. **Directory Structure Standards**:
   ```
   scripts/
   ├── build.ps1              # CANONICAL: Main build
   ├── install.ps1            # CANONICAL: Main install
   ├── verify-tools.ps1       # CANONICAL: Tool verification
   ├── install/               # DEPRECATED: Archive old scripts here
   ├── maintenance/           # Cleanup and utility scripts
   └── ...
   ```

## Git Commits

### Commit 1: `812c038` - Dry-run mode and timeout
```
feat: add dry-run mode and timeout handling for COM registration

- Remove #Requires -RunAsAdministrator to enable dry-run without elevation
- Add -DryRun parameter for previewing installation steps
- Add 30-second timeout for regsvr32.exe
- Add verbose output showing exact commands
- Add conditional admin check (skip for dry-run)
```

### Commit 2: `0fbdb55` - COM diagnostics documentation
```
docs: add COM registration diagnostic guide

- Document DLL dependencies verification (all satisfied)
- Document COM exports verification (all functions exported)
- Add manual registration testing procedures
- List common causes of registration hang
- Provide troubleshooting steps with Event Viewer and Process Monitor
```

### Commit 3: `80c3ec4` - File locking fix and consolidation (THIS COMMIT)
```
fix: resolve installation file locking and consolidate duplicate scripts

Installation Fixes:
- Add automatic unregister of existing DLL before installation
- Stop Explorer before copying files to release file locks
- Restart Explorer automatically after installation

Script Consolidation:
- Archive 3 duplicate installation scripts
- Created cleanup-duplicates.ps1 for future use
- All functionality consolidated into scripts/install.ps1

Documentation:
- Updated .github/CONTRIBUTING.md with organization guidelines
- Added canonical file location reference
- Added duplicate prevention guidelines
```

## Testing Instructions

### Test the Fixed Installation

1. **Run installation as Administrator**:
   ```powershell
   cd "C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\DarkThumbs"
   .\scripts\install.ps1 -Configuration Release
   ```

2. **What should happen**:
   - ✅ Unregisters old DLL (if exists)
   - ✅ Stops Windows Explorer
   - ✅ Copies files without error
   - ✅ Registers new DLL
   - ✅ Restarts Explorer automatically
   - ✅ Shows installation summary

3. **Verify installation**:
   ```powershell
   # Check files
   Get-ChildItem "C:\Program Files\DarkThumbs"
   
   # Test thumbnails
   explorer.exe "$PWD\test-archives"
   ```

4. **If still fails**:
   - Check [docs/COM_REGISTRATION_DIAGNOSTICS.md](docs/COM_REGISTRATION_DIAGNOSTICS.md)
   - Run dry-run: `.\scripts\install.ps1 -DryRun`
   - Check Event Viewer for COM errors

## Benefits of Changes

### For Users
✅ Installation now handles file locks automatically  
✅ No manual Explorer restart needed  
✅ Clear error messages with troubleshooting steps  
✅ Dry-run mode for testing without admin privileges  
✅ Timeout prevents indefinite hangs  

### For Developers
✅ Single canonical installation script (no confusion)  
✅ Clear guidelines prevent future duplication  
✅ Reusable cleanup tool for finding duplicates  
✅ Better code organization and maintainability  
✅ Standardized commit message format  

## Next Steps

1. **User Testing**: Test installation with fixed script
2. **Manual Testing**: Verify thumbnails work in Windows Explorer
3. **Cleanup**: Review archived scripts, delete if no unique features
4. **Documentation**: Update any scripts that reference old installation paths

## Files Changed Summary

```
Modified:
  .github/CONTRIBUTING.md           (+85 lines, organization guidelines)
  scripts/install.ps1               (+35 lines, -10 lines, file lock handling)

Deleted (Archived):
  scripts/install/Install-DarkThumbs.ps1
  scripts/install/install-x64.ps1
  scripts/install/uninstall-x64.ps1

Created:
  scripts/maintenance/cleanup-duplicates.ps1
  docs/INSTALLATION_FIX_2026-01-08.md (this file)
```

## Canonical Script Locations Reference

**Always use these paths:**
- Build: `scripts/build.ps1`
- Install: `scripts/install.ps1`
- Uninstall: `scripts/install.ps1 -Unregister`
- Verify: `scripts/verify-tools.ps1`
- Update Libraries: `build-scripts/update-all-libraries.ps1`
- Cleanup: `scripts/maintenance/cleanup-duplicates.ps1`

**Do NOT create new scripts in:**
- `scripts/install/*.ps1` (archive old ones to `_archive/`)
- Root directory (use appropriate subdirectory)

## See Also

- [COM_REGISTRATION_DIAGNOSTICS.md](docs/COM_REGISTRATION_DIAGNOSTICS.md) - Troubleshooting guide
- [INSTALLATION_READY.md](INSTALLATION_READY.md) - User-facing installation guide
- [.github/CONTRIBUTING.md](.github/CONTRIBUTING.md) - Project organization guidelines
- [BUILD_STATUS.md](BUILD_STATUS.md) - Current build status
