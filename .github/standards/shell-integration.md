# ExplorerLens — Shell & Build Integration Guide

## Build Execution Strategy

**Problem:** Long-running builds (60-120s compile + 30s LTCG link) can be interrupted
when monitoring tools assume the process is hung.

**Solution:** Always redirect build output to log files and monitor via file reads
instead of relying on terminal output streams.

### Build Commands (Use bat wrappers)

```batch
REM Build with log capture (recommended)
build-scripts\build-and-log.bat build-logs\build-latest.log

REM Test with log capture
build-scripts\test-and-log.bat build-logs\test-latest.log
```

### Monitoring Strategy

1. **Start build in background** — let it run independently
2. **Monitor via log file** — use `Get-Content -Tail 10` or `read_file`
3. **Never send Ctrl-C/Break** to a running build process
4. **If terminal is busy** — open a NEW terminal, don't kill the existing one
5. **Check for stale processes** — `Get-Process ninja,cl -ErrorAction SilentlyContinue`

### Terminal Management

- If a shell is busy, open a new one instead of sending break signals
- Kill a stale process ONLY if it's clearly orphaned (check `StartTime`)
- Remove locked `.obj` files before retrying a build: `Remove-Item build\**\*.obj -Force`
- Use `isBackground: true` for builds, then poll the log file

### Build Times (Approximate)

| Step | Duration |
|------|----------|
| vcvars64 sourcing | ~8s |
| EngineTests.cpp compile | 60-90s |
| LTCG linking | 15-30s |
| Full rebuild | ~120s |
| Incremental (no changes) | <1s |

## File Lock Resolution

When builds fail with "Permission denied" or "Cannot open compiler generated file":

```powershell
# Check for stale processes
Get-Process ninja,cl,link -ErrorAction SilentlyContinue | Format-Table Id,Name,StartTime

# Kill stale ninja (only if orphaned from a previous session)
Stop-Process -Name ninja -Force -ErrorAction SilentlyContinue

# Clean locked object files
Remove-Item "build\Engine\Tests\CMakeFiles\EngineTests.dir\*.obj" -Force -ErrorAction SilentlyContinue

# Retry build
build-scripts\build-and-log.bat build-logs\build-latest.log
```

## VS Code Terminal Configuration

Terminal profiles are configured in `.vscode/settings.json`:

- **PowerShell 7** — Default shell (`pwsh -NoProfile -NoLogo`)
- **Developer PowerShell (vcvars64)** — Pre-sources MSVC v145 environment
- **PowerShell (Legacy)** — Windows PowerShell fallback

Settings:
- `confirmOnKill: "always"` — Prevents accidental terminal kills
- `scrollback: 50000` — Large buffer for build output
- `shellIntegration: true` — Command detection enabled
