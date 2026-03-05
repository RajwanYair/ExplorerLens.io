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

```text
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
- Utils headers: before `#`
- Utils sources: before closing `)`
- Check these haven't moved — `grep_search` for nearby markers before inserting

### Test insertion points in EngineTests.cpp
- New includes: after last feature include block
- TEST() functions: before `//==` section
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
| --------- | -------- | ------------ |
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
- Every header starts with a Copyright doc-block BEFORE `#pragma once`:

```cpp
// FileName.h — Brief Title
// Copyright (c) 2026 ExplorerLens Project
//
// Detailed description of what this header provides, what the main
// classes/functions do, and how they fit in the architecture.
//
#pragma once
```

- Do NOT use `=====` decorator lines or version numbers in banners
- Prefer block-level comments over inline comments
- Document inputs/outputs at function level for non-trivial public methods
- Remove dead comments: "merged into X.h", "moved to Y.h", "Batch N" markers
- Remove redundant single-line comments that just restate the type name
  (e.g., `// Configuration` above a `struct Config` is noise)

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

---

## 12. Dynamic DLL Loading (WIN32_LEAN_AND_MEAN Environments)

### When to use dynamic loading
- Any API from a header excluded by `WIN32_LEAN_AND_MEAN` (e.g., `wintrust.h`, WIC headers)
- Optional codec libraries that may not be installed (e.g., `openjp2.dll`)
- Vendor-specific GPU decode SDKs (NVDEC, AMF, QuickSync)

### Pattern: LoadLibrary + GetProcAddress

```cpp
// 1. Define function pointer types matching the target API
typedef LONG (WINAPI* PFN_WinVerifyTrust)(HWND, GUID*, LPVOID);

// 2. Load library dynamically (use W variant)
HMODULE hLib = LoadLibraryW(L"wintrust.dll");
if (!hLib) return false;

// 3. Resolve function pointers
auto pfn = reinterpret_cast<PFN_WinVerifyTrust>(GetProcAddress(hLib, "WinVerifyTrust"));

// 4. Define inline struct replicas if original header is unavailable
// Mirror the Windows SDK struct layout exactly — field names/types must match
struct WINTRUST_FILE_INFO { DWORD cbStruct; LPCWSTR pcwszFilePath; ... };

// 5. Call and cleanup
LONG result = pfn(nullptr, &actionGUID, &trustData);
FreeLibrary(hLib);
```

### Caching pattern for repeated probes

```cpp
static bool IsAvailable() {
    static int cached = -1;  // -1=unchecked, 0=no, 1=yes
    if (cached >= 0) return cached == 1;
    HMODULE h = LoadLibraryW(L"openjp2.dll");
    cached = h ? 1 : 0;
    if (h) FreeLibrary(h);
    return cached == 1;
}
```

### WinHTTP for HTTP clients
- Include `<winhttp.h>` and link `winhttp.lib` (safe under WIN32_LEAN_AND_MEAN)
- Pattern: `WinHttpOpen` → `WinHttpConnect` → `WinHttpOpenRequest` → `WinHttpSendRequest` → `WinHttpReceiveResponse` → `WinHttpQueryDataAvailable` → `WinHttpReadData`
- Always close handles in reverse order with `WinHttpCloseHandle`
- Set timeout via `WinHttpSetTimeouts(hSession, 10000, 10000, 10000, 30000)`

---

## 13. Binary Format Parsing Patterns

### General approach
- Read first 64KB of file via `CreateFileW` + `ReadFile` into stack buffer
- Parse by checking magic bytes, then walking structured headers
- Always bounds-check every offset before dereferencing

### JPEG2000 (JP2 / J2K)
- JP2 container: 12-byte signature `\x00\x00\x00\x0C jP \r\n\x87\n`, then walk JP2 boxes
  (each box: 4-byte size + 4-byte type). Find `ihdr` box → 4-byte height + 4-byte width
- Raw J2K: Starts with `\xFF\x4F` (SOC marker), look for SIZ marker `\xFF\x51` →
  skip 4 bytes → 4-byte width (XSiz) + 4-byte height (YSiz) as big-endian uint32

### JPEG XR (JXR)
- Magic: `II` (little-endian TIFF) + `0xBC01` at offset 2
- Walk IFD entries (12 bytes each): tag + type + count + value
- Key tags: `0xBC80` (ImageWidth), `0xBC81` (ImageHeight), `0xBCC0`–`0xBCC2` (color info)

### DjVu
- Magic: `AT&TFORM` (8 bytes) or `AT&T` prefix variants
- IFF85 chunk structure: 4-byte ID + 4-byte big-endian size
- `INFO` chunk: 2-byte width + 2-byte height (little-endian) at chunk start
- Cover image data in `BG44` (background) or `FG44` (foreground) chunks

### glTF / GLB
- GLB binary: 12-byte header (magic `glTF` + version + length), then JSON chunk (type `JSON`)
- Parse JSON for `meshes[].primitives[].attributes.POSITION` accessor index
- Look up accessor in `accessors[]` array → `count` field = vertex count
- Triangle count ≈ vertex count (approximation without index buffer parsing)

### GIF frame counting
- After header (GIF87a/GIF89a) + LSD + optional GCT, walk blocks
- Extension blocks: `0x21` + sub-type + skip sub-blocks
- Image descriptors: `0x2C` → increment frame counter + skip sub-blocks
- Trailer: `0x3B` → done

---

## 14. COM Integration Patterns

### JumpList implementation

```text
ICustomDestinationList → BeginList → Create IShellLink items →
Add to IObjectCollection → AppendCategory → CommitList
```

- Use `CoCreateInstance(CLSID_DestinationList, ...)` (requires `shobjidl_core.h`)
- Each task: `CoCreateInstance(CLSID_ShellLink, ...)` → `SetPath` + `SetArguments` + `SetDescription`
- Set `PKEY_Title` via `IPropertyStore` on each `IShellLink`
- Cleanup: `Release()` all COM pointers in reverse order

### Safe COM header inclusion
- `shobjidl_core.h` is safe under WIN32_LEAN_AND_MEAN
- `objectarray.h`, `propsys.h`, `propkey.h` are safe
- `shlguid.h` (for CLSID_ShellLink) may need `INITGUID` defined beforehand
- Always test compilation after adding new COM headers

---

## 15. Security Scanning Patterns

### PE import table scanning
- Memory-map the executable with `CreateFileMappingW` + `MapViewOfFile`
- Parse DOS header (`IMAGE_DOS_HEADER`), follow `e_lfanew` to NT headers
- Walk `IMAGE_IMPORT_DESCRIPTOR` array from data directory entry 1
- For each import DLL, walk `IMAGE_THUNK_DATA` → `IMAGE_IMPORT_BY_NAME` → check function name
- Blocklist: `CreateRemoteThread`, `WriteProcessMemory`, `VirtualAllocEx`, `NtCreateProcess`,
  `SetWindowsHookEx`, `LoadLibraryA` (in plugin context), etc.

### Authenticode verification
- Use `WinVerifyTrust` with `WINTRUST_ACTION_GENERIC_VERIFY_V2` action GUID
- Requires dynamic loading of `wintrust.dll` under WIN32_LEAN_AND_MEAN
- `dwUIChoice = WTD_UI_NONE`, `dwProvFlags = WTD_REVOCATION_CHECK_NONE` for silent check
- Return codes: `ERROR_SUCCESS` = valid, `TRUST_E_NOSIGNATURE` = unsigned,
  `TRUST_E_BAD_DIGEST` = tampered

---

## 16. SEH Exception Handling (Fuzzing)

### __try/__except pattern for decoder fuzzing
- Wrap decoder calls in `__try / __except(EXCEPTION_EXECUTE_HANDLER)` to catch access violations
- **MSVC C2712 constraint:** Functions using `__try` cannot contain objects with destructors
- Workaround: Extract the `__try` block into a separate helper function with no local C++ objects

```cpp
// Separate function — no C++ objects with destructors allowed here
static DWORD InvokeDecoderSEH(DecoderFunc func, const uint8_t* data, size_t size) {
    __try { func(data, size); return 0; }
    __except(EXCEPTION_EXECUTE_HANDLER) { return GetExceptionCode(); }
}
// Caller function can freely use std::string, std::vector, etc.
```

- This pattern lets the fuzzing engine detect crashes without terminating the process

---

## 17. GUI Version Synchronization

### Files requiring version updates for LENSManager
In addition to the Engine version checklist (Section 3), the GUI has its own version locations:
- `LENSManager/LENSManager.rc` — `FILEVERSION`, `PRODUCTVERSION`, string table, about dialog text
- `LENSManager/About.cpp` — version string in about dialog, decoder count, format count
- `LENSManager/ExportDiagnostics.h` — baseline filename, version string in diagnostic export
- `LENSManager/DecoderHealthCheck.h` — version comment in health check header
- `LENSManager/MainDlg.cpp` — status bar format count text (e.g., "200+ formats")

### GUI feature wiring pattern
1. Add control ID in `resource.h` (next available IDC_BTN_*, IDC_CB_* etc.)
2. Add control to dialog template in `.rc` file with positioning
3. Add `COMMAND_HANDLER(IDC_*, BN_CLICKED, OnHandler)` to message map in MainDlg.h
4. Declare handler: `LRESULT OnHandler(WORD, WORD, HWND, BOOL&);`
5. Implement handler in MainDlg.cpp
6. For Reset Defaults: iterate `allCheckboxes[]` array and set `BST_CHECKED`
7. For Export Config: use `CFileDialog` save dialog → serialize settings to JSON

---

## 18. Aggregate Format Checkbox Pattern (v15.0 Post-Release)

### When to use aggregate checkboxes
- When adding many related formats that users would toggle as a group (not individually)
- When dialog real estate cannot accommodate individual checkboxes for every extension
- Examples: "Extended Images" (BMP, GIF, WMF, EMF, PCX, JP2, ...), "Texture" (KTX, VTF)

### Implementation checklist for aggregate format groups
Each aggregate checkbox requires changes in **5 files, 15+ locations**:

1. **`resource.h`** — `IDC_CB_GROUPNAME` (next available ID in 1072+ range)
2. **`RegManager.h`** — 7 changes:
   - `LENS_GROUPNAME` define (unique ID in 130+ range)
   - `LENS_PRIMARY_EXT_TH_KEY` / `LENS_PRIMARY_EXT_IH_KEY` registry path macros
   - `GetExtension()` case returning primary extension string
   - `GetTHKeyName()` / `GetIHKeyName()` cases returning primary key
   - `SetHandlers()` multi-extension block with TH/IH arrays + backup name arrays
3. **`LENSManager.rc`** — BS_AUTO3STATE checkbox in appropriate group box
4. **`MainDlg.h`** — `COMMAND_HANDLER(IDC_CB_GROUPNAME, BN_CLICKED, OnCheckboxClicked)`
5. **`MainDlg.cpp`** — 10+ locations:
   - `OnInitDialog()` visibility array
   - `InitUI()` `Button_SetCheck` call
   - `OnApplyImpl()` `formatHandlers[]` entry
   - `InitTooltips()` `AddTooltipWithStatus` call
   - `UpdateStatusBar()` `allFormats[]` entry
   - `GetEnabledFormatCount()` `HasTH` check
   - `InitStatusIcons()` `m_checkboxStatus` mapping
   - `OnSelectAll()` / `OnDeselectAll()` arrays
   - `CaptureCurrentConfig()` / `ApplyConfigSnapshot()`
   - `OnResetDefaults()` defaults array
6. **`ChangeSummaryDlg.h`** — `ConfigSnapshot` struct: new `bool` field

### SetHandlers multi-extension block pattern

```cpp
else if (LENSTYPE == LENS_GROUPNAME) {
    const LPCTSTR thKeys[] = {
        LENS_PRIMARY_TH_KEY,
        _T("SOFTWARE\\Classes\\.EXT2\\shellex\\{BB2E617C-...}"),
        // ... more extensions
    };
    const LPCTSTR ihKeys[] = { /* matching infotip paths */ };
    const LPCTSTR backupTH[] = { _T("ext1_th"), _T("ext2_th"), ... };
    const LPCTSTR backupIH[] = { _T("ext1_ih"), _T("ext2_ih"), ... };
    for (int i = 0; i < _countof(thKeys); i++) {
        if (bSet) { /* BackupHandler + Create keys */ }
        else { /* Check GUID match + RestoreHandler or RegDeleteKey */ }
    }
    return;
}
```

- All 4 arrays MUST have identical element counts
- Use `_countof()` (not hardcoded length) for loop bounds
- Always check `StrCmpI(currentGuid, LENS_GUID_KEY) == 0` before removing

### GetExtension() NULL crash prevention
- `GetExtension()` MUST return a non-NULL string for every `LENS_*` define
- Missing cases cause NULL dereference in `GetHandlerStatus()` → `UpdateStatusBar()` → crash
- When adding new LENS_* defines, ALWAYS add a matching GetExtension() case
- Return the most representative extension (e.g., `.bmp` for LENS_EXT_IMAGE group)

### LENS_* ID allocation scheme
- 0–76: Original format IDs (LENS_ZIP through LENS_MODEL)
- 100–127: Individual extended formats (LENS_BMP through LENS_ODP)
- 130–133: Aggregate format groups (LENS_EXT_IMAGE through LENS_EXT_DOCUMENT)
- Leave gaps for future growth — next individual at 128+, next aggregate at 134+

---

## 19. Sprint/Batch Comment Cleanup Methodology

### Scale and scope
- v15.0 "Zenith" delivered 50 headers across sprints 544–593
- Each header had "(Sprint NNN)" in the file-level doc block and sometimes inline section comments
- Cleanup removed ALL sprint/version references while preserving meaningful documentation

### Sprint comment patterns to search for

```text
(Sprint \d+)           — inline sprint markers
@sprint \d+            — doxygen sprint tags
Sprint \d+ —           — sprint header lines
v15\.0\.0              — version references in sprint context
ExplorerLens Engine v\d+\.\d+\.\d+ \(Sprint — full sprint header lines
```

### Cleanup procedure
1. `grep_search` with `Sprint \d+` across `Engine/` to identify all affected files
2. Process in batches of 10-15 files grouped by subdirectory (Cache, GPU, Memory, Pipeline, etc.)
3. For file-level doc blocks: remove entire "(Sprint NNN)" suffix or "@sprint" doxygen tag
4. For inline section comments (`// Sprint 553: Streaming buffers`): remove entirely
5. For "@brief Sprint NNN — Description": change to "@brief Description" (preserve the description)
6. Verify with `grep_search` for `Sprint \d+` returning 0 matches after each batch
7. Final verification: 0 sprint references across entire Engine/ directory

### What to preserve during cleanup
- File-level `@brief` descriptions (only strip the sprint prefix)
- `@param`, `@return`, `@throws` documentation blocks
- Meaningful section separators (`// --- Category ---`)
- Class/struct doc comments explaining purpose and usage
- Copyright headers and license notices

### Version banner cleanup (v15.0 Post-Release)
- All 49 Engine headers had `// ExplorerLens Engine v15.0.0 "Zenith"` or `=====` decorator banners
- Replaced all with standardized Copyright doc-block format (see Section 10)
- EngineTests.cpp had `// ===== Batch N: Category =====` section headers →
  replaced with `// --- Category ---` (domain names only, no batch numbers)
- Inline comments that just restated the struct/enum name were removed (77 total)
- Pattern: `// Configuration` before `struct Config` is noise → remove
- Pattern: `/// Backends listed in preference order` is useful → keep
- Useful comments describe *why* or *how*, not *what* the next line declares

---

## 20. GUI Dialog Layout & Dark Theme (v15.0 Post-Release)

### Groupbox spacing overlap pattern
- WTL `.rc` dialog groupboxes with 4 checkboxes at y-spacing of 10 DLU need H=65 minimum
- **Root cause:** H=62 gave exactly 0 DLU gap between the last checkbox and the groupbox border,
  causing the checkbox text to overlap or clip the bottom border
- **Fix:** H=62→H=65 (+3 DLU) for all 4-item groups; cascade all Y positions by +3 per fixed group
- When changing groupbox heights, update: StatusBar Y, Apply/Cancel button Y, dialog height,
  `OnGetMinMaxInfo` min track size, and Advanced Options groupbox if present

### Dark theme and BS_GROUPBOX visual styles
- `DarkMode_Explorer` visual style theme (applied via `SetWindowTheme(hChild, L"DarkMode_Explorer", NULL)`)
  overrides GDI text color for groupbox controls with a hardcoded dark-on-dark color
- **Fix:** In `ApplyDarkScrollbars`, detect `BS_GROUPBOX` via `(style & 0x0FL) == BS_GROUPBOX` and
  call `SetWindowTheme(hChild, L"", L"")` to disable visual styles for that control only
- This forces the groupbox to use GDI's `WM_CTLCOLORSTATIC` handler, which respects the white text
  brush set by `OnCtlColorStatic`
- Other button types (pushbutton, checkbox) still get `DarkMode_Explorer` theming as before

### Per-category Select All/Deselect All pattern
- 9 "All" checkboxes (one per format category) positioned in groupbox title area
- Position: `x=140` for column 1, `x=315` for column 2 (right-aligned in groupbox)
- Use `BS_AUTOCHECKBOX` for the "All" control (not BS_AUTO3STATE)
- Category membership: static arrays of `int` IDC values in `MainDlg.cpp`
- Toggle logic: `ToggleCategoryAll(int cbAll, const int* members, int count)` sets all members
  to the same state as the "All" checkbox
- Sync logic: `UpdateCategoryAllState(int cbAll, const int* members, int count)` checks all members
  and sets "All" to checked/unchecked/unchecked (no indeterminate for simplicity)
- Wire `OnSelectAll`/`OnDeselectAll` to also call `UpdateAllCategoryCheckboxStates()`

### System status detection via tri-state checkboxes
- `HasTH()` only returns `TRUE`/`FALSE` — insufficient for "who owns the handler?"
- **Pattern:** Use `GetHandlerStatus()` which returns `HandlerStatus` enum:
  - `HANDLER_NONE` (0) → `BST_UNCHECKED` (no handler registered)
  - `HANDLER_ExplorerLens` (1) → `BST_CHECKED` (our extension active)
  - `HANDLER_NATIVE` (2) → `BST_INDETERMINATE` (Windows native handler — shows −)
  - `HANDLER_THIRD_PARTY` (3) → `BST_INDETERMINATE` (third-party tool — shows −)
- Requires `BS_AUTO3STATE` checkbox style in `.rc` file (not `BS_AUTOCHECKBOX`)
- Call `ApplySystemStatusToCheckboxes()` in `InitUI()` instead of
  `Button_SetCheck(GetDlgItem(IDC), m_reg.HasTH(LENS_*))`

---

## 21. UNICODE Build Difference (Engine vs LENSManager)

### Critical compile environment difference
- **Engine** (CMake/Ninja): Defines `NOMINMAX`, `WINVER=0x0A00`, `_WIN32_WINNT=0x0A00`
  but does **NOT** define `UNICODE` or `_UNICODE`
- **LENSManager** (MSBuild): Defines `_UNICODE` and `UNICODE`
- This means Windows SDK macros expand differently between the two projects:
  - In Engine: `SE_LOCK_MEMORY_NAME` → `"SeLockMemoryPrivilege"` (ANSI char*)
  - In LENSManager: `SE_LOCK_MEMORY_NAME` → `L"SeLockMemoryPrivilege"` (wide wchar_t*)

### Impact on Windows API calls
- Functions like `LookupPrivilegeValueW` expect `LPCWSTR` (wide string) parameters
- Passing `SE_LOCK_MEMORY_NAME` in Engine code causes C2664 error because it expands to ANSI
- **Fix:** Use explicit wide string literals: `L"SeLockMemoryPrivilege"` instead of the macro
- Similarly, use `L"..."` for any string parameter passed to `*W` variants of Windows APIs
- For Engine headers that call Windows APIs: always use explicit `W` function variants
  and `L"..."` string literals — never rely on `TCHAR`/`_T()` macros

### Safe Windows API patterns for Engine headers
```cpp
// GOOD — explicit wide, works regardless of UNICODE define
::LookupPrivilegeValueW(nullptr, L"SeLockMemoryPrivilege", &luid);
::CreateFileW(L"path", ...);

// BAD — expands to ANSI in Engine build, causes type mismatch with W functions
::LookupPrivilegeValueW(nullptr, SE_LOCK_MEMORY_NAME, &luid);  // C2664!
```

---

## 22. Memory Allocator Implementation Patterns (v15.0 Post-Release)

### Arena (bump) allocator — MemoryArenaAllocator.h
- Linked-list of fixed-size blocks allocated via `std::malloc`
- Bump pointer within current block; new block allocated on overflow
- Alignment: `(offset + align - 1) & ~(align - 1)` — power-of-2 only
- `Reset()` frees all blocks except the first; `FreeAllBlocks()` frees everything
- Move-only semantics (deleted copy ctor/assignment)
- No Windows API dependency — portable C++ with `<cstdlib>` and `<cstdint>`

### Large page allocator — LargePageAllocator.h
- Uses `VirtualAlloc(MEM_LARGE_PAGES | MEM_COMMIT | MEM_RESERVE)` for 2MB pages
- Requires `SeLockMemoryPrivilege` — acquired at runtime via:
  `OpenProcessToken` → `LookupPrivilegeValueW` → `AdjustTokenPrivileges`
- Fallback: if privilege not available or VirtualAlloc fails, falls back to standard
  `VirtualAlloc(MEM_COMMIT | MEM_RESERVE)` with 4KB pages
- Deallocation: `VirtualFree(ptr, 0, MEM_RELEASE)`
- Links against: `advapi32.lib` (for token/privilege APIs — already in default MSVC link set)

### NUMA-aware allocator — NUMAMemoryRouter.h
- Topology detection: `GetNumaHighestNodeNumber()` for node count
- Local node: `GetCurrentProcessorNumberEx()` + `GetNumaProcessorNodeEx()`
- Per-node memory query: `GetNumaAvailableMemoryNode()` (returns bytes available)
- Per-node processor count: `GetNumaNodeProcessorMaskEx()` + `__popcnt64()` on mask
- Allocation: `VirtualAllocExNuma(GetCurrentProcess(), NULL, size, MEM_COMMIT|MEM_RESERVE,
  PAGE_READWRITE, preferredNode)` with fallback to standard `VirtualAlloc`
- Interleaving: round-robin across nodes via `(m_nextNode++) % m_nodeCount`
- Deallocation: `VirtualFree(ptr, 0, MEM_RELEASE)` — same as large page

---

## 23. VS Code / Copilot Build Workflow Lessons

### File save gotcha
- `replace_string_in_file` edits the VS Code buffer but may NOT auto-save to disk
- **Always** run `workbench.action.files.saveAll` before building
- Symptom: MSBuild LNK2001 "unresolved external symbol" for newly added functions
- Diagnostic: `[System.IO.File]::ReadAllText(path).IndexOf("functionName")` returns -1 on disk

### Terminal output buffering (VS Code integrated terminal)
- The shared foreground PowerShell terminal accumulates old output across commands
- Background terminals (`isBackground: true`) may return empty `get_terminal_output`
- **Most reliable approach:** Write output to log files, then `read_file` the log
- Use `Tee-Object` or `> logfile 2>&1` to capture output
- Use `build-and-log.bat` which redirects all build output to a dated log file

### Build timing expectations
- `EngineTests.cpp` (~22K lines): ~60-90 seconds to compile
- LTCG linking: ~30 seconds
- Total incremental build after header change: ~90-120 seconds
- "ninja: no work to do" means file timestamps haven't changed — touch files or check save state
- Never send Ctrl+C/Break to a building terminal — builds are slow but normal
