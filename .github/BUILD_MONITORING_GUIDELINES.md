# Build Monitoring Guidelines for Slow Machines

## Problem Statement

On slow machines, AI assistants may interrupt build processes by using the same shell for both execution and monitoring. This leads to:
- Build processes being killed with Ctrl-C
- `Start-Sleep` commands interrupting compilation
- Incomplete builds that appear to succeed
- Wasted time re-running builds

## Best Practices

### ✅ DO: Separate Execution from Monitoring

**Execute builds in background:**
```powershell
# Start build in background - DO NOT WAIT IN SAME SHELL
Start-Process pwsh -ArgumentList "-NoProfile", "-NoLogo", "-File", "build-script.ps1" -NoNewWindow
```

**Monitor using VS Code file system tools:**
- Use `list_dir` to check for output files
- Use `file_search` to find build artifacts
- Read build log files from disk
- Check file timestamps to detect changes

### ❌ DON'T: Mix Execution and Monitoring

**WRONG - Using same shell:**
```powershell
# BAD: Running build
& msbuild project.sln /t:Rebuild

# BAD: Interrupting with Ctrl-C then checking
Start-Sleep -Seconds 30
Get-ChildItem output\  # May interrupt the build!
```

## Correct Workflow

### 1. Start Build Process
```powershell
# Option A: Background task
$msbuild = "path\to\MSBuild.exe"
Start-Process $msbuild -ArgumentList "project.sln", "/t:Rebuild", "/p:Configuration=Release" -NoNewWindow -PassThru

# Option B: Use isBackground=true in run_in_terminal
# This returns immediately without blocking
```

### 2. Monitor Using VS Code File System

**Check for output files:**
```python
# Use list_dir tool to check build output
list_dir("C:\path\to\project\x64\Release")

# Check specific file existence
file_search("**/CBXShell.dll")
```

**Read build logs:**
```python
# Read log files written by build process
read_file("build-logs\build-progress.json")
read_file("x64\Release\BuildLog.txt")
```

**Monitor file timestamps:**
```python
# Check last modified time to see if build is progressing
list_dir("x64\Release")  # Shows LastWriteTime for each file
```

### 3. Wait for Completion

**Use file timestamp polling:**
```powershell
# Separate monitoring script (if needed)
$outputFile = "x64\Release\CBXShell.dll"
while (-not (Test-Path $outputFile)) {
    Start-Sleep -Seconds 5
}
# File exists - build complete
```

**Or use VS Code's file watcher:**
- Simply check periodically using list_dir
- No active waiting in build shell

## Build Log Monitoring

### Create Progress Logs

**Build scripts should write progress:**
```powershell
# In build script
"Starting build at $(Get-Date)" | Out-File "build-logs\progress.log"
# ... build steps ...
"Build complete at $(Get-Date)" | Out-File "build-logs\progress.log" -Append
```

**Monitor logs from VS Code:**
```python
# Read log file periodically
read_file("build-logs\progress.log", 1, 100)
```

## Examples

### ✅ Correct: Background Build with File Monitoring

```python
# 1. Start build in background
run_in_terminal(
    command="& 'C:\...\MSBuild.exe' CBXShell.sln /t:Rebuild /p:Configuration=Release",
    isBackground=True,  # Returns immediately
    explanation="Starting full rebuild in background"
)

# 2. Wait a bit for build to start
# (Don't use Start-Sleep in build shell!)

# 3. Monitor using file system
list_dir("CBXShell\x64\Release")  # Check for .obj files (compilation phase)

# 4. Later, check for final output
list_dir("CBXShell\x64\Release")  # Check for .dll (linking phase)
```

### ❌ Wrong: Same Shell for Both

```python
# DON'T DO THIS:
run_in_terminal(
    command="& msbuild ...; Start-Sleep -Seconds 30; Get-ChildItem output",
    isBackground=False
)
# This blocks, then may timeout or interrupt the build
```

## MSBuild Specific

### Parallel Builds
```powershell
# Use /maxcpucount for parallel builds on slow machines
msbuild project.sln /t:Rebuild /m:2 /p:Configuration=Release
```

### Log Files
```powershell
# Always create log files for monitoring
msbuild project.sln /t:Rebuild /fl /flp:LogFile=build-logs\msbuild.log;Verbosity=minimal
```

### Monitor via Task Manager
- Check for `MSBuild.exe` and `cl.exe` processes
- Don't interrupt them from the terminal!

## VS Code Integration

### Use Output Channels
- Build scripts can write to files
- Read those files from VS Code perspective
- Don't rely on terminal output for slow builds

### Use Tasks
- VS Code tasks can run in background
- Monitor their output via task output channel
- Don't mix with interactive shell commands

## Summary

**Key Principle:** 
> **Execute in shell. Monitor from VS Code file system. Never mix the two.**

**Remember:**
- Slow machines need time to complete builds
- Don't interrupt processes by checking status in same shell
- Use file system tools (list_dir, file_search, read_file) for monitoring
- Build logs are your friend
- Patience is a virtue on slow machines

---

**Date Created:** January 7, 2026  
**Last Updated:** January 7, 2026  
**Status:** Active Standard Practice
