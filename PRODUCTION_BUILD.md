# Production Build Guide

## ✅ Build Status: CLEAN (Zero Warnings, Zero Errors)

Last verified: January 7, 2026

## Production/Release Requirements

**Release builds must be absolutely clean:**
- ✅ 0 Warnings
- ✅ 0 Errors
- ✅ No workarounds or ignored warnings
- ✅ Proper runtime library linkage

**Debug builds acceptance criteria:**
- Warnings are acceptable for Debug configuration
- Focus is on functionality, not production quality
- Runtime conflicts with external libraries are expected (using Release libs in Debug)

## Quick Build

### Using VS Code (Recommended)

1. Press `Ctrl+Shift+B` (Run Build Task)
2. Select "Build Release (Production - Zero Warnings)"
3. VS Code will show errors/warnings in the Problems panel with `$msCompile` problem matcher

### Using Command Line

```powershell
# Clean build (recommended for production)
msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /t:Rebuild /m

# Incremental build
msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /m
```

**Expected output:**
```
Build succeeded.
    0 Warning(s)
    0 Error(s)
Time Elapsed 00:00:XX.XX
```

## Build Configuration

### Release Configuration (Production)
- **Runtime Library**: `/MD` (Multi-threaded DLL)
- **Optimization**: Full (`/O2`, `/Ot`, `/Oy`, `/GL`, `/LTCG`)
- **CPU Extensions**: AVX2, Spectre mitigation
- **Security**: Control Flow Guard, Buffer Security Check
- **Standard**: C++20, C17
- **Output**: `x64\Release\CBXShell.dll` (~828 KB)

### External Library Requirements
- **zlib 1.3.1**: Built with `/MT` → Ignored via `/NODEFAULTLIB:LIBCMT`
- **lz4 1.10.0**: Built with `/MT` → Ignored via `/NODEFAULTLIB:LIBCMT`
- **zstd 1.5.7**: Built with `/MD` ✅ (matches CBXShell)
- **minizip-ng 4.0.10**: Built with `/MD` ✅ (matches CBXShell)
- **libwebp 1.5.0**: Built with `/MT` → Ignored via `/NODEFAULTLIB:LIBCMT`

## Complete Build Process

### Step 1: Build External Libraries

```powershell
# Navigate to each library directory and build
cd external\compression\minizip-ng-4.0.10
.\build-minizip-manual.ps1  # Builds with /MD runtime
cd ..\..\..
```

**Or use the automated script:**
```powershell
.\Build-Production-SlowMachine.ps1
```

### Step 2: Build CBXShell

```powershell
# Clean rebuild (production)
msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /t:Rebuild /m
```

### Step 3: Verify Zero Warnings

Check the build output - it **MUST** show:
```
Build succeeded.
    0 Warning(s)
    0 Error(s)
```

If you see **ANY warnings**, investigate and fix them. Do not use `/NODEFAULTLIB` or other workarounds without proper justification.

## VS Code Integration

### Build Tasks
- **Build Release (Production - Zero Warnings)**: Incremental build with VS Code problem matcher
- **Rebuild Release (Clean Build)**: Full clean rebuild
- **Build with VS Code Monitoring**: For long-running external library builds

### Problem Matchers
All production build tasks use `$msCompile` problem matcher:
- Errors appear in VS Code Problems panel
- Click to jump to source location
- No need for PowerShell-level monitoring

## Runtime Library Strategy

### Why We Use `/MD` for Release

1. **External Libraries**: minizip-ng and zstd are built with `/MD` (MSVCRT.dll)
2. **Smaller Static Sections**: Reduces DLL size
3. **Standard Practice**: Most modern Windows applications use DLL runtime
4. **Compatibility**: Matches Windows SDK and most libraries

### Handling `/MT` Libraries

Some libraries (zlib, lz4, libwebp) are built with `/MT` (static runtime). We handle this by:

```xml
<IgnoreSpecificDefaultLibraries>LIBCMT</IgnoreSpecificDefaultLibraries>
```

This tells the linker to ignore the static runtime library when linking against `/MT` libraries. The `/MD` runtime (MSVCRT.dll) provides all necessary functions.

### Debug Configuration

Debug uses `/MTd` (static debug runtime) which conflicts with Release-built external libraries. This is **acceptable** because:
- Debug is not for production deployment
- Warnings are allowed in Debug
- Rebuilding all external libraries with `/MTd` is unnecessary overhead

## Troubleshooting

### Warning LNK4098: defaultlib 'LIBCMT' conflicts

**Solution**: Already handled via `<IgnoreSpecificDefaultLibraries>LIBCMT</IgnoreSpecificDefaultLibraries>`

If this warning appears, verify:
1. Release configuration uses `/MD` runtime
2. `IgnoreSpecificDefaultLibraries` includes `LIBCMT`

### MIDL1011: missing arguments for /tlb

**Cause**: Code formatter breaks MIDL command across lines

**Solution**: Keep MIDL command on single line in `CBXShell.vcxproj`:
```xml
<Command>midl.exe /nologo /char signed /env x64 /Oicf /h "CBXShell.h" /iid "CBXShell_i.c" /tlb ".\CBXShell.tlb" /target "NT60" "%(FullPath)"</Command>
```

### Missing minizip.lib

**Cause**: Build directory cleaned or library not built

**Solution**:
```powershell
cd external\compression\minizip-ng-4.0.10
.\build-minizip-manual.ps1
```

## Build Verification Checklist

Before committing or releasing:

- [ ] Clean rebuild completes successfully
- [ ] Build output shows **0 Warnings, 0 Errors**
- [ ] `x64\Release\CBXShell.dll` is created (~828 KB)
- [ ] `x64\Release\CBXManager.exe` is created (~293 KB)
- [ ] No `/NODEFAULTLIB` workarounds except for documented LIBCMT case
- [ ] All external libraries present in expected locations
- [ ] Git status clean (no uncommitted build artifacts)

## Continuous Integration

For CI/CD pipelines:

```powershell
# Full clean build with error checking
msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /t:Rebuild /m /v:minimal
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed"
    exit 1
}

# Verify zero warnings (parse output)
$output = msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /m 2>&1
if ($output -match "(\d+) Warning\(s\)") {
    $warnings = [int]$matches[1]
    if ($warnings -gt 0) {
        Write-Error "Build has $warnings warning(s) - Production builds must be clean!"
        exit 1
    }
}
```

## Additional Resources

- [Build Scripts](build-scripts/): Automated build scripts for external libraries
- [External Libraries](external/): Source code for compression and image libraries
- [.vscode/tasks.json](.vscode/tasks.json): VS Code build tasks configuration
- [CBXShell.vcxproj](CBXShell/CBXShell.vcxproj): MSBuild project configuration

## Notes

- **MIDL Command**: Must remain on single line (formatter breaks it)
- **Runtime Library**: Release uses `/MD`, Debug uses `/MTd`
- **External Libraries**: Only Release builds required
- **Build Time**: ~30-90 seconds depending on machine
- **Warnings**: Absolutely not acceptable in Release builds
