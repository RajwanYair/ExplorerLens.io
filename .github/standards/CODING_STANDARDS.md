# Coding Standards

This document defines the coding standards and conventions for the ExplorerLens project.

## Table of Contents

1. [C++ Standards](#c-standards)
2. [PowerShell Standards](#powershell-standards)
3. [Documentation Standards](#documentation-standards)
4. [Git Commit Standards](#git-commit-standards)
5. [File Organization](#file-organization)
6. [Code Review Guidelines](#code-review-guidelines)

---

## C++ Standards

### Language Version

- **Standard:** C++20
- **Compiler:** MSVC 19.50+ (Visual Studio 18 2026, v145 toolset)
- **Platform:** Windows x64 + ARM64

### Naming Conventions

```cpp
// Classes: PascalCase
class ThumbnailEngine { };
class IFormatDecoder { }; // Interfaces start with 'I'

// Functions/Methods: PascalCase
void GenerateThumbnail();
bool IsValidFormat();

// Variables: camelCase
int thumbnailSize;
std::string filePath;

// Member variables: camelCase with m_ prefix
class Example {
 int m_count;
 std::string m_name;
};

// Constants: UPPER_CASE or kPascalCase
const int MAX_THUMBNAIL_SIZE = 1024;
constexpr int kDefaultSize = 256;

// Namespaces: lowercase
namespace ExplorerLens { }
namespace ExplorerLens::decoders { }

// Enums: PascalCase for type, UPPER_CASE for values
enum class ImageFormat {
 UNKNOWN,
 JPEG,
 PNG,
 WEBP
};

// Type aliases: PascalCase with _t suffix
using ImageBuffer_t = std::vector<uint8_t>;
```

### File Naming

```
ThumbnailEngine.h // Header file: PascalCase
ThumbnailEngine.cpp // Implementation: PascalCase
format_utils.h // Utility headers: lowercase with underscores
```

### Code Formatting

```cpp
// Indentation: 4 spaces (NO TABS)
class Example {
 void Method() {
 if (condition) {
 DoSomething();
 }
 }
};

// Line length: 120 characters maximum
// Break long lines at logical points

// Braces: K&R style (opening brace on same line)
if (condition) {
 DoSomething();
} else {
 DoSomethingElse();
}

// Function definitions: opening brace on same line
void Function() {
 // ...
}

// Namespaces: no indentation inside
namespace ExplorerLens {

class MyClass {
 // ...
};

} // namespace ExplorerLens
```

### Header Files

```cpp
// Use #pragma once instead of include guards
#pragma once

// Include order:
// 1. Corresponding header (for .cpp files)
// 2. C system headers
// 3. C++ standard library headers
// 4. Third-party library headers
// 5. Project headers

#include "ThumbnailEngine.h" // Corresponding header

#include <windows.h> // C system headers
#include <d3d11.h>

#include <string> // C++ standard library
#include <vector>
#include <memory>

#include <webp/decode.h> // Third-party libraries

#include "Types.h" // Project headers
#include "Logger.h"

// Forward declarations when possible
class IFormatDecoder;
struct DecoderInfo;
```

### Documentation

```cpp
/// @brief Generate a thumbnail from an image file
/// @param filePath Path to the source image file
/// @param size Desired thumbnail size in pixels
/// @return HRESULT indicating success or failure
/// @throws None - returns error codes via HRESULT
/// @note This function is thread-safe
HRESULT GenerateThumbnail(const std::wstring& filePath, int size);

// Use Doxygen-style comments for public APIs
// Use regular comments for implementation details

class ThumbnailEngine {
public:
 /// @brief Initialize the thumbnail engine
 /// @param config Engine configuration
 /// @return S_OK on success, error code on failure
 HRESULT Initialize(const EngineConfig& config);

private:
 // Internal implementation detail
 void ProcessQueue();
};
```

### Error Handling

```cpp
// Use HRESULT for Windows APIs and COM
HRESULT LoadImage(const std::wstring& path) {
 if (path.empty()) {
 return E_INVALIDARG;
 }
 
 HRESULT hr = DoSomething();
 if (FAILED(hr)) {
 LogError(L"Failed to do something", hr);
 return hr;
 }
 
 return S_OK;
}

// Use exceptions sparingly (only in Engine, not in COM code)
void ParseConfig(const std::string& json) {
 if (json.empty()) {
 throw std::invalid_argument("Empty JSON");
 }
 // ...
}

// Check pointers before use
if (!ptr) {
 return E_POINTER;
}
```

### Modern C++ Features

```cpp
// Prefer smart pointers over raw pointers
std::unique_ptr<Decoder> decoder = std::make_unique<WebPDecoder>();
std::shared_ptr<Cache> cache = std::make_shared<ThumbnailCache>();

// Use auto for complex types
auto it = container.find(key);
auto result = CalculateSomethingComplex();

// Range-based for loops
for (const auto& item : collection) {
 ProcessItem(item);
}

// Structured bindings (C++17)
auto [success, data] = ParseImage(buffer);

// Use std::optional for optional values (C++17)
std::optional<int> FindValue(const std::string& key);

// Use std::variant for type-safe unions (C++17)
using Value = std::variant<int, double, std::string>;

// Use constexpr for compile-time constants
constexpr int kMaxSize = 1024;
```

### Thread Safety

```cpp
// Document thread safety explicitly
/// @brief Thread-safe thumbnail cache
/// @note All public methods are thread-safe
class ThumbnailCache {
public:
 void Put(const std::wstring& key, const Image& image);
 
private:
 mutable std::mutex m_mutex; // Protect shared data
};

// Use RAII for lock management
void Method() {
 std::lock_guard<std::mutex> lock(m_mutex);
 // Critical section
}

// Prefer std::atomic for simple flags
std::atomic<bool> m_running{false};
```

---

## PowerShell Standards

### Script Structure

```powershell
<#
.SYNOPSIS
 Brief one-line description
 
.DESCRIPTION
 Detailed multi-line description of what the script does
 
.PARAMETER Configuration
 Build configuration (Debug or Release)
 
.PARAMETER Clean
 Perform a clean build
 
.EXAMPLE
 .\build.ps1 -Configuration Release
 
.EXAMPLE
 .\build.ps1 -Configuration Debug -Clean
 
.NOTES
 Version: 1.0.0
 Author: ExplorerLens Team
 Last Updated: 2026-02-11
#>

param(
 [Parameter(Mandatory=$false)]
 [ValidateSet("Debug", "Release")]
 [string]$Configuration = "Release",
 
 [switch]$Clean
)

$ErrorActionPreference = "Stop"
$ScriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path

# Script implementation...
```

### Naming Conventions

```powershell
# Scripts: Verb-Noun.ps1 (use approved PowerShell verbs)
Build-Project.ps1
Get-ToolVersion.ps1
Test-Environment.ps1

# Parameters: PascalCase
param(
 [string]$ProjectPath,
 [int]$RetryCount
)

# Variables: camelCase
$buildResult = $true
$errorCount = 0

# Constants: PascalCase or UPPER_CASE
$MaxRetries = 3
$TIMEOUT_SECONDS = 300
```

### Approved Verbs

Use standard PowerShell verbs:
- **Get** - Retrieve data
- **Set** - Modify data
- **New** - Create new resource
- **Remove** - Delete resource
- **Start** - Begin operation
- **Stop** - End operation
- **Build** - Compile source
- **Test** - Verify condition
- **Find** - Locate resource
- **Install** - Add component

```powershell
# Get list of approved verbs
Get-Verb
```

### Error Handling

```powershell
# Set strict error handling
$ErrorActionPreference = "Stop"

# Handle specific errors
try {
 $result = Invoke-Command -ScriptBlock { ... }
} catch [System.IO.FileNotFoundException] {
 Write-Error "File not found: $_"
 return
} catch {
 Write-Error "Unexpected error: $_"
 throw
}

# Check command success
if ($LASTEXITCODE -ne 0) {
 Write-Error "Command failed with exit code: $LASTEXITCODE"
 exit $LASTEXITCODE
}
```

### Output Formatting

```powershell
# Use appropriate Write-* cmdlets
Write-Host "Starting build..." -ForegroundColor Cyan
Write-Verbose "Detailed information"
Write-Warning "This is a warning"
Write-Error "This is an error"

# Use consistent formatting
Write-Host "`n=== Build Started ===" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration" -ForegroundColor Gray
Write-Host "✓ Build successful" -ForegroundColor Green
Write-Host "✗ Build failed" -ForegroundColor Red

# Use structured output for data
$result = @{
 Success = $true
 BuildTime = $elapsed.TotalSeconds
 Warnings = 0
}
return $result
```

---

## Documentation Standards

### Markdown Style

```markdown
# Document Title (H1 - only one per document)

Brief introduction paragraph.

## Section (H2)

Content...

### Subsection (H3)

Content...

#### Sub-subsection (H4 - use sparingly)

Content...

## Lists

- Unordered list item 1
- Unordered list item 2
 - Nested item

1. Ordered list item 1
2. Ordered list item 2

## Code Blocks

Use language-specific fencing:

```cpp
// C++ code
void Function() {
 // ...
}
```

```powershell
# PowerShell code
Get-ChildItem -Path "C:\Temp"
```

## Links

[Relative link to other doc](../path/to/file.md)
[External link](https://example.com)

## Tables

| Column 1 | Column 2 | Column 3 |
|----------|----------|----------|
| Data 1 | Data 2 | Data 3 |
```

### README Files

Every major directory should have a `README.md`:

```markdown
# Directory Name

**Purpose:** Brief description

## Contents

- Overview of directory contents
- Key files/subdirectories

## Usage

How to use this directory

## See Also

- Links to related documentation
```

---

## Git Commit Standards

### Commit Message Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

- **feat:** New feature
- **fix:** Bug fix
- **docs:** Documentation changes
- **style:** Code style changes (formatting, no logic change)
- **refactor:** Code refactoring
- **perf:** Performance improvement
- **test:** Add or modify tests
- **build:** Build system changes
- **ci:** CI/CD changes
- **chore:** Maintenance tasks

### Examples

```
feat(decoder): add JPEG XL format support

Implement JXL decoder using libjxl 0.11.1.
Supports basic decoding and thumbnail generation.

Closes #123
```

```
fix(cache): resolve memory leak in cache eviction

The LRU cache was not properly releasing memory when
evicting old entries. Now uses RAII for cleanup.

Fixes #456
```

```
docs(readme): update build instructions

Add section for building on Windows 11.
Clarify CMake version requirements.
```

### Commit Guidelines

- **Write in imperative mood:** "Add feature" not "Added feature"
- **Keep subject line under 50 characters**
- **Wrap body at 72 characters**
- **Reference issues:** Use "Fixes #123" or "Closes #456"
- **Separate subject from body with blank line**
- **Explain WHAT and WHY, not HOW**

---

## File Organization

### Project Structure

See [PROJECT_STRUCTURE.md](../../PROJECT_STRUCTURE.md) for complete structure.

### File Naming

- **C++ headers:** `PascalCase.h`
- **C++ implementation:** `PascalCase.cpp`
- **PowerShell scripts:** `Verb-Noun.ps1`
- **Markdown docs:** `SHOUTING_CASE.md` or `PascalCase.md`
- **Configuration:** `lowercase.json`, `lowercase.yml`

### Directory Naming

- **Source code:** `PascalCase/` (e.g., `LENSShell/`, `Engine/`)
- **Documentation:** `lowercase/` (e.g., `docs/`, `tests/`)
- **Build scripts:** `lowercase-with-dash/` (e.g., `build-scripts/`)

---

## Code Review Guidelines

### Review Checklist

- [ ] Code follows project style guidelines
- [ ] All functions are documented
- [ ] Thread safety is documented
- [ ] Error handling is appropriate
- [ ] No memory leaks (checked with sanitizers if possible)
- [ ] Tests are included for new features
- [ ] Documentation is updated
- [ ] Commit messages follow standards
- [ ] No debugging code (printf, breakpoints, etc.)
- [ ] Build completes without warnings

### Review Comments

Be constructive and specific:

✅ **Good:**
```
Consider using std::make_unique here for exception safety:
auto decoder = std::make_unique<WebPDecoder>();
```

❌ **Bad:**
```
This code is wrong.
```

### Approval Criteria

- **+1 (Approve):** Code is production-ready
- **Comment:** Suggestions for improvement (non-blocking)
- **Request Changes:** Issues that must be addressed

---

## Tools and Enforcement

### C++ Formatting

Consider using clang-format with this configuration:

```yaml
# .clang-format
BasedOnStyle: LLVM
IndentWidth: 4
ColumnLimit: 120
PointerAlignment: Left
```

### PowerShell Linting

Use `PSScriptAnalyzer`:

```powershell
Install-Module -Name PSScriptAnalyzer
Invoke-ScriptAnalyzer -Path .\script.ps1
```

### Markdown Linting

Use `markdownlint`:

```bash
npm install -g markdownlint-cli
markdownlint *.md
```

---

## Static Analysis & CI Quality Gates

### Clang-Tidy

Configuration: `.clang-tidy` in project root.

```powershell
# Single file
clang-tidy Engine/Core/ThumbnailDecoder.h -- -std=c++20 -I Engine/include

# All engine headers
Get-ChildItem Engine -Recurse -Include *.h,*.cpp | ForEach-Object {
 clang-tidy $_.FullName -- -std=c++20
}
```

**Key checks enabled:**
- `bugprone-*` — Common bug patterns
- `modernize-*` — C++17/20 modernization
- `performance-*` — Performance anti-patterns
- `readability-*` — Code readability
- `cppcoreguidelines-*` — C++ Core Guidelines

### MSVC Code Analysis (CI)

Enabled via `/p:EnableCppCoreCheck=true /p:RunCodeAnalysis=true` in CI builds.

### Header Guard Validation

Automated check that all `.h` files contain `#pragma once`. Use `NOLINT` comments sparingly and with justification:

```cpp
// NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) — COM interface cast required by Windows API
auto* ptr = reinterpret_cast<IStream*>(stream);
```

---

## Questions?

For questions about coding standards:
- Review this document
- Check existing code for examples
- Ask in pull request reviews
- Open GitHub Discussion

---

**Last Updated:** July 2025 
**Version:** v15.0.0 "Zenith" 
**Maintained by:** ExplorerLens Development Team

