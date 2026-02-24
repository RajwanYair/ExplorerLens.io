# ExplorerLens — Development Learnings & Best Practices

**Last Updated:** v15.0.0 "Zenith" — July 2025

This file captures hard-won lessons from iterative development sessions to avoid repeating
mistakes and to accelerate future work.

---

## 1. Build System

### MSVC is mandatory — never use Clang for production builds
- CMake picks Clang from PATH if `vcvars64.bat` isn't sourced first
- Always use `Build-MSVC.ps1` or source vcvars64 before running cmake
- The v145 toolset (`cl.exe 19.50.35720`) is the only validated compiler

### CRT linkage must be /MD everywhere
- All external libraries MUST be built with `/MD` (MultiThreadedDLL)
- libwebp was historically built with `/MT` — requires `/NODEFAULTLIB:LIBCMT` workaround
- Proper fix: rebuild with `-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL`

### Static libraries over DLLs
- Prefer static linking (`*.lib`) over dynamic (`*.dll`) for external deps
- Reduces deployment complexity (single DLL + EXE is the goal)
- libarchive, zlib, lz4, zstd, minizip-ng, lzma are all statically linked

### Build script pattern
- All library build scripts import `build-scripts/core/Build-Library-Core.ps1`
- Use `$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)` for root
- Never duplicate utility functions — extend Build-Library-Core.ps1 instead
- Downloaded archives go to a `downloads/` area, NOT into the `external/` lib directory

### CMake registration is required for new files
- New headers → `ENGINE_HEADERS` in `Engine/CMakeLists.txt`
- New sources → `ENGINE_SOURCES` in `Engine/CMakeLists.txt`
- New tests → `Engine/Tests/CMakeLists.txt` EngineTests target

---

## 2. Code Quality

### Zero warnings policy
- Build must produce 0 errors AND 0 warnings
- Use `/W4` warning level, treat warnings as errors where possible
- Run `clang-format` before committing — use `.clang-format` config in root

### clang-format changes whitespace
- After clang-format runs, file indentation and spacing changes
- When replacing code in formatted files, always `read_file` first to get exact whitespace
- Parameters may change from `const std::string& filePath` to `const std::string & /*filePath*/`
- Class member indentation may shift to 2-space

### WIN32_LEAN_AND_MEAN constraints
- Globally defined — many Windows SDK headers are excluded
- NEVER include `<versionhelpers.h>` — use `RtlGetVersion()` instead
- Always verify new SDK includes compile under WIN32_LEAN_AND_MEAN

### Stub replacement methodology
1. Identify stub by searching for "Stub:", "TODO:", or hard-coded fake return values
2. Read the actual file content (exact whitespace) before attempting replacement
3. Implement using Windows native APIs where possible (BCrypt, FindFirstFileExW, etc.)
4. Keep the same function signature — only change the body
5. Test stubs that need external libraries (djvulibre, MuPDF) are acceptable to leave as graceful degradation

---

## 3. Version Management

### Canonical version source
- `Engine/Engine.h` contains the `EXPLORERLENS_ENGINE_VERSION_*` macros
- `CMakeLists.txt` root `project(ExplorerLensEngine VERSION x.y.z)` must match
- All documentation should reference these rather than hard-coding versions

### Version update checklist
When bumping versions, update ALL of these:
- `Engine/Engine.h` (macros)
- `CMakeLists.txt` (project version)
- `vcpkg.json` (version field)
- `README.md` (badge + status section)
- `CHANGELOG.md` (new version entry)
- `packaging/ExplorerLens.wxs` (version registry value)
- `docs/INDEX.md`, `docs/PERFORMANCE.md`, `tests/README.md`
- `.github/copilot-instructions.md` (version + sprint count)
- `LENSShellClass.cpp` (version string)

### CLSID management
- COM CLSID: `{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}` — NEVER change
- MSI UpgradeCode: `{A8394D0D-EE2B-4A00-9FAC-AB8D3B03F078}` — separate, NEVER change
- These are different GUIDs for different purposes (COM registration vs MSI upgrade detection)
- The WiX CLSID mismatch bug caused DLL registration failure — always verify WiX matches DLL

---

## 4. Library Integration Pattern

### Adding a new external library
1. Create build script in `build-scripts/external-libs/Build-LibName.ps1`
2. Import `Build-Library-Core.ps1` for shared utilities
3. Build with `/MD` CRT, Release configuration, static library output
4. Install to `external/<category>-libs/libname-install/` (include + lib dirs)
5. Add CMake find logic in `Engine/CMakeLists.txt` with `HAS_LIBNAME` feature flag
6. Add to `LIBRARY_INVENTORY.md` and `CHANGELOG.md`
7. Download archives go to `downloads/` — NOT into `external/<lib>/`

### External library directory structure
```
external/
  compression-libs/   — zlib, lz4, zstd, minizip-ng, lzma, unrar, bzip2, libarchive, xz
  image-libs/         — libwebp, libjxl, libavif, libheif, libde265, dav1d
  camera-libs/        — libraw, libraw-install
  pdf-libs/           — mupdf
  ui-libs/            — wtl
```

### Feature flags
- Use `HAS_LIBNAME` CMake options for optional libraries
- Decoders must gracefully degrade when library is not available
- Current flags: HAS_LIBJXL, HAS_LIBHEIF, HAS_LIBRAW, HAS_LIBAVIF, HAS_LIBARCHIVE, HAS_MUPDF

---

## 5. Sprint Execution

### Sprint deliverable pattern
1. Create header/source file in appropriate `Engine/` subdirectory
2. Register in `Engine/CMakeLists.txt` (ENGINE_HEADERS and/or ENGINE_SOURCES)
3. Add `#include` in `Engine/Tests/EngineTests.cpp`
4. Add `TEST(name)` function and `RUN_TEST(name)` call
5. Register test file in `Engine/Tests/CMakeLists.txt` if separate
6. Git commit with descriptive message: `Sprint N: Description — details`

### Batch sprint pattern (for bulk work)
1. Create all source files first
2. Register all in CMakeLists.txt in one multi-replace operation
3. Add all includes + TEST() + RUN_TEST() to EngineTests.cpp
4. Build verify: `cmake --build --preset default-release -j 8`
5. Git commit each sprint individually (or batch 5 at a time)

### CMakeLists.txt insertion points
- Core headers: before `# Pipeline`
- Core sources: before `# Pipeline implementations`
- Utils headers: before `# Sprint 8-12:`
- Utils sources: before closing `)`
- Check these haven't moved — `grep_search` for nearby markers before inserting

### Test insertion points in EngineTests.cpp
- New includes: after last sprint include block
- TEST() functions: before `//== Sprint 6:` section
- RUN_TEST() calls: before `// Sprint 6: Isolation & Stability Tests`

---

## 6. Documentation

### Consolidation principles
- One authoritative file per topic — avoid partial duplicates
- Delete obsolete files, don't leave them with "deprecated" headers
- Archive historical plans to `docs/archive/` rather than deleting
- Cross-reference with relative links that work from any doc

### File naming conventions
- Standards/guides: `CODING_STANDARDS.md`, `BUILD_QUICK_REFERENCE.md`
- Troubleshooting: `BUILD_TROUBLESHOOTING.md`, `TROUBLESHOOTING.md`
- Planning: `ENHANCEMENT_PLAN_V{N}.md` (archive when version superseded)
- No redundant prefixes: `docs/development/DEVELOPER_GUIDE.md` not `docs/development/DEV_DEVELOPER_GUIDE.md`

### Dead link prevention
- After deleting/renaming a doc, search for all references across the workspace
- `grep_search` for the old filename before committing deletions
- Update `docs/INDEX.md` to reflect current file inventory

---

## 7. Common Pitfalls

| Pitfall | Impact | Prevention |
|---------|--------|------------|
| Forgetting vcvars64 before cmake | Clang selected, wrong ABI | Always use Build-MSVC.ps1 |
| /MT vs /MD CRT mismatch | Linker errors or runtime crashes | Rebuild libs with /MD |
| CLSID mismatch between WiX and DLL | COM registration fails silently | Verify GUIDs match |
| Editing formatted files with wrong whitespace | replace_string_in_file fails | Read file first |
| Adding headers without CMakeLists registration | File not compiled, linker errors | Check both HEADERS and SOURCES |
| Version number drift across docs | User confusion, broken expectations | Use version update checklist |
| Including versionhelpers.h | Build failure under WIN32_LEAN_AND_MEAN | Use RtlGetVersion() |
| Placing downloads in external/ lib dir | Clutter, git tracking issues | Use downloads/ area |

---

## 8. Git Workflow

### Commit conventions
- `feat:` — New features or real implementations replacing stubs
- `fix:` — Bug fixes, CLSID corrections, linker fixes
- `docs:` — Documentation-only changes
- `refactor:` — Code restructuring without behavior change
- `style:` — Formatting (clang-format)
- `build:` — Build system changes (CMake, scripts)

### Commit granularity
- One commit per logical change (fix, feature, or sprint)
- Batch related changes (e.g., "delete 6 obsolete files") into single commits
- Never mix bug fixes with unrelated features in one commit

### Working tree discipline
- Verify `git status --short` before starting new work
- Commit or stash outstanding changes before switching tasks
- Use `git add -f` for files in gitignored directories (like `Engine/Release/`)
