# Project Consolidation & Cleanup Guide

**Last Updated:** January 7, 2026  
**Purpose:** Maintain a clean, organized project structure

---

## 🎯 Cleanup Principles

1. **No Duplicate Documentation** - One authoritative source per topic
2. **No Archive Directories** - Use git history, not folder archives
3. **No Old/Backup Files** - Use version control, not file suffixes
4. **Consolidated Scripts** - One script with options, not multiple versions
5. **Essential Root Only** - Keep root directory minimal

---

## 📁 Standard Directory Structure

```
DarkThumbs/
├── .github/               # GitHub configuration ONLY
│   ├── workflows/        # CI/CD pipelines
│   ├── ISSUE_TEMPLATE/   # Issue templates
│   ├── CONTRIBUTING.md   # Contribution guidelines
│   ├── SECURITY.md       # Security policy
│   └── PULL_REQUEST_TEMPLATE.md
├── build-scripts/         # Build automation ONLY
├── CBXShell/             # Main shell extension source
├── CBXManager/           # Configuration GUI source
├── docs/                 # ALL documentation (centralized)
│   ├── getting-started/  # User guides
│   ├── development/      # Dev documentation
│   ├── api/             # API documentation
│   └── release-notes/   # Release notes by version
├── external/             # Third-party libraries (source only)
├── tests/               # Test files and suites
├── tools/               # Development tools
├── .gitattributes       # Git configuration
├── .gitignore          # Git ignore rules
├── CBXShell.sln        # Visual Studio solution
├── LICENSE             # License file
├── README.md           # Project overview
├── ROADMAP.md          # Development roadmap
└── RUN-BUILD.bat       # Quick build script
```

---

## ❌ What NOT to Keep

### Forbidden Directories

- ❌ `documentation/` - Duplicate of `docs/`
- ❌ `docs/archive/` - Use git history
- ❌ `release-packages/` - Binary releases only (no docs)
- ❌ Any directory named `archive`, `old`, `backup`, `deprecated`

### Forbidden Files

- ❌ `*_old.*`, `*.old` - Use version control
- ❌ `*_backup.*`, `*.backup`, `*.bak` - Use version control
- ❌ `*_deprecated.*` - Remove or update
- ❌ `*_new.*` unless actively migrating (document in README if so)
- ❌ Duplicate README files in subdirectories (except for truly independent components)

### Forbidden Patterns

- ❌ Multiple versions of same script (e.g., `build.ps1`, `build-v2.ps1`, `build-new.ps1`)
  - ✅ Instead: One script with parameters/options
- ❌ Multiple quick start guides (e.g., `QUICK_START.md`, `QUICK_START_UPDATED.md`)
  - ✅ Instead: Keep docs/QUICK_START.md updated
- ❌ Multiple project status files
  - ✅ Instead: Keep docs/PROJECT_STATUS.md current

---

## 🧹 Regular Cleanup Tasks

### Monthly Cleanup Checklist

```powershell
# Run this monthly cleanup script
cd DarkThumbs

# 1. Remove archive directories
Remove-Item -Path "documentation","release-packages","docs/archive" -Recurse -Force -ErrorAction SilentlyContinue

# 2. Find and remove old/backup files
Get-ChildItem -Recurse -File | Where-Object {
    $_.Name -match '_old\.|_backup\.|_deprecated\.|\.old$|\.backup$|\.bak$'
} | Remove-Item -Force

# 3. Find duplicate documentation
Get-ChildItem -Path docs -Filter "*.md" -Recurse | Group-Object Name | 
    Where-Object Count -gt 1 | Select-Object Name, Count

# 4. List README files (should be minimal)
Get-ChildItem -Recurse -Filter "README.md" | 
    Where-Object { $_.FullName -notmatch 'external|tools' }
```

### Before Committing

- [ ] No `*_old.*` or `*.backup` files
- [ ] No `archive/` directories
- [ ] No duplicate documentation
- [ ] Root directory has ≤10 files
- [ ] All build outputs cleaned (`build/`, `x64/`, etc.)

---

## 📝 Documentation Consolidation Rules

### Single Source of Truth

| Topic | Location | No Duplicates In |
|-------|----------|------------------|
| Build Instructions | `docs/BUILD_GUIDE.md` | ❌ documentation/, tests/, root |
| Quick Start | `docs/QUICK_START.md` | ❌ tests/, documentation/ |
| Project Status | `docs/PROJECT_STATUS.md` | ❌ docs/development/, root |
| Installation | `docs/INSTALLATION_GUIDE.md` | ❌ documentation/, root |
| Project Structure | `docs/PROJECT_STRUCTURE.md` | ❌ root, documentation/ |

### When to Create Subdirectory READMEs

**Only** create `README.md` in subdirectories if:

1. External library documentation (e.g., `external/libwebp/README.md`)
2. Independent component with different ownership (e.g., `SDK/examples/SamplePlugin/README.md`)
3. Tests with special setup instructions (e.g., `tests/README.md`)

**Never** duplicate information from root `README.md` or `docs/`.

---

## 🔄 Script Consolidation

### Bad: Multiple Script Versions

```
❌ build-scripts/
   ├── Build-Production.ps1
   ├── Build-Production-v2.ps1
   ├── Build-Production-New.ps1
   ├── Build-Production-Fixed.ps1
   └── Build-Production-SlowMachine.ps1
```

### Good: One Script with Options

```
✅ build-scripts/
   └── Build-Production.ps1 -Mode [Fast|Slow|Clean] -Verbose
```

### Implementation Pattern

```powershell
# Build-Production.ps1
param(
    [ValidateSet('Fast', 'Slow', 'Clean')]
    [string]$Mode = 'Fast',
    
    [switch]$Verbose
)

switch ($Mode) {
    'Fast'  { # Fast build logic }
    'Slow'  { # Slow machine optimizations }
    'Clean' { # Clean build logic }
}
```

---

## 🚫 .gitignore Enforcement

The `.gitignore` file now prevents committing:

- Build artifacts (`build/`, `x64/`, `build-logs/`)
- Archive directories (`release-packages/`, `documentation/`, `docs/archive/`)
- Backup files (`*_old.*`, `*.backup`, `*.bak`)

### Verify Clean Status

```bash
git status --ignored
```

Should show minimal ignored files (only build artifacts and external binaries).

---

## 🔍 Audit Commands

### Find Duplicate Files

```powershell
# Find files with same name in different directories
Get-ChildItem -Path docs -Recurse -File | 
    Group-Object Name | 
    Where-Object Count -gt 1 |
    ForEach-Object {
        Write-Host "`n$($_.Name) - $($_.Count) copies:" -ForegroundColor Yellow
        $_.Group | Select-Object FullName
    }
```

### Find Old/Backup Files

```powershell
Get-ChildItem -Recurse -File | 
    Where-Object { $_.Name -match '_old|_backup|_deprecated|\.old$|\.backup$' } |
    Select-Object FullName
```

### Find Large Documentation Directories

```powershell
Get-ChildItem -Directory -Recurse | 
    Where-Object { (Get-ChildItem $_ -Filter "*.md").Count -gt 10 } |
    Select-Object FullName, @{N='MdFiles';E={(Get-ChildItem $_ -Filter "*.md").Count}}
```

---

## ✅ Cleanup Completed (Jan 7, 2026)

### Removed

- ✅ `release-packages/` (56 duplicate markdown files)
- ✅ `documentation/` (complete duplicate of `docs/`)
- ✅ `docs/archive/` (outdated archived docs)
- ✅ `docs/development/README-old.md`
- ✅ `docs/development/COMPLETE_PROJECT_STATUS.md`
- ✅ `docs/QUICK_START_SLOW_MACHINES.md`
- ✅ `tests/QUICK_START.md`
- ✅ `build-scripts/PowerShell_Profile_Backup.ps1`

### Issues Found

- ⚠️ `CBXShell/unzip.cpp` - Currently active (4339 lines)
- ⚠️ `CBXShell/unzip_new.cpp` - Replacement ready (382 lines, minizip-ng based)
- 📝 Migration pending - requires minizip-ng library built first

### Statistics

- **Before:** 164+ markdown files across multiple directories
- **After:** ~60 markdown files in centralized `docs/`
- **Removed:** ~100 duplicate/archived documentation files
- **Cleaned:** 3 major duplicate directory trees

---

## 🎯 Ongoing Maintenance

### Weekly

- Remove any `*_old.*` or backup files
- Check for new archive directories

### Monthly  

- Run full cleanup audit
- Consolidate any new duplicate docs
- Review and prune `docs/development/` session logs

### Before Release

- Full cleanup verification
- Ensure no test files in release
- Verify documentation is current

---

## 📞 Questions?

If you're unsure whether to keep a file:

1. Is it in version control? → Delete local backup
2. Is it documented elsewhere? → Delete duplicate
3. Is it older than 3 months and unused? → Delete
4. Still unsure? → Ask in PR review

**Remember:** Git is our backup and history system. We don't need file-based archives.

---

**Maintained by:** DarkThumbs Project Team  
**Next Review:** February 7, 2026
