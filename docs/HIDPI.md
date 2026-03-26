# HiDPI & Multi-Monitor Support

ExplorerLens v18.1.0+ renders thumbnails at the correct **physical pixel density** for every monitor in a multi-display setup. This document describes the HiDPI pipeline and configuration options.

---

## Architecture Overview

```
Shell Request (logical px)
        │
        ▼
ThumbnailDensitySelector
 ├── Uses MonitorDPI from MultiMonitorContext
 ├── Applies DPIScalingPolicy (Exact / SnapToInteger / ShellHinted)
 └── Emits physicalSize + cacheKey "@2x" suffix
        │
        ▼
HiDPIThumbnailCache.Get(cacheKey)
 ├── HIT  → return cached BGRA immediately (<0.5 ms)
 └── MISS → Decode at native resolution via CodecPlatformV2
              └── HiDPIScaler.Scale() if src < dst (Lanczos-3)
                       │
                       ▼
               HiDPIThumbnailCache.Put(entry)
```

---

## DPI Buckets

ExplorerLens snaps to the nearest standard Windows DPI bucket for cache efficiency:

| DPI  | Scale | Suffix | Example Display       |
|------|-------|--------|-----------------------|
| 96   | 1.0×  | —      | 1080p 24"             |
| 120  | 1.25× | @1.25x | 1080p 21.5"           |
| 144  | 1.5×  | @1.5x  | QHD 27" or 4K 32"     |
| 168  | 1.75× | @1.75x | 4K 27"                |
| 192  | 2.0×  | @2x    | 4K 24", Retina        |
| 240  | 2.5×  | @2.5x  | 5K iMac equivalent    |
| 288  | 3.0×  | @3x    | Phone-density panels  |

Fractional DPIs (e.g. 150 DPI → 1.5625×) are either snapped (safe mode) or decoded at exact ratio depending on `DPIScalingPolicy.mode`.

---

## Scaling Algorithms

| Algorithm      | Use Case                     | Quality     | Cost    |
|----------------|------------------------------|-------------|---------|
| Lanczos-3      | HiDPI upscale (default)      | Excellent   | Medium  |
| Mitchell B=1/3 | Bicubic general purpose      | Very Good   | Low     |
| Area Average   | Downscale (>2× reduction)    | Best        | Low     |
| Bilinear       | Fast preview                 | Acceptable  | Minimal |
| Nearest Neighbour | Pixel art / debug         | N/A         | None    |

`HiDPIScaler::RecommendAlgorithm()` auto-selects based on the src→dst ratio.

---

## Cache Isolation

Each DPI bucket maintains independent cache entries:

```
thumb_abc123@2x_256   ← 2× physical entry for 256 logical px
thumb_abc123@1x_256   ← 1× entry for same file
thumb_abc123@2x_128   ← 2× entry at 128 logical px shell size
```

`HiDPIThumbnailCache::InvalidateBucket(DPIBucket::DPI_192)` removes all `@2x` entries — triggered automatically when a 4K monitor is disconnected via `MonitorConfigWatcher`.

---

## Multi-Monitor Scenarios

### Dragging a Thumbnail Between Monitors

When a file is visible on multiple monitors simultaneously, ExplorerLens renders at the highest DPI of all monitors showing the thumbnail. The lower-DPI copies are served from the same entry (downscale is free).

### WM_DISPLAYCHANGE Handling

`MonitorConfigWatcher` runs a hidden message-only HWND on a dedicated thread. On `WM_DISPLAYCHANGE` or `WM_DPICHANGED`:

1. `MultiMonitorContext::Refresh()` re-enumerates all monitors
2. `HiDPIThumbnailCache::InvalidateBucket()` evicts stale density entries
3. `DisplayColorProfileLoader::RefreshAll()` reloads ICC profiles

---

## Color Management

`DisplayColorProfileLoader` loads the ICC profile associated with each `HMONITOR` via the Windows ICM API. If the profile is non-sRGB (Display-P3, AdobeRGB):

1. Decoded BGRA is converted from the internal sRGB pipeline to the display color space using a 3×3 matrix
2. `DisplayColorSpace::DisplayP3` and `DisplayColorSpace::AdobeRGB` are handled with D65 white point adaptation

To disable color management: set `DisplayColorProfileLoader::NeedsGamutMapping()` result to `false` by forcing sRGB fallback in `LENSManager` settings.

---

## Configuration

`DPIScalingPolicy` fields (accessible via `LENSManager` advanced tab):

| Field               | Default  | Description                               |
|---------------------|----------|-------------------------------------------|
| `mode`              | `Exact`  | `Exact` / `SnapToInteger` / `ShellHinted` |
| `snapFractionalDPI` | `false`  | Force integer scale for compatibility     |
| `maxPhysicalPx`     | `2048`   | Hard upper limit for decoded size         |
| `emitHighDPISuffix` | `true`   | Enable DPI-bucketed cache keys            |
| `sharpenOnUpscale`  | `false`  | Unsharp mask after Lanczos upscale        |
| `sharpenAmount`     | `0.3`    | Sharpness intensity (0.0–1.0)             |

---

## Performance

| Scenario                          | Latency    |
|-----------------------------------|------------|
| HiDPI cache hit (@2x)             | < 0.5 ms   |
| Lanczos-3 upscale (256→512 px)    | < 2 ms     |
| Full decode + upscale (4K HEIC)   | ~18 ms     |
| WM_DISPLAYCHANGE invalidation     | < 5 ms     |
