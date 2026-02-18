# Sprint 143 — Decoder Health Dashboard

**Status:** Complete  
**Phase:** N5 — GUI & Manager Polish  
**Component:** `Engine/Core/DecoderHealthDashboard.h`

## Objective
Implement a real-time decoder health monitoring data model with circuit breaker state tracking, aggregate statistics, and dashboard-ready output.

## Deliverables
- `Engine/Core/DecoderHealthDashboard.h` — Header-only health dashboard
- `tests/Sprint143_DecoderHealthDashboard.cpp` — 13 GTest cases

## Key Features
- **CircuitState** — Closed/Open/HalfOpen circuit breaker pattern
- **HealthStatus** — Healthy/Degraded/Unhealthy/Disabled/Unknown
- **DecoderMetrics** — Success rate, average time, peak time, peak memory
- **DecoderHealthEntry** — Per-decoder health, circuit state, metrics, consecutive failures
- **DashboardConfig** — Refresh interval, failure threshold, degraded/unhealthy thresholds
- **DashboardStats** — Aggregate counts, overall success rate, health percentage
- **DecoderHealthDashboard** — Registration, decode recording, auto-health computation, circuit breaker trips, half-open recovery, forced disable
