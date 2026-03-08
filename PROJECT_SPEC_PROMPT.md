# UNIVERSAL PROJECT ENHANCEMENT SPECIFICATION

**Professional Software Development Methodology**  
*Transform any script or application into a production-ready, enterprise-grade solution*

**Version: 13.0.0 - Complete Enhancement Framework**  
**Updated: March 2026**

---

## TABLE OF CONTENTS

1. [What Is This Specification?](#what-is-this-specification)
2. [Core Philosophy & Principles](#core-philosophy--principles)
3. [Critical Success Patterns](#critical-success-patterns)
4. [Project Structure Template](#project-structure-template)
5. [Universal Enhancement Categories](#universal-enhancement-categories)
6. [Configuration Management](#configuration-management)
7. [Implementation Methodology](#implementation-methodology)
8. [Quality Assurance & Testing](#quality-assurance--testing)
9. [Success Metrics & Validation](#success-metrics--validation)
10. [Adaptation Guidelines](#adaptation-guidelines)
11. [C++ / Native Desktop Projects](#c--native-desktop-projects)
12. [Dual-GUI Parity (Native + Python)](#dual-gui-parity-native--python)
13. [CMake Build System Patterns](#cmake-build-system-patterns)
14. [GitHub Repository Best Practices](#github-repository-best-practices)
15. [VS Code Workspace Configuration](#vs-code-workspace-configuration)
16. [Windows COM / Shell Extension Patterns](#windows-com--shell-extension-patterns)
17. [AI-Assisted Development Workflow](#ai-assisted-development-workflow)

---

## WHAT IS THIS SPECIFICATION?

This specification is a **comprehensive, universal framework** for transforming any software project into a production-ready, enterprise-grade solution ready for GitHub publication. The methodology consolidates proven patterns for code organization, naming consistency, cross-platform compatibility, and professional presentation based on real-world project transformations.

Whether you're starting with a simple script or enhancing an existing application, this framework provides proven patterns and methodologies to achieve enterprise-level quality across any technology stack or domain.

### Who Should Use This?

**For AI Assistants**: Apply this specification systematically to any project enhancement request. Follow the phases and checklists to ensure comprehensive coverage.

**For Developers**: Use this as a roadmap to professionalize your projects with industry best practices, enterprise features, and production-ready quality.

**For Organizations**: Implement this framework to standardize development practices and achieve consistent, high-quality software across all projects.

### Core Principles

1. **Portability First** - No hardcoded paths (`Path(__file__).parent` only)
2. **Single Entry Point** - One executable with command routing
3. **Three Interfaces** - CLI + Desktop GUI + Web GUI (full feature parity)
4. **Cross-Platform** - Windows, Linux, macOS, WSL tested
5. **Configuration-Driven** - YAML with `${ENV_VAR:default}` substitution
6. **Signal-Safe** - Graceful shutdown (SIGTERM/SIGINT) with cleanup
7. **Progress Indicators** - tqdm/rich for CLI, callbacks for GUI
8. **Zero Duplication** - One source of truth for everything
9. **Professional Docs** - Comprehensive, portable, synchronized with code
10. **GitHub Ready** - Clean structure, no artifacts or hardcoded values

---

## CORE PHILOSOPHY & PRINCIPLES

### Development Standards

- **Python-First Development**: Prefer Python for cross-platform compatibility, maintainability, and robust error handling
- **YAML-First Configuration**: Use YAML for human-readable, maintainable configuration over JSON
- **Modular Architecture**: Self-contained components with clear interfaces and single responsibility
- **Configuration-Driven Behavior**: External configuration controls all customizable aspects
- **Security-First Design**: User-space execution by default with selective privilege escalation
- **Package Manager Preference**: Prefer system package managers (APT, yum, brew) over language-specific installers
- **No Virtual Environments by Default**: System-wide installations preferred unless explicitly configured
- **Hardware-Aware Installation**: Detect hardware before installing drivers and hardware-specific packages

### Quality Standards

- **Error-First Development**: Design comprehensive error handling before implementing happy paths
- **Testing-Integrated Development**: Write tests alongside implementation, not as an afterthought
- **Documentation-Driven Development**: Maintain comprehensive documentation at all levels
- **Performance-Conscious Design**: Optimize for efficiency without sacrificing maintainability
- **User-Centric Design**: Progressive disclosure of features with multiple interface options
- **Graceful Degradation**: Always provide fallback mechanisms when dependencies are unavailable
- **Signal-Safe Operations**: Handle SIGTERM, SIGINT gracefully with proper cleanup

---

## CRITICAL SUCCESS PATTERNS

### 1. Portability

```python
# ✅ Good
PROJECT_ROOT = Path(__file__).parent.resolve()
config = PROJECT_ROOT / "config" / "default.yaml"

# ❌ Bad  
config = "C:\\Users\\name\\project\\config\\default.yaml"
```

**Requirements**:
- Use `Path(__file__).parent` everywhere
- Generic placeholders in docs (`<username>`, not specific names)
- Relative references in help (`./README.md`)
- Works from any directory

### 2. Single Entry Point Consolidation

**Problem**: Multiple entry points create confusion, maintenance overhead, and user experience issues

**Solution**: Consolidate all functionality into one unified script with clear, consistent naming

**Implementation**:
- Remove duplicate scripts (`script-advanced.py`, `script-enhanced`, etc.)
- Merge all functionality into the main project script
- Update all documentation to reference single entry point
- Eliminate intermediate import files and wrapper scripts
- Name main executable after project (e.g., `project-name` without .py extension)
- Create clear command routing system for all features

```python
def main():
    parser = argparse.ArgumentParser()
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument('--gui', '--web', '--cli')
    
    if args.web: start_web_gui()
    elif args.cli: run_cli()
    else: start_desktop_gui()
```

### 3. Three Interfaces (Feature Parity)

```
Desktop GUI (Tkinter) ──┐
Web GUI (FastAPI)  ──────┼──> Shared Backend Services
CLI (argparse)     ──────┘
```

**Desktop GUI (Tkinter)**:
- Standalone application for local desktop use
- Professional styling with ttk themes
- All CLI functionality accessible
- Real-time progress indicators
- Configuration management (load, save, apply, reset)
- Cross-platform (Windows, Linux, macOS)
- Keyboard shortcuts and accessibility

**Web GUI (FastAPI/Flask)**:
- Browser-based interface for remote/headless systems
- Responsive design (desktop and mobile)
- All CLI functionality accessible via web
- Real-time updates (WebSockets/SSE)
- REST API for programmatic access
- Modern frameworks (Bootstrap, htmx)
- Authentication-ready architecture

**Shared Backend Layer**:
- Service classes that both GUIs call
- Identical functionality and behavior
- Unified configuration management
- Shared progress callback system
- Common error handling and logging
- Zero business logic duplication

**Feature Parity Checklist**:
- [ ] All commands accessible in desktop GUI
- [ ] All commands accessible in web GUI
- [ ] Configuration editing in both GUIs
- [ ] Progress tracking in all interfaces
- [ ] Help/documentation in all interfaces
- [ ] Error messages consistent across interfaces

### 4. Graceful Signal Handling and Shutdown

**Signals to Handle**:
- SIGTERM (termination request)
- SIGINT (Ctrl+C)
- SIGHUP (hang up, if applicable)

**Implementation Pattern**:
```python
import signal
import atexit
import sys

class GracefulShutdown:
    def __init__(self):
        self.is_shutting_down = False
        self.cleanup_handlers = []
        
        signal.signal(signal.SIGTERM, self.handle_shutdown)
        signal.signal(signal.SIGINT, self.handle_shutdown)
        atexit.register(self.cleanup)
    
    def handle_shutdown(self, signum, frame):
        if self.is_shutting_down:
            print("\nForce exit!")
            sys.exit(1)
        
        self.is_shutting_down = True
        print("\nShutting down gracefully...")
        self.cleanup()
        sys.exit(0)
    
    def register_cleanup(self, handler):
        self.cleanup_handlers.append(handler)
    
    def cleanup(self):
        for handler in self.cleanup_handlers:
            try:
                handler()
            except Exception as e:
                print(f"Cleanup error: {e}")
```

**Cleanup Requirements**:
- Close all file handles explicitly
- Terminate or join all child threads/processes
- Remove temporary files and directories
- Flush logging buffers
- Release network connections and locks
- Save important application state
- Implement timeout for cleanup (5-10 seconds max)

### 5. Progress Indicators and Status Reporting

**CLI Progress (rich)**:
```python
from rich.progress import Progress, SpinnerColumn, TextColumn, BarColumn

with Progress(
    SpinnerColumn(),
    TextColumn("[progress.description]{task.description}"),
    BarColumn(),
    TextColumn("[progress.percentage]{task.percentage:>3.0f}%"),
) as progress:
    task = progress.add_task("Processing...", total=len(items))
    for item in items:
        process_single_item(item)
        progress.update(task, advance=1)
```

**GUI Progress Integration**:
```python
def process_with_progress(items, callback=None):
    for i, item in enumerate(items):
        process(item)
        if callback: callback(i+1, len(items), f"Processing {item}")
```

### 6. Intelligent Package Management Strategy

**Preference Order**:
1. **System Package Managers** (APT, yum, brew, choco)
2. **Language-Specific** (pip with `--break-system-packages`, npm, cargo)
3. **Universal** (Snap, Flatpak, Conda)
4. **Manual Installation** (last resort)

```python
def detect_package_managers():
    """Detect available package managers"""
    managers = {}
    checks = {
        'apt': ['apt', '--version'],
        'yum': ['yum', '--version'],
        'brew': ['brew', '--version'],
        'pip': ['pip', '--version'],
    }
    for name, cmd in checks.items():
        try:
            subprocess.run(cmd, capture_output=True, timeout=5, check=True)
            managers[name] = True
        except (subprocess.SubprocessError, FileNotFoundError):
            managers[name] = False
    return managers
```

### 7. Proxy Handling (No Hardcoding)

```yaml
network:
  proxy:
    enabled: "${PROXY_ENABLED:false}"
    http: "${HTTP_PROXY:}"
    https: "${HTTPS_PROXY:}"
    no_proxy: "${NO_PROXY:localhost,127.0.0.1}"
  prefer_direct: true
```

**Strategy**:
- Try direct connection first
- Check environment variables
- Read from configuration
- Test & fallback
- CLEANUP after use! Never hardcode proxy URLs

### 8. Configuration Management

**Hierarchy** (highest to lowest):
1. Command-line arguments
2. Environment variables
3. User configuration file
4. System configuration file
5. Default configuration

**Sample Configuration**:
```yaml
application:
  name: "${APP_NAME:My Application}"
  version: "1.0.0"
  environment: "${ENVIRONMENT:development}"

logging:
  level: "${LOG_LEVEL:INFO}"
  file: "${LOG_FILE:app.log}"
  format: "%(asctime)s [%(levelname)s] %(name)s: %(message)s"

performance:
  cache_size: "${CACHE_SIZE:1000}"
  max_workers: "${MAX_WORKERS:4}"
  timeout: "${TIMEOUT:30}"

security:
  user_space: true
  privilege_escalation: "selective"
  audit_enabled: "${AUDIT_ENABLED:false}"

network:
  proxy:
    enabled: "${PROXY_ENABLED:false}"
    url: "${PROXY_URL:}"
  timeout: "${NETWORK_TIMEOUT:30}"
  prefer_direct: true

gui:
  desktop:
    theme: "${GUI_THEME:default}"
    geometry: "${GUI_GEOMETRY:800x600}"
  web:
    host: "${WEB_HOST:127.0.0.1}"
    port: "${WEB_PORT:8080}"
```

---

## PROJECT STRUCTURE TEMPLATE

```
project-root/
├── project-name                  # Single entry point (no .py extension)
├── README.md                     # Main documentation
├── LICENSE                       # License (MIT recommended)
├── VERSION                       # Version number
├── requirements.txt              # Python dependencies
├── apt-packages.txt              # System package dependencies
├── pyproject.toml                # Modern Python config
├── .gitignore                    # Git ignore rules
│
├── src/                          # Source code
│   ├── __init__.py
│   │
│   ├── cli/                      # Command-line interface
│   │   ├── __init__.py
│   │   ├── main_cli.py
│   │   ├── commands.py
│   │   └── validators.py
│   │
│   ├── core/                     # Core business logic
│   │   ├── __init__.py
│   │   ├── business_logic.py
│   │   ├── signal_handler.py
│   │   ├── logging_framework.py
│   │   └── performance.py
│   │
│   ├── utils/                    # Utilities
│   │   ├── __init__.py
│   │   ├── portable.py
│   │   ├── config_manager.py
│   │   ├── network_utils.py
│   │   ├── file_utils.py
│   │   └── progress.py
│   │
│   ├── integrations/             # External integrations
│   │   ├── __init__.py
│   │   ├── gui_tkinter.py
│   │   ├── web_interface.py
│   │   └── api.py
│   │
│   └── models/                   # Data models
│       ├── __init__.py
│       └── config_schema.py
│
├── config/                       # Configuration files (YAML)
│   ├── default.yaml
│   ├── production.yaml
│   └── development.yaml
│
├── tests/                        # Test suite
│   ├── __init__.py
│   ├── conftest.py
│   ├── test_core.py
│   ├── test_cli.py
│   └── test_gui.py
│
├── docs/                         # Documentation
│   ├── README.md
│   ├── QUICK_START.md
│   ├── API.md
│   ├── CONFIGURATION.md
│   └── TROUBLESHOOTING.md
│
├── scripts/                      # Utility scripts
│   ├── install.py
│   └── deploy.py
│
├── examples/                     # Usage examples
│   └── basic_usage.py
│
└── archive/                      # Legacy files (optional)
    └── ARCHIVE_MANIFEST.md
```

**Root Directory Rule**: Only entry point, README, LICENSE, VERSION, requirements.txt, apt-packages.txt, pyproject.toml, .gitignore, and directories.

---

## UNIVERSAL ENHANCEMENT CATEGORIES

### 1. User Interface Requirements

| Interface | Framework | Purpose |
|-----------|-----------|---------|
| Desktop GUI | Tkinter + ttk | Local desktop use |
| Web GUI | FastAPI/Flask | Remote/headless access |
| CLI | argparse + rich | Automation and scripting |

### 2. Cross-Platform Support

| Platform | Status |
|----------|--------|
| Windows 10/11 | Required |
| WSL (Ubuntu) | Required |
| Linux (Ubuntu/Debian) | Required |
| macOS | Recommended |

### 3. Error Handling & Logging

```python
import logging
from pathlib import Path

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(name)s: %(message)s',
    handlers=[
        logging.FileHandler(Path(__file__).parent / 'logs' / 'app.log'),
        logging.StreamHandler()
    ]
)
```

### 4. Testing Framework

- **Unit Tests**: pytest, 90%+ coverage goal
- **Integration Tests**: All feature workflows
- **Cross-Platform Tests**: All target platforms
- **Performance Tests**: Response time benchmarks

---

## IMPLEMENTATION METHODOLOGY

### Phase 1: Foundation (2-3 days)
- Structure, entry point, config, logging, signals, portability

### Phase 2: CLI System (2-3 days)
- CLI system, argparse, progress, help

### Phase 3: GUI Development (4-5 days)
- Shared backend, Desktop GUI, Web GUI, feature parity

### Phase 4: Testing & Quality (2-3 days)
- Cross-platform tests, 90%+ coverage, benchmarks, security

### Phase 5: Production Prep (1-2 days)
- Version/naming consistency, clean root, portability check, GitHub prep

---

## QUALITY ASSURANCE & TESTING

### Quality Checklist

#### Code
- [ ] Single entry point, no duplicates
- [ ] All paths relative (portable)
- [ ] Clean root directory
- [ ] Consistent naming
- [ ] No hardcoded values

#### Functionality
- [ ] Cross-platform tested
- [ ] Config loading works
- [ ] Error handling comprehensive
- [ ] Signal handling functional
- [ ] Progress tracking working

#### Interfaces
- [ ] Desktop GUI launches
- [ ] Web GUI accessible
- [ ] CLI fully functional
- [ ] Feature parity verified
- [ ] Config UI in both GUIs

#### Portability
- [ ] No hardcoded paths in code
- [ ] Generic placeholders in docs
- [ ] Relative paths in help
- [ ] Works from any directory
- [ ] Tested on multiple platforms

#### Documentation
- [ ] README comprehensive
- [ ] QUICK-START complete
- [ ] All docs use portable paths
- [ ] Help system complete
- [ ] Examples tested

#### Security
- [ ] No hardcoded credentials/proxies
- [ ] Proxy cleanup after use
- [ ] Graceful shutdown
- [ ] Audit logging (if needed)

---

## SUCCESS METRICS & VALIDATION

| Metric | Target |
|--------|--------|
| Code | Clean, modular, documented, portable |
| Tests | 90%+ coverage, all platforms |
| Performance | Sub-second response |
| Security | Zero critical vulnerabilities |
| Portability | 100% compatible, no hardcoded paths |
| Docs | Complete, accurate, portable |

---

## ADAPTATION GUIDELINES

### For Different Project Types

| Type | Focus Areas |
|------|-------------|
| CLI Tools | Rich CLI, config files, shell completion |
| Web Apps | API design, authentication, database |
| Desktop Apps | Cross-platform GUI, local storage, auto-update |
| System Admin | Security, privilege management, audit logging |
| Data Processing | Performance, batch processing, progress tracking |
| Enterprise | Security, compliance, centralized config |

### For Different Technology Stacks

| Stack | Equivalent Components |
|-------|----------------------|
| Python | argparse, pytest, PyYAML, tqdm, rich |
| Node.js | commander, jest, js-yaml, ora |
| Java | Spring Boot, JUnit, SnakeYAML |
| Go | Cobra, testing, gopkg.in/yaml |

### Common Pitfalls to Avoid

**Don't**:
- Hardcode absolute paths
- Use specific usernames in docs
- Skip signal handlers
- Miss GUI/CLI feature parity
- Hardcode proxy configs
- Leave dev artifacts

**Do**:
- Relative paths everywhere
- Generic placeholders
- Comprehensive signals
- Verify feature parity
- Config-driven proxy
- Clean structure

---

## C++ / NATIVE DESKTOP PROJECTS

### Build Toolchain (Windows)

When the primary target is Windows native code, establish a fixed MSVC toolchain:

| Component | Best Practice |
|-----------|--------------|
| **Compiler** | MSVC cl.exe (specific version pinned in CI + copilot-instructions.md) |
| **Build System** | CMake 3.25+ with Presets (`CMakePresets.json`) + Ninja generator |
| **Package Manager** | vcpkg (manifest mode) or local `external/` with per-library build scripts |
| **CRT Linkage** | `/MD` (dynamic CRT) across ALL targets and ALL external libs — consistency is critical |
| **Warning Level** | `/W4` with zero-warnings policy enforced in CI |
| **Static Analysis** | `.clang-tidy` config committed to repo |

### Key Learnings — C++ Projects

1. **Pin your toolset version** in copilot-instructions.md — AI assistants pick up random compilers from PATH otherwise
2. **Source `vcvars64.bat` before CMake** — without it, CMake may find Clang/GCC instead of MSVC
3. **CRT mismatch is the #1 link error** — if one `.lib` uses `/MT` and another `/MD`, you get `LNK2038`. Rebuild ALL externals with the same CRT flag
4. **External libraries need build scripts** — each dependency gets its own `Build-<Lib>.ps1` importing a shared `Build-Library-Core.ps1` module
5. **Header-only is fine at scale** — 860+ header-only components are valid when each is a self-contained feature with `Initialize()`/`GetName()` pattern
6. **Namespace everything** — `namespace ProjectName { namespace Engine { } }` prevents collisions
7. **Naming conventions matter in large codebases** — PascalCase classes, camelBack variables, `m_` prefix for members, `UPPER_CASE` constants

### Header Banner Standard

```cpp
// FileName.h — Short Title
// Copyright (c) 2026 ProjectName Project
//
// Description of what this header provides.
//
#pragma once
```

- No decorator lines (`===`), no version numbers, no sprint tags
- `#pragma once` after the banner (not `#ifndef` guards)

### Test Framework Pattern (Custom)

For projects that don't need GTest overhead:

```cpp
// Custom lightweight test macros
#define TEST(name) void name()
#define RUN_TEST(name) do { g_testsRun++; try { name(); g_testsPassed++; } \
    catch (...) { g_testsFailed++; } } while(0)
#define ASSERT(cond) if (!(cond)) throw std::runtime_error(#cond)
```

- Naming: `Test_S<Sprint>_<ClassName>` for traceability
- All tests in a single `EngineTests.cpp` with `#include` per feature header
- Large test files (22K+ lines) take ~90s to compile — this is normal

---

## DUAL-GUI PARITY (NATIVE + PYTHON)

### Architecture

When a project has both a native (C++/WTL) GUI and a Python (tkinter) GUI:

```
Native GUI (WTL/Win32)  ──→ Registry-based config
Python GUI (tkinter)    ──→ JSON-based config
                              ↕ (shared format definitions)
Both manage the same COM registration / shell integration
```

### UI/UX Parity Checklist

Both GUIs must implement:

- [ ] **Per-format checkboxes** (not just category toggles) — users need granular control
- [ ] **Category groupboxes** with "Select All" per group
- [ ] **Global Select All / Deselect All** buttons
- [ ] **Collage mode** radio/combo (1×1, 2×2, 3×3, 4×4)
- [ ] **Sort** and **Show Icon** options
- [ ] **Dark mode** with system detection (`AppsUseLightTheme` registry key)
- [ ] **Change summary** dialog before applying (show what will change, allow revert)
- [ ] **Performance dashboard** (cache stats, decode speed, throughput)
- [ ] **Format status indicators** (Active / Degraded / Unavailable)
- [ ] **Decoder health check** per format
- [ ] **Export diagnostics** bundle (system info, decoder health, settings)
- [ ] **System tray icon** with Show/Quit menu
- [ ] **Config import/export** (JSON for Python, .reg for native)
- [ ] **Status bar** showing enabled format count
- [ ] **Tooltips** with format details and file extensions
- [ ] **Registration** controls (register/unregister with UAC elevation)
- [ ] **About** dialog with system info

### Dark Mode Implementation

**Native (WTL/Win32)**:
```cpp
// Use undocumented DWM APIs for dark title bar
DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
// Windows 11: Mica backdrop + rounded corners
DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &MICA, sizeof(MICA));
```

**Python (tkinter)**:
```python
# Detect from Windows registry
key = winreg.OpenKey(HKEY_CURRENT_USER,
    r"Software\Microsoft\Windows\CurrentVersion\Themes\Personalize")
val, _ = winreg.QueryValueEx(key, "AppsUseLightTheme")
is_dark = val == 0

# Apply via ttk.Style with clam theme
style.theme_use("clam")
style.configure(".", background="#1e1e1e", foreground="#d4d4d4")
```

### Config Mirroring

Python's `FORMAT_REGISTRY` dict should mirror native's resource IDs:
- Each format key maps to native's `IDC_CB_*` / `LENS_*` constant
- Category groups mirror native's `IDC_*_GROUP` groupboxes
- Collage mode values match: 1, 4, 9, 16
- Tooltip text should be consistent between both GUIs

---

## CMAKE BUILD SYSTEM PATTERNS

### CMakePresets.json

Define multiple presets for different scenarios:

```json
{
  "configurePresets": [
    {"name": "default-release", "generator": "Ninja", "binaryDir": "build"},
    {"name": "vcpkg-release", "generator": "Ninja", "toolchainFile": "...vcpkg.cmake"},
    {"name": "vs2026", "generator": "Visual Studio 18 2026"}
  ]
}
```

### ENGINE_HEADERS / ENGINE_SOURCES Pattern

For large projects with 800+ headers:

```cmake
set(ENGINE_HEADERS
    # Core (Section comment for organization)
    Core/FeatureA.h
    Core/FeatureB.h
    # Pipeline
    Pipeline/StageA.h
    # ... organized by subsystem
)

set(ENGINE_SOURCES
    Core/MainEngine.cpp
    # ... only files with actual implementation
)
```

**Rules:**
- New headers MUST be registered in ENGINE_HEADERS
- New .cpp files MUST be registered in ENGINE_SOURCES
- Use section comments (`# Core`, `# Pipeline`) for navigation
- Keep alphabetical within each section

### .gitignore for CMake Projects

```gitignore
# CRITICAL: Use leading slash for top-level only!
/build/           # ← Only matches root build/, NOT docs/build/
/build-vcpkg/
/build-logs/
```

**Lesson learned**: `build/` without leading slash matches ANY `build/` directory, including `docs/build/`. Always use `/build/` for top-level build directories.

---

## GITHUB REPOSITORY BEST PRACTICES

### Magic Filenames (MUST Be Uppercase)

GitHub has "magic" filenames that get special treatment. These **MUST** remain uppercase:

| File | Purpose |
|------|---------|
| `CONTRIBUTING.md` | Shown in PR creation UI, linked from issue templates |
| `SECURITY.md` | Shown in Security tab, linked from vulnerability reporting |
| `PULL_REQUEST_TEMPLATE.md` | Auto-populates PR description |
| `ISSUE_TEMPLATE/` | Directory with issue form templates |
| `CODE_OF_CONDUCT.md` | Linked from community health |
| `LICENSE` | Detected by GitHub for license badge |
| `FUNDING.yml` | Sponsors button configuration |

All other files (docs, configs, scripts) should use lowercase-with-dashes.

### `.github/` Directory Structure

```
.github/
├── CONTRIBUTING.md          # Uppercase — GitHub magic file
├── SECURITY.md              # Uppercase — GitHub magic file
├── PULL_REQUEST_TEMPLATE.md # Uppercase — GitHub magic file
├── ISSUE_TEMPLATE/          # Uppercase — GitHub magic directory
│   ├── bug_report.yml
│   └── feature_request.yml
├── copilot-instructions.md  # Lowercase — AI assistant config
├── development-learnings.md # Lowercase — team knowledge base
├── standards/               # Lowercase — project standards
│   ├── coding-standards.md
│   └── build-troubleshooting.md
└── workflows/               # Lowercase — CI/CD
    ├── ci.yml
    ├── release.yml
    └── arm64.yml
```

### `copilot-instructions.md` — The AI Config File

This is the **single most important file** for AI-assisted development. Include:

1. **Project overview** — what it is, version, language, build system
2. **Architecture diagram** — key directories and their purpose
3. **Toolchain table** — exact paths and versions for compiler, build tools
4. **Build commands** — the ONE command to build, test, clean
5. **Code conventions** — naming, header format, namespace pattern
6. **Key types** — the 5-10 most important classes/interfaces
7. **Testing** — framework, conventions, expected pass count
8. **Rules** — numbered list of "never do X" and "always do Y"
9. **Common pitfalls** — things an AI might get wrong (e.g., "never include `<versionhelpers.h>` with `WIN32_LEAN_AND_MEAN`")

### CI/CD Workflows

```yaml
name: CI
on: [push, pull_request]
jobs:
  build:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      - name: Setup MSVC
        uses: ilammy/msvc-dev-cmd@v1
        with:
          toolset: 14.50
      - name: Build
        run: cmake --preset default-release && cmake --build --preset default-release
      - name: Test
        run: ctest --test-dir build -C Release --output-on-failure
```

---

## VS CODE WORKSPACE CONFIGURATION

### Tasks (`tasks.json`)

Define tasks for every common operation:

```json
{
  "tasks": [
    {"label": "Build Engine (CMake Release)", "type": "shell", "command": "build-scripts/Build-MSVC.ps1"},
    {"label": "Clean Build", "type": "shell", "command": "build-scripts/Build-MSVC.ps1", "args": ["-Clean"]},
    {"label": "Build & Test", "type": "shell", "command": "build-scripts/Build-MSVC.ps1", "args": ["-Clean", "-Test"]},
    {"label": "Run Engine Tests", "dependsOn": "Build Engine", "command": "build/bin/EngineTests.exe"}
  ]
}
```

**Pattern**: Build tasks → Test tasks → Package tasks → Library build tasks

### Extensions Recommendations (`.vscode/extensions.json`)

```json
{
  "recommendations": [
    "ms-vscode.cpptools",
    "ms-vscode.cmake-tools",
    "github.copilot",
    "github.copilot-chat",
    "ms-python.python"
  ]
}
```

### Settings (`.vscode/settings.json`)

```json
{
  "cmake.configureOnOpen": false,
  "C_Cpp.default.cppStandard": "c++20",
  "files.associations": {"*.h": "cpp"},
  "editor.formatOnSave": true,
  "python.defaultInterpreterPath": "./ExplorerLens.py/.venv/Scripts/python.exe"
}
```

### Copilot Customization Files

| File | Scope | Purpose |
|------|-------|---------|
| `.github/copilot-instructions.md` | Repository | AI coding instructions for entire repo |
| `.vscode/*.instructions.md` | Workspace | VS Code-specific AI instructions |
| `.github/AGENTS.md` | Repository | Custom agent definitions |
| `SKILL.md` files | Skill | Domain-specific AI knowledge packages |
| `.prompt.md` files | Prompt | Reusable prompt templates |

### Build Monitoring Pattern

For long-running builds (60-120s):
1. Use `build-and-log.bat` wrapper that captures stdout to logfile
2. Monitor via `read_file` on log instead of watching terminal
3. Never send Ctrl+C to running builds
4. If terminal is busy, open a NEW terminal

---

## WINDOWS COM / SHELL EXTENSION PATTERNS

### IThumbnailProvider Architecture

```
Explorer.exe
  └─ Calls IExtractImage / IThumbnailProvider
      └─ COM CLSID lookup in HKCR\CLSID\{...}
          └─ InprocServer32 → LENSShell.dll
              └─ Selects decoder based on file extension
                  └─ Returns HBITMAP thumbnail
```

### Registry-Based Configuration

Native Windows apps store settings in registry:
```
HKCU\Software\ExplorerLens\
├── Handlers\    (per-format: CBZ=1, RAR=0, ...)
├── Options\     (Sort=0, ShowIcon=1)
├── Collage\     (Mode=4)
└── Performance\ (CacheSize=256, ...)
```

The Python GUI uses JSON config but must be able to **read and write** the same registry keys for interop.

### COM Registration Pattern

```cpp
// Fixed CLSID — NEVER change once published
// {9E6ECB90-5A61-42BD-B851-D3297D9C7F39}
HRESULT DllRegisterServer() {
    // Write CLSID\{...}\InprocServer32 = path to DLL
    // Write shellex\{E357FCCD...}\{CLSID} for each file extension
}
```

**Rules:**
- COM CLSID is immutable after first release
- Registration requires admin privileges (HKLM writes)
- Python companion uses a DIFFERENT CLSID to avoid conflicts
- Unregistration must clean up ALL registry entries

---

## AI-ASSISTED DEVELOPMENT WORKFLOW

### Sprint-Based Feature Delivery

Pattern for adding features in batches:

1. **Plan**: Define N features (e.g., 50 per sprint) with name, subsystem, purpose
2. **Create headers**: One `.h` per feature following the project template
3. **Register in build**: Add to `CMakeLists.txt` ENGINE_HEADERS
4. **Add tests**: `#include`, `TEST()` function, `RUN_TEST()` call in test file
5. **Build**: Must produce 0 errors, 0 warnings
6. **Test**: Must pass 100% (existing + new tests)
7. **Commit**: One descriptive commit per sprint with full change list

### copilot-instructions.md Maintenance

Update after each sprint with:
- New key types and their patterns
- Updated test count
- New build/test commands
- New conventions or rules discovered

### AI Safety Rules

1. **Never break zero-warnings build** — verify before committing
2. **Never change immutable identifiers** (COM CLSIDs, API versions)
3. **Never kill running build processes** unless confirmed stale (check `StartTime`)
4. **Always read before editing** — understand existing code
5. **Use build scripts** — don't run raw compiler commands
6. **Large test files compile slowly** — 22K line file taking 90s is normal, not hung
7. **LTCG linking takes 30s** — this is normal for Release builds

### Repository Memory Pattern

Store verified facts in `/memories/repo/`:
- Build commands that work
- Error patterns and their fixes
- File insertion points for common operations
- Test count milestones

---

---

## CONCLUSION

This Universal Project Enhancement Specification represents proven methodologies for transforming any project into a production-ready, enterprise-grade solution. Apply this specification systematically to transform any project into a professional, production-ready application with enterprise-grade quality, security, and operational excellence.

---

**Framework Status**: ✅ Production Ready  
**Validation**: ✅ Multi-Project Tested  
**Applicability**: ✅ Universal  
**Version**: 13.0.0  
**Updated**: March 2026  
**License**: MIT
