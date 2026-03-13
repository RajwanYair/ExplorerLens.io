# ExplorerLens — Architecture Reference

> **Version:** 15.0.0 "Zenith" | **Last Updated:** 2026-03-10

## System Overview

ExplorerLens is a **Windows Shell Extension** (COM DLL) that generates GPU-accelerated thumbnails
for 200+ file formats. The system is composed of three main deliverables:

```text
┌─────────────────────────────────────────────────────────────┐
│                    Windows Explorer                          │
│  (Host process: explorer.exe / prevhost.exe)                │
└────┬────────────────────────┬───────────────────────────────┘
     │ IThumbnailProvider     │ IPropertyStore
     │ IExtractImage2         │ IQueryInfo
     ▼                        ▼
┌─────────────────────────────────────────────────────────────┐
│  LENSShell.dll  (2940 KB)  — COM Shell Extension            │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ CLENSShell   │  │PropertyStore │  │ QueryInfo    │      │
│  │ (COM Class)  │  │  Impl        │  │  Impl        │      │
│  └──────┬───────┘  └──────────────┘  └──────────────┘      │
│         │                                                    │
│  ┌──────▼──────────────────────────────────────────────┐    │
│  │  ExplorerLensEngine.lib  (311 MB static library)    │    │
│  │  ┌─────────┐ ┌──────────┐ ┌────────┐ ┌──────────┐  │    │
│  │  │ Pipeline │ │ Decoders │ │  GPU   │ │  Cache   │  │    │
│  │  │ Manager  │ │ (25+)    │ │ Render │ │ Manager  │  │    │
│  │  └─────────┘ └──────────┘ └────────┘ └──────────┘  │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│  LENSManager.exe  (400 KB)  — WTL Admin GUI                │
│  ┌────────────────┐  ┌──────────────┐  ┌───────────────┐   │
│  │ Format Config   │  │ COM Register │  │ Settings I/O  │   │
│  │ (formatHandlers)│  │ (Admin Elev) │  │ (JSON/Reg)    │   │
│  └────────────────┘  └──────────────┘  └───────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

## Data Flow: Thumbnail Generation

```text
              ┌──────────────┐
              │  Explorer     │
              │  requests     │
              │  thumbnail    │
              └──────┬───────┘
                     │ IStream
                     ▼
              ┌──────────────┐
              │  Format      │     FormatDetector + FormatSignatureDetector
              │  Detection   │     (magic bytes, extension, heuristics)
              └──────┬───────┘
                     │ LENSTYPE
                     ▼
              ┌──────────────┐
              │  Decoder     │     DecoderRegistry → IThumbnailDecoder
              │  Dispatch    │     (25+ specialized decoders)
              └──────┬───────┘
                     │ Raw pixels (BGRA)
                     ▼
              ┌──────────────┐
              │  GPU Resize  │     D3D11 → D3D12 → Vulkan → GDI fallback
              │  Pipeline    │     Lanczos3 / Bicubic / HDR tone-map
              └──────┬───────┘
                     │ HBITMAP (target size)
                     ▼
              ┌──────────────┐
              │  Cache       │     ThumbnailCache (memory) + PersistentDiskCache
              │  Write       │     SubMillisecondCacheEngine for hot lookups
              └──────┬───────┘
                     │
                     ▼
              ┌──────────────┐
              │  Return to   │
              │  Explorer    │
              └──────────────┘
```

## Module Architecture

### Engine/Core/
Central engine components including format detection, configuration, error recovery,
telemetry, and version management. Key classes:

| Class | Purpose |
| ------- | --------- |
| `EngineAPI` | Public C++ API surface for engine consumers |
| `FormatRegistry` | Maps extensions → LENSTYPE → decoder |
| `FormatStatusProvider` | Aggregates decoder health for UI display |
| `Config` | Runtime configuration (registry-backed) |
| `ErrorRecoveryEngine` | Automatic retry + fallback on decode failure |
| `SIMDAccelerationManager` | SIMD dispatch routing (AVX2/SSE4.1/NEON) |
| `SIMDCapabilityDetector` | CPUID-based feature detection + OS verification |
| `PerformanceDashboard` | Real-time metrics collection with P95/P99 |
| `SettingsExportImport` | JSON/Registry settings export/import |
| `SystemTrayManager` | Notification area icon + balloon notifications |

### Engine/Pipeline/
Request processing pipeline with zero-copy data paths:

| Class | Purpose |
| ------- | --------- |
| `ThumbnailPipeline` | Main entry point (PImpl pattern) |
| `FormatDetector` | Magic-byte based format identification |
| `DecoderRegistry` | Maps formats to decoder instances |
| `ZeroCopyPipeline` | Decoder → GPU without intermediate copies |
| `ZeroCopyActivation` | Production activation wrapper (GPU-direct uploads) |
| `ParallelIOPipeline` | IOCP-based multi-file parallel reads |
| `ParallelIOActivation` | Production parallel I/O activation (batch pre-read) |
| `PipelineActivator` | Orchestrates all subsystem activation |
| `AsyncThumbnailProvider` | Non-blocking thumbnail generation |
| `ParallelBatchDecoder` | Multi-threaded batch decode |

### Engine/Decoders/
25+ format-specific decoders, each implementing `IThumbnailDecoder`:

| Decoder | Formats | Library |
| --------- | --------- | --------- |
| `ImageDecoder` | BMP, PNG, JPEG, GIF, TIFF | WIC (built-in) |
| `WebPDecoder` | WebP | libwebp 1.5.0 |
| `JXLDecoder` | JPEG XL | libjxl 0.11.1 |
| `HEIFDecoder` | HEIF, HEIC | libheif 1.19.5 |
| `AVIFDecoder` | AVIF | libavif 1.3.0 + dav1d |
| `RAWDecoder` | CR2, NEF, ARW, DNG, 50+ | LibRaw 0.21.3 |
| `PDFDecoder` | PDF | MuPDF 1.25.5 |
| `JPEG2000Decoder` | JP2, J2K | OpenJPEG 2.5.3 |
| `FontDecoder` | TTF, OTF, WOFF | FreeType 2.13.3 |
| `ArchiveDecoder` | ZIP, 7z, RAR, TAR | minizip-ng + LZMA + UnRAR |
| `SVGDecoder` | SVG | Custom parser |
| `EXRDecoder` | OpenEXR | Built-in |
| `HDRDecoder` | Radiance HDR | Built-in |
| `DDSDecoder` | DirectDraw Surface | Built-in (BC1-7) |
| `PSDDecoder` | Photoshop | Built-in |
| `VideoDecoder` | MP4, AVI, MKV, MOV | Media Foundation |

### Engine/GPU/
Multi-backend GPU rendering with automatic fallback:

```text
Priority: D3D11 → D3D12 → Vulkan → GDI (software)
```

| Class | Purpose |
| ------- | --------- |
| `D3D11Renderer` | Primary GPU path (broadest compatibility) |
| `D3D12ComputePipeline` | Compute shader path for batch decode |
| `VulkanComputePipeline` | Cross-vendor compute (AMD/Intel/NVIDIA) |
| `GDIRenderer` | Software fallback (always available) |
| `GPUDecodeAccelerationV2` | Vendor-specific HW decode (NVDEC/QSV/AMF) |

HLSL Shaders (in `LENSShell/shaders/`):
- `thumbnail_resize.hlsl` — Standard bilinear resize
- `lanczos_resize.hlsl` — Lanczos-3 high-quality resize
- `bicubic_resize.hlsl` — Bicubic interpolation
- `hdr_tonemap.hlsl` — HDR → SDR tone mapping (Reinhard/ACES)
- `color_convert.hlsl` — Color space conversion (sRGB/P3/Rec.2020)

### Engine/Cache/
Multi-tier caching with sub-millisecond hot lookups:

| Class | Purpose |
| ------- | --------- |
| `ThumbnailCache` | In-memory LRU cache |
| `PersistentDiskCache` | Disk-backed cache with USN invalidation |
| `SubMillisecondCacheEngine` | Robin-hood hash, XXH3, <0.5ms lookup |
| `CacheWarmingService` | Proactive directory-watch pre-warming |
| `CacheWarmingActivation` | Production cache warming activation |
| `CacheBudgetAutoTuner` | RAM-aware automatic budget adjustment |
| `PSOCachePersistence` | GPU pipeline state object disk cache |
| `AdaptiveCacheBudgetManager` | Dynamic cache sizing per workload |

### Engine/Memory/
Memory management for constrained shell extension environment:

| Class | Purpose |
| ------- | --------- |
| `BitmapPool` | Pre-allocated HBITMAP pool (128/256/512) |
| `MemoryPressureControllerV2` | 5-tier pressure ladder |
| `ArchiveMemoryCompactor` | Archive buffer compaction |
| `BufferPoolAllocator` | Pool-based buffer allocation |
| `SmartPointerPool` | Ref-counted smart pointer pool |

### Engine/Plugin/
Plugin ecosystem with sandboxing and trust validation:

| Class | Purpose |
| ------- | --------- |
| `PluginManager` | Discovery, loading, lifecycle |
| `PluginDecoder` | Wraps plugin decoders as IThumbnailDecoder |
| `PluginSandboxPolicy` | Job object isolation |
| `PluginTrustChain` | Code signing verification |
| `PluginMarketplaceV2` | Online plugin marketplace |
| `CrashIntelligenceEngine` | Plugin crash analysis |

### Engine/AI/
ML-powered thumbnail intelligence:

| Class | Purpose |
| ------- | --------- |
| `SmartCropV2` | Content-aware crop (saliency detection) |
| `SceneUnderstandingEngine` | Scene classification |
| `ImageQualityAssessorV2` | No-reference quality scoring |
| `AISearchIntegration` | Semantic thumbnail search |
| `ThumbnailRelevanceScorer` | Page/frame relevance ranking |

## COM Registration

| Property | Value |
| ---------- | ------- |
| CLSID | `{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}` |
| ProgID | `ExplorerLens.ThumbnailProvider.1` |
| Threading Model | Apartment |
| Interfaces | IThumbnailProvider, IInitializeWithStream, IPropertyStore, IPropertyStoreCapabilities, IExtractImage2, IPersistFile, IQueryInfo |

## Build Architecture

```text
CMake 3.20+ → Ninja → MSVC v145 (cl.exe 19.50)
                          │
         ┌────────────────┼────────────────┐
         ▼                ▼                ▼
  ExplorerLensEngine   EngineTests    EngineBenchmarks
  (STATIC .lib)        (.exe)         (.exe)
         │
         ▼
  MSBuild → LENSShell.dll + LENSManager.exe
         │
         ▼
  WiX v6 → ExplorerLens.msi
```

### Feature Flags (CMake Options)

| Flag | Default | Purpose |
| ------ | --------- | --------- |
| `HAS_LIBJXL` | ON | JPEG XL decoder support |
| `HAS_LIBHEIF` | ON | HEIF/HEIC decoder support |
| `HAS_LIBRAW` | ON | RAW camera format support |
| `HAS_LIBAVIF` | ON | AVIF decoder support |
| `HAS_MUPDF` | ON | PDF decoder support |
| `HAS_OPENJPEG` | ON | JPEG 2000 decoder support |
| `HAS_FREETYPE` | ON | Font rendering support |
| `HAS_LIBARCHIVE` | OFF | Unified archive support |

## Performance Targets

| Metric | Target | Actual |
| -------- | -------- | -------- |
| Single thumbnail | < 17 ms | ~15 ms |
| Batch throughput | > 235 img/sec | ~250 img/sec |
| Cache hit latency | < 5 ms | < 0.5 ms |
| Memory footprint | < 50 MB idle | ~35 MB |
| DLL size | < 3 MB | 2940 KB |
| Test count | > 1000 | 2938 |
| Warning count | 0 | 0 |

## Security Model

- **COM Apartment:** STA (Single-Threaded Apartment) — matches Explorer's model
- **Process Isolation:** Shell extension runs in explorer.exe process space
- **Plugin Sandbox:** Job objects with restricted tokens
- **Code Signing:** Authenticode for plugins via PluginTrustChain
- **Memory Safety:** ASLR, DEP, CFG enabled; /GS buffer overrun detection
- **Race Protection:** SRWLOCK throughout for thread safety

## Cross-Platform Support (Python)

The `ExplorerLens.py` package provides cross-platform thumbnail generation:

| Platform | Backend | Module |
| -------- | ------- | ------ |
| Windows | COM IThumbnailProvider | `shell.com_server` |
| Linux | Freedesktop.org .thumbnailer | `shell.linux_thumbnailer` |
| macOS | Quick Look .qlgenerator | `shell.macos_quicklook` |

All platforms share the same `platform_provider` abstraction for registration,
unregistration, and status queries.
