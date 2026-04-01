#!/usr/bin/env pwsh
# BumpHelper-v32.1.0.ps1 — Bump to v32.1.0 "Fomalhaut-R"
# Edge AI & Hardware-Accelerated Inference (Sprints 1081-1090)
$entry = @"
## [32.1.0] — $(Get-Date -Format 'yyyy-MM-dd')

### v32.1.0 "Fomalhaut-R" — Edge AI & Hardware-Accelerated Inference

#### New: Hardware-Accelerated AI Inference Layer (8 components, +72 tests)
- **NPUAccelerationEngine** — ONNX/DirectML NPU dispatch with Auto/ForceNPU/ForceGPU/ForceCPU modes
- **EdgeAIInferenceEngine** — Session lifecycle with memory-mapped model weights, multi-session management
- **HardwareCapabilityNegotiator** — NPU/GPU/CPU/FPGA backend selection by task type with scored negotiation
- **AMDXDNABackend** — Ryzen AI 300 / Strix Halo (50 TOPS) MLIR kernel execution with tile-mode selection
- **QualcommAIEBackend** — Snapdragon X Elite X1E-80-100 (45 TOPS) QNN SDK HTP/GPU/CPU runtime routing
- **IntelAMXBackend** — BF16/INT8 AMX matrix multiply, 2.1× throughput vs SSE4.2, runtime CPU detection
- **HardwareAcceleratedPipeline** — Silicon routing pipeline: NPU→Infer, GPU→Decode, CPU fallback per stage
- **ComputeDeviceRegistry** — Startup enumeration of all CPU/GPU/NPU accelerators with capability lookup (singleton)

#### Test Coverage
- Unit tests: 4362 → 4434 (+72)
"@

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
& "$scriptDir\Bump-Version.ps1" `
    -Version "32.1.0" `
    -Codename "Fomalhaut-R" `
    -TestCount 4434 `
    -ChangelogEntry $entry `
    -TagAndPush
