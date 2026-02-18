# Sprint 142 — Per-Monitor DPI V2

**Status:** Complete  
**Phase:** N5 — GUI & Manager Polish  
**Component:** `Engine/Utils/PerMonitorDPIManager.h`

## Objective
Handle DPI changes mid-session with per-monitor DPI V2 awareness, real-time control rescaling, font clamping, and callback notification system.

## Deliverables
- `Engine/Utils/PerMonitorDPIManager.h` — Header-only per-monitor DPI manager
- `tests/Sprint142_PerMonitorDPIManager.cpp` — 13 GTest cases

## Key Features
- **DPIScale** — 7 standard tiers (100%–300%) with factor/name helpers
- **LayoutRect** — Control rectangle with proportional scaling
- **ScaledFont** — Font scaling with min/max clamping
- **DPIChangeEvent** — Old/new DPI, scale ratio, up/down detection
- **MonitorInfo** — Per-monitor handle, DPI, work area, primary flag
- **DPIConfig** — Per-monitor V2, font/icon scaling, tier snapping
- **PerMonitorDPIManager** — Monitor registry, DPI change handling, rect/font scaling, change history, callback system
