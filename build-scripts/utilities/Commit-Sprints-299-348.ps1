# Sprint 299-348 Git Commit Script
# Run from workspace root
$root = "c:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\ExplorerLens"
Set-Location $root

function Commit-Sprint {
    param([int]$n, [string]$header, [string]$msg)
    $doc = "docs/development/sprints-v8/SPRINT_${n}.md"
    git add $header $doc 2>&1 | Out-Null
    git commit -m "Sprint ${n}: ${msg}" 2>&1
    Write-Host "Sprint $n committed: $msg" -ForegroundColor Green
}

# Phase 1: GPU Pipeline V3
Commit-Sprint 299 "Engine/Core/VersionSyncV14.h"      "Version Sync V14 — v14.0 Apex architecture baseline and version bootstrap"
Commit-Sprint 300 "Engine/Core/GPUPipelineV3.h"        "GPU Pipeline V3 — DirectX 12 Ultimate mesh shaders, DXR, and VRS"
Commit-Sprint 301 "Engine/Core/ShaderCompilerV2.h"     "Shader Compiler V2 — SM6.7 DXIL compilation and reflection pipeline"
Commit-Sprint 302 "Engine/Cache/PipelineStateCacheV2.h" "Pipeline State Cache V2 — PSO disk serialization and warm-up"
Commit-Sprint 303 "Engine/Core/GPUMemoryPoolV2.h"       "GPU Memory Pool V2 — D3D12 residency management and heap allocation"
Commit-Sprint 304 "Engine/Utils/ReleaseGateV23.h"       "Release Gate V23 — Phase 1 GPU Pipeline V3 gate (12 KPIs)"

# Phase 2: Format Intelligence
Commit-Sprint 305 "Engine/Pipeline/SmartFormatDetectorV2.h" "Smart Format Detector V2 — multi-signal heuristic format detection"
Commit-Sprint 306 "Engine/Decoders/ExtendedVideoDecoder.h"  "Extended Video Decoder — HEVC/VP9/AV1/ProRes hardware-accel decode"
Commit-Sprint 307 "Engine/Decoders/AudioVisualizationV2.h"  "Audio Visualization V2 — LUFS metering and GPU waveform rendering"
Commit-Sprint 308 "Engine/Decoders/Model3DRendererV2.h"     "3D Model Renderer V2 — multi-format 3D thumbnail with PBR lighting"
Commit-Sprint 309 "Engine/Utils/ReleaseGateV24.h"           "Release Gate V24 — Phase 2 Format Intelligence gate (12 KPIs)"

# Phase 3: Plugin Ecosystem V2
Commit-Sprint 310 "Engine/Plugin/PluginSDKV2.h"                   "Plugin SDK V2 — extended API with 9 capabilities and host services"
Commit-Sprint 311 "Engine/Plugin/PluginDebuggerIntegration.h"     "Plugin Debugger Integration — in-process debug event bridge"
Commit-Sprint 312 "Engine/Plugin/PluginHotReload.h"               "Plugin Hot Reload — live DLL shadow-copy hot reload"
Commit-Sprint 313 "Engine/Plugin/PluginPerformanceProfiler.h"     "Plugin Performance Profiler — lock-free ring-buffer perf sampling"
Commit-Sprint 314 "Engine/Utils/ReleaseGateV25.h"                 "Release Gate V25 — Phase 3 Plugin Ecosystem V2 gate (11 KPIs)"

# Phase 4: Security Hardening V2
Commit-Sprint 315 "Engine/Utils/ThreatModelV2.h"              "Threat Model V2 — STRIDE threat registry with CVSS scoring"
Commit-Sprint 316 "Engine/Utils/MemorySafetyAuditV2.h"        "Memory Safety Audit V2 — ASan/Valgrind heap safety integration"
Commit-Sprint 317 "Engine/Utils/SupplyChainIntegrityV2.h"     "Supply Chain Integrity V2 — SBOM and CVE vulnerability scanning"
Commit-Sprint 318 "Engine/Core/RuntimeIntegrityVerifier.h"    "Runtime Integrity Verifier — PE hash and tamper-indicator detection"
Commit-Sprint 319 "Engine/Utils/ReleaseGateV26.h"             "Release Gate V26 — Phase 4 Security Hardening V2 gate (11 KPIs)"

# Phase 5: UX Polish V2
Commit-Sprint 320 "Engine/Core/ProgressiveThumbnailLoader.h"     "Progressive Thumbnail Loader — multi-stage placeholder → full decode"
Commit-Sprint 321 "Engine/Core/ThumbnailAnimationEngineV2.h"     "Thumbnail Animation Engine V2 — GIF/APNG hover-playback"
Commit-Sprint 322 "Engine/Core/PreviewPanelV2.h"                  "Preview Panel V2 — histogram, color picker, hex dump tabs"
Commit-Sprint 323 "Engine/Core/QuickLookIntegration.h"            "Quick Look Integration — Space-bar instant preview panel"
Commit-Sprint 324 "Engine/Utils/ReleaseGateV27.h"                 "Release Gate V27 — Phase 5 UX Polish V2 gate (11 KPIs)"

# Phase 6: AI Intelligence V2
Commit-Sprint 325 "Engine/AI/SceneUnderstandingEngine.h"  "Scene Understanding Engine — ML scene classification for smart crop"
Commit-Sprint 326 "Engine/AI/SmartCropV2.h"               "Smart Crop V2 — AI saliency-map auto-crop with 6 strategies"
Commit-Sprint 327 "Engine/AI/ImageQualityAssessor.h"      "Image Quality Assessor — BRISQUE/NIQE IQA with defect grading"
Commit-Sprint 328 "Engine/AI/AISearchIntegration.h"       "AI Search Integration — CLIP embedding semantic thumbnail search"
Commit-Sprint 329 "Engine/Utils/ReleaseGateV28.h"         "Release Gate V28 — Phase 6 AI Intelligence V2 gate (11 KPIs)"

# Phase 7: Enterprise & Cloud V2
Commit-Sprint 330 "Engine/Utils/EnterprisePolicyEngineV2.h" "Enterprise Policy Engine V2 — GPO and MDM policy enforcement"
Commit-Sprint 331 "Engine/Core/SharePointTeamsIntegration.h" "SharePoint & Teams Integration — Microsoft 365 Graph cloud thumbs"
Commit-Sprint 332 "Engine/Cache/MultiTenantCacheManager.h"   "Multi-Tenant Cache Manager — per-tenant namespace isolation"
Commit-Sprint 333 "Engine/Utils/ComplianceAuditLogger.h"     "Compliance Audit Logger — GDPR/HIPAA/SOX immutable audit trail"
Commit-Sprint 334 "Engine/Utils/ReleaseGateV29.h"            "Release Gate V29 — Phase 7 Enterprise & Cloud V2 gate (11 KPIs)"

# Phase 8: Platform Modernization
Commit-Sprint 335 "Engine/Utils/Windows12Compatibility.h"      "Windows 12 Compatibility — Win12 feature detection and compat layer"
Commit-Sprint 336 "Engine/Utils/ARM64PerformanceOptimizer.h"   "ARM64 Performance Optimizer — NEON/SVE SIMD and big.LITTLE tuning"
Commit-Sprint 337 "Engine/Core/WinRTAppSDKIntegrationV2.h"    "WinRT App SDK Integration V2 — MSIX activation and WinRT streams"
Commit-Sprint 338 "Engine/Utils/InstallerV2Manager.h"          "Installer V2 Manager — MSI/MSIX/NSIS multi-format atomic installer"
Commit-Sprint 339 "Engine/Utils/ReleaseGateV30.h"              "Release Gate V30 — Phase 8 Platform Modernization gate (10 KPIs)"

# Phase 9: Performance Excellence
Commit-Sprint 340 "Engine/Cache/SubMillisecondCacheEngine.h"        "Sub-Millisecond Cache Engine — NUMA lock-free sub-1ms P99 cache"
Commit-Sprint 341 "Engine/GPU/GPUDecodeAccelerationV2.h"            "GPU Decode Acceleration V2 — NVDEC/QuickSync/AMF hardware decode"
Commit-Sprint 342 "Engine/Pipeline/ParallelIOPipeline.h"            "Parallel I/O Pipeline — IOCP scatter-gather async overlapped I/O"
Commit-Sprint 343 "Engine/Memory/MemoryFootprintOptimizerV2.h"      "Memory Footprint Optimizer V2 — slab allocator and large-page pool"
Commit-Sprint 344 "Engine/Utils/ReleaseGateV31.h"                   "Release Gate V31 — Phase 9 Performance Excellence gate (10 KPIs)"

# Phase 10: v14.0 Release
Commit-Sprint 345 "Engine/Core/AccessibilitySuiteV2.h"         "Accessibility Suite V2 — WCAG 2.1 AA + 5 color-blind modes + UIA"
Commit-Sprint 346 "Engine/Utils/DocumentationExcellenceV2.h"   "Documentation Excellence V2 — drift detection and auto-gen pipeline"
Commit-Sprint 347 "Engine/Utils/QualityAssuranceV2.h"           "Quality Assurance V2 — ship-signal QA gate with defect severity"
Commit-Sprint 348 "Engine/Utils/ReleaseGateV32.h"               "Release Gate V32 (v14.0 Final) — Apex complete, 23 KPIs, v14ShipApproved"

# Bulk commit for CMakeLists.txt + EngineTests changes
git add Engine/CMakeLists.txt Engine/Tests/EngineTests.cpp 2>&1 | Out-Null
git commit -m "Sprints 299-348: Register all v14.0 Apex headers in CMakeLists.txt and add 250 tests to EngineTests.cpp" 2>&1
Write-Host "Bulk registration commit done" -ForegroundColor Cyan

Write-Host "`nAll 51 commits complete!" -ForegroundColor Yellow

