# CRITICAL: Missing unzip.cpp File

**Date:** January 7, 2026  
**Severity:** HIGH  
**Status:** ACTION REQUIRED

---

## Issue

The file `CBXShell/unzip.cpp` was accidentally deleted during consolidation cleanup but is still **actively used** in the project.

## Impact

- ❌ Project will not compile
- ❌ CBXShell.dll cannot be built
- ❌ ZIP archive thumbnail functionality broken

## Root Cause

During cleanup to remove duplicate files, `CBXShell/unzip.cpp` (4339 lines) was mistakenly identified as "old code" because:

- A newer replacement exists: `CBXShell/unzip_new.cpp` (382 lines, minizip-ng based)
- The replacement is more modern and efficient

**However**, examination of `CBXShell/CBXShell.vcxproj` shows:

```xml
<!-- Legacy ZIP support using built-in unzip.cpp (to be replaced with minizip-ng) -->
<ClCompile Include="unzip.cpp" />

<!-- Modern ZIP using minizip-ng - disabled until minizip-ng library is built -->
<!-- <ClCompile Include="unzip_new.cpp" /> -->
```

The old `unzip.cpp` is **still active**, and `unzip_new.cpp` is commented out pending minizip-ng library completion.

---

## Required Actions

### Option 1: Restore unzip.cpp (RECOMMENDED for immediate fix)

```powershell
# If you have a backup or can recover the file:
# 1. Locate unzip.cpp from backup
# 2. Copy to CBXShell/unzip.cpp
# 3. Verify project compiles
```

### Option 2: Switch to unzip_new.cpp (Better long-term, requires work)

```xml
<!-- In CBXShell/CBXShell.vcxproj, change: -->
<!-- <ClCompile Include="unzip.cpp" /> -->
<ClCompile Include="unzip_new.cpp" />
```

**Prerequisites:**

1. Build minizip-ng library (see ROADMAP.md Phase 1)
2. Link minizip-ng in project
3. Test ZIP functionality thoroughly
4. Update dependencies

---

## File Information

### unzip.cpp (MISSING - NEEDED NOW)

- **Size:** 4339 lines
- **Description:** 1998 zlib-based unzip implementation
- **Status:** Currently active in .vcxproj
- **Purpose:** ZIP archive extraction for CBZ/ZIP thumbnails

### unzip_new.cpp (EXISTS - FUTURE REPLACEMENT)

- **Size:** 382 lines
- **Description:** Modern minizip-ng 4.0.7 wrapper
- **Status:** Ready but commented out
- **Purpose:** Modern, secure replacement
- **Dependency:** Requires minizip-ng library built first

---

## Prevention

This issue occurred because:

1. Both files exist with "_new" suffix indicating migration
2. Cleanup script assumed newer was in use
3. Project file was not checked before deletion

### Updated Cleanup Process

New guideline added to `.github/PROJECT_ORGANIZATION.md`:

> **Before removing *_new or *_old files:**
>
> 1. Check .vcxproj to see which is active
> 2. Check for comments explaining migration status
> 3. If migration is in progress, keep both files
> 4. Document in README if keeping temporary duplicates

---

## Temporary Workaround

If unzip.cpp cannot be immediately restored:

1. Comment out ZIP-related code in CBXShell
2. Disable CBZ/ZIP thumbnail functionality temporarily
3. Update README with known limitation
4. Prioritize restoring/migrating ZIP support

---

## Status Tracking

- [ ] unzip.cpp restored from backup
- [ ] Project compiles successfully
- [ ] ZIP thumbnails tested and working
- [ ] OR: Migration to unzip_new.cpp completed
- [ ] OR: ZIP functionality temporarily disabled (documented)

---

**Priority:** Complete before next build attempt  
**Assigned:** Developer  
**Due:** ASAP
