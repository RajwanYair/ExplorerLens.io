# Sprint 144 — Diagnostics Export Finalization

**Status:** Complete  
**Phase:** N4 — Reliability & Hardening  
**Component:** `Engine/Core/DiagnosticsExporter.h`

## Objective
One-click ZIP bundle export of support diagnostics including system info, decoder health, ETW logs, configuration, and error logs with anonymization and size limits.

## Deliverables
- `Engine/Core/DiagnosticsExporter.h` — Header-only diagnostics exporter
- `tests/Sprint144_DiagnosticsExporter.cpp` — 12 GTest cases

## Key Features
- **DiagCategory** — 10 diagnostic categories (SystemInfo through PluginStatus)
- **DiagEntry** — Category-tagged content with sensitivity flag
- **DiagExportConfig** — Default/Minimal/Full presets, per-category toggles, size limits, path anonymization
- **ExportResult** — Status, file count, total size, skipped tracking
- **DiagnosticsExporter** — Entry collection, category-based filtering, path anonymization, size-limited export
