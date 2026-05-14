# Benchmark History — `history.jsonl`

This file tracks Engine benchmark results across releases. Each line is a JSON object recording
performance metrics for a single version.

## Schema

Each line in `history.jsonl` is a JSON object with these top-level fields:

| Field | Type | Description |
|-------|------|-------------|
| `schema_version` | `"1"` | Schema version — increment when fields change |
| `version` | `string` | ExplorerLens release version (e.g., `"39.9.0"`) |
| `codename` | `string` | Release codename |
| `date` | `string` | ISO 8601 date when measurement was taken |
| `toolchain` | `string` | Compiler + flags description |
| `machine` | `string` | Reference hardware description |
| `commit` | `string` | Git commit hash (short) |
| `note` | `string` | Optional free-text note |
| `benchmarks` | `object` | Benchmark name → timing object |

### Timing object

Each value in `benchmarks` is an object with any combination of:

| Field | Unit | Description |
|-------|------|-------------|
| `p50_ms` | milliseconds | Median latency |
| `p95_ms` | milliseconds | 95th percentile latency |
| `p99_ms` | milliseconds | 99th percentile latency |
| `throughput_img_sec` | images/second | Batch decode throughput |

## Benchmark names

Benchmark names match those in `Engine/Tests/benchmarks/baseline.json`:

| Name | Category | Description |
|------|----------|-------------|
| `BM_SingleDecode_JPEG` | single_decode | Median JPEG decode for a 6MP image |
| `BM_SingleDecode_PNG` | single_decode | Median PNG decode for a 4K image |
| `BM_SingleDecode_WebP` | single_decode | Median WebP decode |
| `BM_BatchDecode_1000` | batch | 1000-image batch (235 img/sec target) |
| `BM_CacheHit_L1` | cache | L1 LRU cache hit path (< 0.5 ms target) |

## Updating history

After a version bump and performance validation:

```powershell
# Measure with Google Benchmark
.\build\bin\EngineTests.exe --benchmark --benchmark_format=json | `
  Out-File data\benchmarks\raw\$version-raw.json

# Parse and append to history.jsonl
# (script: build-scripts/utilities/Record-BenchmarkHistory.ps1 — Phase 4 implementation)
```

**Automated recording** is planned for the `nightly.yml` workflow (Phase 4, ROADMAP §14).

## Regression detection

`baseline.json` (in `Engine/Tests/benchmarks/`) is the authoritative threshold file consumed by
`performance-regression-gate.yml`. `history.jsonl` is a historical record — it does not gate builds.

The `performance-regression-gate.yml` workflow blocks PRs that regress beyond:
- Single decode latency: 20% block / 10% warn
- Batch throughput: 15% block
- Cache hit latency: 50% block

## ROADMAP reference

- **§14** — Benchmark history + performance tracking system
- **D42** — SQLite L2 cache (which will add L2 hit metrics to this file)
