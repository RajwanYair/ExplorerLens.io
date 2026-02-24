# AI Assistant Build Instructions

**For:** GitHub Copilot, AI coding assistants  
**Project:** ExplorerLens  
**Environment:** Persistent dev tools (no searching!)  
**Last Updated:** February 9, 2026

---

## 🎯 NEW: Zero-Configuration Environment (Feb 2026)

**All build tools are now available immediately in every PowerShell session!**

No more searching for MSBuild, CMake, or MSVC. Everything is pre-configured.

### How It Works

1. **Setup script auto-loads** in PowerShell profile
2. **MSVC environment loaded** on demand (CL, NMake, Link, RC)
3. **All tools verified** before first use
4. **Convenient aliases** ready to use (dtbuild, dttest, dtclean)

### Quick Verification

```powershell
# Check environment is loaded:
Show-ExplorerLensInfo

# Test all tools:
Test-BuildTools
```

**Expected Output:**
```
✅ MSBuild (18.3.0)
✅ CMake (4.2.1)
✅ Git (2.x.x)
✅ MSVC (CL) (19.44.x)
✅ NMake
✅ Link
✅ RC
✅ Ninja (1.x.x)
```

---

## � Recommended Build Approach (2026)

### Use Convenient Aliases

```powershell
# Build full solution (Release)
dtbuild Release

# Build Engine only
dtbuild Engine

# Clean artifacts
dtbuild Clean

# Run tests
dttest
```

These commands handle all complexity internally. No monitoring needed!

---

## 🚨 CRITICAL RULES - READ FIRST (Legacy/Manual Builds)

### Rule #1: NEVER Interrupt Running Builds

**You MUST NOT:**
- Use Ctrl-C to interrupt any `msbuild` or compilation command
- Run monitoring commands in the same shell as a build
- Assume build is done when command prompt returns
- Use short sleep delays (< 30 seconds) for large C++ projects

### Rule #2: Use File System for Monitoring

**You MUST:**
- Monitor builds by checking files with `list_dir`, `file_search`, `read_file`
- Wait adequate time (60+ seconds) before checking build results
- Check file timestamps to verify recent compilation
- Read log files from disk, not terminal output

### Rule #3: Background Execution Only

**All builds MUST use one of these patterns:**

```powershell
# Pattern 1: Background with logging (PREFERRED)
msbuild Project.vcxproj /p:Configuration=Release /fl /flp:LogFile=build.log

# DO NOT run any commands for 60+ seconds
# Then use file_search or list_dir to check outputs

# Pattern 2: Separate terminal monitoring
# Terminal 1: Start build with logging
msbuild Project.vcxproj /fl /flp:LogFile=build.log

# Terminal 2: Monitor log file (separate terminal ID)
.\build-scripts\Monitor-Build-Safe.ps1 -LogFile build.log
```

---

## 📋 Standard Build Workflow

### Step 1: Start Build (Terminal Command)

```powershell
# Use run_in_terminal with isBackground=true or wait 60+ seconds
msbuild LENSShell.vcxproj /p:Configuration=Release /p:Platform=x64 /fl /flp:LogFile=LENSShell-build.log
```

### Step 2: Wait (Do Nothing)

```powershell
# Wait adequate time based on project:
# - Engine library: 45 seconds minimum
# - LENSShell DLL: 60 seconds minimum  
# - Full solution: 180 seconds (3 minutes) minimum

# DO NOT run any commands during this time
```

### Step 3: Check Results (Use File System Tools)

```powershell
# Use list_dir tool
list_dir("c:\\Users\\ryair\\...\\LENSShell\\x64\\Release")

# Use file_search tool
file_search("LENSShell/x64/Release/*.dll")

# Check log file with read_file tool
read_file("LENSShell-build.log", startLine=1, endLine=50)
```

### Step 4: Verify Success

```powershell
# Check if DLL exists and is recent (< 5 minutes old)
if (Test-Path 'LENSShell\x64\Release\LENSShell.dll') {
    $dll = Get-Item 'LENSShell\x64\Release\LENSShell.dll'
    $age = (Get-Date) - $dll.LastWriteTime
    if ($age.TotalMinutes -lt 5) {
        Write-Host "✓ Build successful"
    }
}
```

---

## 🔧 Available Helper Scripts

### Monitor-Build-Safe.ps1

Monitors build log file without interfering with build process.

```powershell
# Usage (in separate terminal):
.\build-scripts\Monitor-Build-Safe.ps1 -LogFile build.log
```

### Verify-Build-Output.ps1

Checks all expected output files exist and are recent.

```powershell
# Usage (after build completes):
.\build-scripts\Verify-Build-Output.ps1
.\build-scripts\Verify-Build-Output.ps1 -Detailed
```

---

## 📊 Expected Build Times (Slow Machine)

| Project | Configuration | Minimum Wait | Safe Wait |
|---------|--------------|--------------|-----------|
| ExplorerLensEngine.lib | Release x64 | 30 sec | 60 sec |
| LENSShell.dll | Release x64 | 45 sec | 90 sec |
| LENSManager.exe | Release x64 | 15 sec | 30 sec |
| Full Solution | Release x64 | 180 sec (3 min) | 300 sec (5 min) |

**Always use "Safe Wait" time to be certain build completes.**

---

## ✅ Correct Example Workflow

```powershell
# AI Assistant Action 1: Start build
# Tool: run_in_terminal
# Command: msbuild Engine/ExplorerLensEngine.vcxproj /p:Configuration=Release /p:Platform=x64 /fl /flp:LogFile=engine-build.log
# isBackground: true (or wait without interruption)

# AI Assistant Action 2: Wait (do nothing for 60 seconds)
# No tool calls

# AI Assistant Action 3: Check build output via file system
# Tool: list_dir
# Path: c:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\ExplorerLens\Engine\Release

# AI Assistant Action 4: Verify file exists
# Tool: file_search
# Query: Engine/Release/ExplorerLensEngine.lib

# AI Assistant Action 5: Read log for status
# Tool: read_file
# File: engine-build.log
# Lines: -50 to -1 (last 50 lines)

# AI Assistant Action 6: Report success or failure
# Parse log content, report to user
```

---

## ❌ Incorrect Example (DO NOT DO THIS)

```powershell
# ❌ WRONG - This is what causes failures:

# Action 1: Start build
msbuild Engine/ExplorerLensEngine.vcxproj

# Action 2: Interrupt after 5 seconds
^C  # <-- THIS KILLS THE BUILD

# Action 3: Try to wait for interrupted build
Start-Sleep -Seconds 10  # <-- BUILD IS ALREADY DEAD

# Action 4: Check for output that was never created
Get-Item Engine\Release\ExplorerLensEngine.lib  # <-- FILE DOESN'T EXIST

# Result: Build failed, time wasted
```

---

## 🎯 Decision Tree for AI Assistants

```
User asks to build project
│
├─ Is this a full rebuild?
│  ├─ Yes → Use 180+ second wait time
│  └─ No → Continue
│
├─ Is this Engine library?
│  ├─ Yes → Use 60+ second wait time
│  └─ No → Continue
│
├─ Is this LENSShell DLL?
│  ├─ Yes → Use 90+ second wait time
│  └─ No → Use 30+ second wait time
│
├─ Start build with logging:
│  └─ msbuild <project> /fl /flp:LogFile=build.log
│
├─ WAIT (do not run any commands)
│  └─ Wait time based on project type
│
├─ Check results using FILE SYSTEM tools:
│  ├─ list_dir to see output directory
│  ├─ file_search to find specific files
│  └─ read_file to check log file
│
└─ Report results to user
   ├─ Success → Show file info (size, timestamp)
   └─ Failure → Show errors from log file
```

---

## 🛠️ Tools You Can Use

### Allowed (Safe for Slow Machines)

- ✅ `list_dir` - Check directory contents
- ✅ `file_search` - Find files by pattern
- ✅ `read_file` - Read log files
- ✅ `run_in_terminal` with `isBackground: true`
- ✅ `run_in_terminal` with adequate wait time (60+ sec)

### Restricted (Use Carefully)

- ⚠️ `run_in_terminal` with short waits - Must wait 60+ seconds
- ⚠️ `get_terminal_output` - Build may not be done yet

### Forbidden (Never Use During Builds)

- ❌ Ctrl-C / `^C` to interrupt builds
- ❌ Running multiple commands in same terminal during build
- ❌ Checking output immediately after starting build
- ❌ Using `Start-Sleep` in same shell as build

---

## 📝 Template Response

When user asks to build:

```
I'll build [project] now. This will take approximately [X] seconds on your machine.

[Start build with run_in_terminal, isBackground=true]

The build has started with logging to [logfile]. I'll check the results in [X] seconds using the file system.

[Wait adequate time - no actions]

[Use list_dir to check output directory]

[Use file_search to verify output file exists]

[Use read_file to check log file for errors/success]

✓ Build completed successfully! Output: [file path and size]
  (or)
✗ Build failed. Errors: [show errors from log]
```

---

## 🔍 Debugging Build Issues

If build appears to fail:

1. ✅ Use `read_file` to check last 50 lines of log
2. ✅ Use `grep_search` to find error messages in log
3. ✅ Use `list_dir` to see if partial outputs exist
4. ✅ Check file timestamps to verify build ran

Do NOT:
- ❌ Restart build immediately
- ❌ Run diagnostic commands in same terminal
- ❌ Assume fast failure means build error

---

## 📚 Related Documentation

- [Build Monitoring Guidelines](BUILD_MONITORING_GUIDELINES.md) - Detailed methodology
- [build-scripts/Monitor-Build-Safe.ps1](../build-scripts/Monitor-Build-Safe.ps1) - Log monitoring script
- [build-scripts/Verify-Build-Output.ps1](../build-scripts/Verify-Build-Output.ps1) - Output verification script

---

**Remember: Patience is required for slow machines. Wait for builds to complete!**

**Last updated:** January 8, 2026  
**Review:** Required before any build operation

