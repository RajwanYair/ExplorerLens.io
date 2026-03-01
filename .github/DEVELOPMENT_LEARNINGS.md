# ExplorerLens — Development Learnings & Best Practices

**Last Updated:** v15.0.0 "Zenith" — June 2026

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
- **libwebp FIX (2026-02):** Switched link path from `libwebp-1.5.0-original` (/MT) to
  `libwebp-1.5.0-build/build-cmake/Release` (/MD). Removed all 3 `/NODEFAULTLIB:LIBCMT`
  workarounds from CMakeLists.txt (root, Engine, Engine/Tests). Added `LIBWEBP_REBUILT_WITH_MD`
  compile definition. Verified via `dumpbin /directives`: `/DEFAULTLIB:MSVCRT` = correct.
- Validate with: `dumpbin /directives libwebp.lib | Select-String "DEFAULTLIB"`

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
- `.github/copilot-instructions.md` (version info)
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
 compression-libs/ — zlib, lz4, zstd, minizip-ng, lzma, unrar, bzip2, libarchive, xz
 image-libs/ — libwebp, libjxl, libavif, libheif, libde265, dav1d
 camera-libs/ — libraw, libraw-install
 pdf-libs/ — mupdf
 ui-libs/ — wtl
```

### Feature flags
- Use `HAS_LIBNAME` CMake options for optional libraries
- Decoders must gracefully degrade when library is not available
- Current flags: HAS_LIBJXL, HAS_LIBHEIF, HAS_LIBRAW, HAS_LIBAVIF, HAS_LIBARCHIVE, HAS_MUPDF

---

## 5. Feature Delivery

### Feature deliverable pattern
1. Create header/source file in appropriate `Engine/` subdirectory
2. Register in `Engine/CMakeLists.txt` (ENGINE_HEADERS and/or ENGINE_SOURCES)
3. Add `#include` in `Engine/Tests/EngineTests.cpp`
4. Add `TEST(name)` function and `RUN_TEST(name)` call
5. Register test file in `Engine/Tests/CMakeLists.txt` if separate
6. Git commit with descriptive message: `feat: Description — details`

### Batch delivery pattern (for bulk work)
1. Create all source files first
2. Register all in CMakeLists.txt in one multi-replace operation
3. Add all includes + TEST() + RUN_TEST() to EngineTests.cpp
4. Build verify: `cmake --build --preset default-release -j 8`
5. Git commit each feature individually (or batch 5 at a time)

### CMakeLists.txt insertion points
- Core headers: before `# Pipeline`
- Core sources: before `# Pipeline implementations`
- Utils headers: before `# `
- Utils sources: before closing `)`
- Check these haven't moved — `grep_search` for nearby markers before inserting

### Test insertion points in EngineTests.cpp
- New includes: after last feature include block
- TEST() functions: before `//== ` section
- RUN_TEST() calls: before `// Isolation & Stability Tests`

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
- One commit per logical change (fix, feature, or milestone)
- Batch related changes (e.g., "delete 6 obsolete files") into single commits
- Never mix bug fixes with unrelated features in one commit

### Working tree discipline
- Verify `git status --short` before starting new work
- Commit or stash outstanding changes before switching tasks
- Use `git add -f` for files in gitignored directories (like `Engine/Release/`)

---

## 9. Architecture Patterns (v15.0 "Zenith")

### Umbrella header pattern
- Consolidate related headers into one umbrella (e.g., `TestInfrastructure.h`, `WindowsCompat.h`)
- **Type conflict rule:** If two headers define the same enum/class, EXCLUDE one from the umbrella
  and document the conflict. Known conflicts:
  - `CodeCoverage.h` vs `TestFramework.h`/`CodeCoverageIntegration.h` — CoverageTool, CoverageMetric
  - `WindowsUI.h` vs `WindowsCompat.h` — DPIScale enum (different member names)
  - V1 PluginMarketplace.h vs MSIXPackageManager.h — PackageType/CertificateInfo

### Interface consolidation pattern
- Core interfaces (`I*.h`) live in `Engine/Core/Interfaces/` (canonical location)
- Original files in `Engine/Core/` are thin forwarding headers: `#include "Interfaces/X.h"`
- Both canonical and forwarding headers registered in CMakeLists.txt

### Dead code tracking
- `Engine/Core/DeadCodeAudit.h` tracks all findings with type/severity/status
- Audit report: `docs/DEAD_CODE_AUDIT.md`
- 16.7% of engine headers (63/377) have zero consumers
- 39% of headers (147/377) are test-only — no production caller

### Build time tracking
- `Build-MSVC.ps1` captures per-phase timing (vcvars, configure, build, test)
- Appends JSONL entries to `build-logs/build-history.jsonl`
- Typical times: vcvars ~7s, configure ~2s, incremental build <1s, full LTCG ~270s

### Singleton test pattern
- Many engine classes are singletons — use `ClassName::Instance()`, NOT direct construction
- `IntegrationTestFramework`, `DeadCodeAudit`, `LibWebPConfig` all use this pattern
- Some classes expose free functions in namespace — `ExplorerLens::GetWindowsBuildNumber()`, not
  `Win11CompatibilityLayer::GetWindowsBuildNumber()`

### Header registration audit
- New headers MUST be registered in `Engine/CMakeLists.txt` or subdirectory CMakeLists.txt
- Dead code audit (2026-02) found 31 unregistered headers — all now registered
- Run periodic audit: search for `.h` files not in any CMakeLists.txt
---

## 10. Code Quality Polish (v15.0 Post-Release)

### Sprint comment removal pattern
- Batch/sprint comments (`// Sprint NNN — ExplorerLens v15.0.0 Zenith`) accumulate during delivery
- Remove with PowerShell regex: `$content -replace '(?m)^// Sprint \d+[^\r\n]*\r?\n', ''`
- Applied across 38 header files in one pass, verified with `Select-String` afterward
- Keep meaningful file-level doc blocks; remove only tracking/versioning artifacts

### Header documentation standard
- Every header should have a structured doc block immediately after `#pragma once`:
```cpp
#pragma once
// ============================================================================
// FileName.h — Brief one-line description
//
// Purpose:   What this module provides and why it exists
// Provides:  Key classes, enums, constants, free functions
// Used by:   Consumer modules, pipelines, test harness
// ============================================================================
```
- Prefer block-level comments over inline comments
- Document inputs/outputs at function level for non-trivial public methods
- Remove dead comments: "merged into X.h", "moved to Y.h", "Batch N" markers

### EngineTests.cpp include organization
- Group includes by functional category with `// --- Category Name ---` section headers
- 20 categories identified: Pipeline, Rendering, Core Architecture, Memory, Cache, etc.
- Remove per-include comments (`// EngineTests.h — description`) — they add clutter
- Keep the section headers concise: `// --- GPU & Rendering ---` not paragraph descriptions

### Stub replacement methodology
1. Search for stubs: `return 0`, `return path`, `placeholder`, `itemsTotal = 100`
2. Read exact file content first (whitespace matters for `replace_string_in_file`)
3. Prefer Windows native APIs: `FindFirstFileW`/`FindNextFileW` for directory scans,
   `std::chrono::system_clock::now()` for timestamps, `std::regex_replace` for string transforms
4. Only add `#include` headers genuinely needed by the implementation
5. Leave stubs for external library calls (djvulibre, MuPDF) as graceful degradation

### Name collision patterns in large header sets
- When creating 50+ headers in batch, name collisions are common:
  - `PrefetchStrategy` enum used by multiple prefetch headers → prefix with module name
  - `StreamProtocol` struct in both ThumbnailStreamProtocol.h and existing pipeline code
  - `SandboxPolicy` class vs existing DecoderSandbox policy enums
  - `MEM_RESET` Windows SDK macro conflicts with custom memory reset enums
  - ZSTD compression library macros conflict with Zstandard-related class names
- Prevention: search for the symbol name across the codebase before declaring it
- Fix pattern: use fully-qualified namespace names or add unique prefixes

### Global regex rename dangers
- Bulk rename operations (e.g., renaming a class across all files) can corrupt unrelated code
- Pre-existing test functions named similarly to the target can be accidentally modified
- Always use precise patterns: `\bOldName\b` word-boundary regex, not substring match
- Verify diff covers only intended files before committing

### C4100 unused parameter warning pattern
- MSVC /W4 warns on unused function parameters
- Standard fix: `(void)paramName;` at function start
- Do NOT rename parameters to `/*paramName*/` — reduces readability
- Do NOT remove parameter names from declarations — breaks documentation

---

## 11. GUI Format Coverage (LENSManager)

### GUI architecture
- WTL/ATL-based dialog in `LENSManager/MainDlg.h` + `MainDlg.cpp`
- 35 format checkboxes (IDC_CB_*) defined in `resource.h` (IDs 1004–1068)
- Format-to-registry mapping in `OnApplyImpl()` via `formatHandlers[]` array
- Each checkbox toggles shell extension registration via `CRegManager::SetHandlers()`

### Format type ID spaces are separate
- `LENSTYPE_*` (in `LENSShell/LENSTypes.h`) = engine-internal format routing IDs
- `LENS_*` (in `LENSManager/RegManager.h`) = GUI/registry toggle IDs
- These numeric values intentionally differ (e.g., LENS_WEBP=15, LENSTYPE_WEBP=40)
- The `CRegManager` maps between them using per-format TH/IH registry key macros
- Comment "must match LENSTYPE in LENSArchive.h" only applies where values align (TIFF=45, SVG=46, etc.)

### Group vs individual toggles
- Group toggles (VIDEO, AUDIO, RAW, DOCUMENT, FONT, MODEL) control multiple file extensions each
- Sub-types (TAR_GZ, TAR_BZ2, CPIO, ISO, DEB, DOCX/PPTX/XLSX, TTF/OTF) inherit from parent group
- Format-specific sub-types don't need individual checkboxes — the parent controls them
- Windows-native formats (BMP, GIF) are not toggled because Windows handles them natively

### FormatGroupHelper categorization
- 12 categories in `FormatGroupHelper.h` with `FormatCategory` enum
- `FormatStatusProvider.h` provides traffic-light status (ACTIVE/DEGRADED/UNAVAILABLE/UNKNOWN)
- Status colors: Green=Active (34,139,34), Yellow=Degraded (218,165,32), Red=Unavailable (178,34,34)
- `DecoderHealthCheck::CheckAll()` populates live status on dialog init

### Adding a new format to the GUI
1. Define `LENS_NEWFORMAT <id>` in `RegManager.h` (unique, non-colliding)
2. Define `LENS_NEWFORMATTH_KEY` and `LENS_NEWFORMATIH_KEY` registry key macros
3. Add `IDC_CB_NEWFORMAT <id>` in `resource.h` (next available ID)
4. Add checkbox control in dialog resource (.rc file)
5. Add `COMMAND_HANDLER(IDC_CB_NEWFORMAT, BN_CLICKED, OnCheckboxClicked)` in MainDlg.h message map
6. Add `Button_SetCheck(GetDlgItem(IDC_CB_NEWFORMAT), m_reg.HasTH(LENS_NEWFORMAT))` in `InitUI()`
7. Add `{IDC_CB_NEWFORMAT, LENS_NEWFORMAT}` to `formatHandlers[]` in `OnApplyImpl()`
8. Add to `allCheckboxes[]` array in `OnInitDialog()`, `OnSelectAll()`, `OnDeselectAll()`
9. Add tooltip in `InitTooltips()` and status icon in `InitStatusIcons()`
10. Add to `FormatGroupHelper.h` `FormatEntry` table under appropriate category