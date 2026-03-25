$ErrorActionPreference = "Stop"
$repo = "c:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\ExplorerLens.io"
Set-Location $repo

git add -A
git commit -m "feat: v15.3.0 Zenith-T — Resilience & Hardening (Sprints 9-16 of 100)

Objective: Edge-case resilience, structured error taxonomy, per-decoder timeout,
crash dump capture, graceful degradation catalog, archive security hardening,
fuzz harness scaffold.

Impacted areas:
- Engine/Core/DecodeInputValidator.h    (Sprint 9) — file-size/dimension/bit-depth guards
- Engine/Core/DecodeErrorCategory.h     (Sprint 10) — 24-value structured error enum
- Engine/Core/DecoderTimeoutGuard.h/.cpp(Sprint 11) — 5s per-decoder watchdog
- LENSShell/CrashDumpCapture.h/.cpp     (Sprint 12) — SEH MiniDump to %TEMP%
- Engine/Core/GracefulDegradation.h     (Sprint 13) — 6 failure mode catalog
- Engine/Core/ArchiveSecurityValidator.h(Sprint 14) — ZIP bomb/path-traversal/symlink
- Engine/Tests/FuzzTargets/Fuzz*.cpp    (Sprint 15) — LibFuzzer harness scaffold
- Engine/CMakeLists.txt                 — new headers + DecoderTimeoutGuard.cpp registered
- Engine/Tests/EngineTests.cpp          — 13 new tests (total: 2,951)
- Engine/Core/BuildValidation.h         — v15.3.0, Zenith-T, UnitTestCount=1255
- VERSION, README, CHANGELOG, docs      — all updated to 15.3.0"

git tag v15.3.0
git push origin main --tags

Write-Host "SUCCESS: v15.3.0 committed, tagged, and pushed" -ForegroundColor Green
