# ExplorerLens v34.x "Arcturus" — Best-in-Class Format & Performance Roadmap

**Status:** Active planning
**Current Version:** v33.0.0 "Spica" (2026-04-05)
**Next Major:** v34.0.0 "Arcturus" (star Alpha Boötis, 4th brightest)
**Goal:** Achieve undisputed format coverage and decode performance leadership among
all thumbnail providers on Windows, macOS, and Linux.

---

## Executive Summary

v33.x "Spica" delivers the cross-platform shell (macOS/Linux), generative AI thumbnails,
and enterprise policy engines. v34.x "Arcturus" pivots to **breadth and speed** — the two
dimensions that directly determine whether users keep ExplorerLens or uninstall it.

**Three pillars:**

1. **350+ File Extensions** — Cover every format a photographer, VFX artist, game developer,
   architect, electrical engineer, scientist, or musician encounters on disk. The mission:
   *if a file exists, ExplorerLens thumbnails it.*

2. **Sub-10 ms Common-Format Decode** — GPU-first pipeline makes JPEG/PNG/WebP/AVIF
   thumbnails faster than the OS shell can request them. Predictive pre-generation means
   thumbnails are ready before the user navigates to a folder.

3. **Zero-Regression Performance Gates** — Per-PR automated benchmarks block any commit
   that degrades P95 latency by >5% or batch throughput by >10%.

---

## Version Timeline

```
v33.0.0  Spica        Cross-Platform Shell + GenAI + Enterprise v4    (2026-04-05) ✅
v33.1.0  Spica-R      PAL Metal + Vulkan EGL implementations          (planned)
v33.2.0  Spica-S      Enterprise Console v4 + GPO Policy Templates     (planned)
v33.3.0  Spica-T      Generative AI Thumbnails (NPU)                   (planned)
v33.4.0  Spica-U      Plugin Marketplace v5 + SDK Compat Kit v3        (planned)
v33.5.0  Spica-V      LTS Hardening + Security Audit                   (planned)
   │
   ├── v34.0.0  Arcturus       Format Coverage Blitz ★★             (target)
   ├── v34.1.0  Arcturus-R     GPU-First Decode Pipeline             (target)
   ├── v34.2.0  Arcturus-S     HDR / Wide Color Gamut Mastery        (target)
   ├── v34.3.0  Arcturus-T     Predictive Pre-Generation Engine      (target)
   ├── v34.4.0  Arcturus-U     Animated & Sequence Format Suite      (target)
   ├── v34.5.0  Arcturus-V     Industrial & Scientific Formats v2    (target)
   ├── v34.6.0  Arcturus-W     CAD / BIM / EDA Native Decode         (target)
   ├── v34.7.0  Arcturus-X     Performance Hardening + LTS Gate      (target)
   │
   └── v35.0.0  Vega           Streaming & Cloud-Native Thumbnails ★★  (future)

★★ = Landmark MAJOR release
```

---

## Current Baseline (v33.0.0 "Spica")

| Metric | Value |
|--------|-------|
| File extensions supported | ~200+ |
| LENSTYPE enum values | 74 |
| Decoder classes | 179 |
| Unit tests | 4,583 |
| Single decode P50 (JPEG 6MP) | 4.2 ms |
| Batch throughput | 235 img/sec |
| Cache hit P50 | 0.8 ms |
| Idle memory | ~24 MB |
| Platforms | Windows + macOS (stub) + Linux (stub) |

---

## T1 — Format Coverage Blitz (v34.0.0 "Arcturus") ★★

**Goal:** 200+ → 350+ file extensions. Fill every gap that competitors cover.

### 1.1 Missing Modern Image Formats

| Format | Extensions | Library | Priority | Notes |
|--------|-----------|---------|----------|-------|
| Ultra HDR (Google Gainmap JPEG) | `.jpg` (gainmap) | libultrahdr 1.3+ | **P0** | Android/Pixel default; gainmap → HDR display mapping |
| Apple ProRAW | `.dng` (with gainmap) | LibRaw + custom gainmap | **P0** | iPhone 15/16 Pro default RAW format |
| HEIC Live Photo | `.heic` (sequence) | libheif + MOV demux | **P0** | Key frame + 3s video; show still + play icon overlay |
| Samsung Expert RAW | `.dng` | LibRaw 0.21.3 | P1 | Galaxy S24/S25 Ultra RAW; already handled by DNG path |
| Animated WebP 2 | `.wp2` | libwebp2 (experimental) | P2 | Google's next-gen; track upstream stability |
| JPEG-XR HD Photo | `.jxr`, `.hdp`, `.wdp` | WIC fallback | P1 | Already have JXRWICDecoder; ensure P50 < 8 ms |
| QOIR | `.qoir` | header-only | P2 | QOI successor; already have QOIRDecoder class |

### 1.2 Professional Photography & VFX

| Format | Extensions | Library | Priority | Notes |
|--------|-----------|---------|----------|-------|
| Cineon (film scan) | `.cin` | Custom header (10-bit log) | P1 | Post-production houses use daily |
| DPX v2.0 | `.dpx` | Already have DPXDecoder | P1 | Verify v2.0 field support; 10/12/16-bit |
| ACES / ACEScg EXR | `.exr` (ACES) | OpenEXR + ACES CTL | P1 | Color-managed display with ACES ODT |
| Nikon Z9 NEV | `.nev` | LibRaw 0.21.3 | P1 | HE video-style RAW; niche but requested |
| Hasselblad 3FR/FFF | `.3fr`, `.fff` | LibRaw | P1 | Medium-format; verify >100MP decode path |
| Phase One IIQ | `.iiq` | LibRaw | P1 | 150MP+ files; need tiled decode with ROI |
| Leica L2 DNG | `.dng` | LibRaw | P2 | Leica SL3 RAW |
| Sigma X3F/Foveon | `.x3f` | LibRaw | P2 | Already listed in decoder; verify |
| Canon CR3 (C-RAW) | `.cr3` | LibRaw 0.21.3 | P0 | Compressed RAW; ensure CRAW variant works |
| Fujifilm RAF (X-Trans V) | `.raf` | LibRaw | P1 | X-T6 sensor; verify X-Trans demosaic quality |

### 1.3 Game Development & Real-Time Graphics

| Format | Extensions | Library | Priority | Notes |
|--------|-----------|---------|----------|-------|
| Basis Universal | `.basis`, `.ktx2` | basisu transcoder | **P0** | GPU-native; transcode to BC7/ASTC in decode |
| ASTC (Adaptive Scalable) | `.astc` | Custom header | P1 | ARM Mali/Adreno textures; common in mobile |
| BC7/BPTC (DDS variant) | `.dds` (DX11 BC7) | DDSDecoder upgrade | P1 | Verify all DX11/DX12 block formats |
| PVR (PowerVR) | `.pvr` | Custom header | P2 | iOS/console textures |
| Unreal PAK assets | `.uasset` | Custom parser | P2 | Show embedded texture; read UObject header |
| Unity asset bundles | `.unity3d`, `.assets` | Custom parser | P2 | Extract embedded PNG/DDS preview |
| FBX 7.x binary | `.fbx` | Already have FBXInspector | P1 | Render first mesh wireframe at 256×256 |
| USD/USDA/USDZ | `.usd`, `.usda`, `.usdc`, `.usdz` | Already have USDADecoder/USDDecoder | P1 | Ensure MaterialX support + Apple AR preview |
| Alembic | `.abc` | Already have AlembicDecoder | P1 | Animated mesh; show first frame |
| MaterialX | `.mtlx` | Already have MaterialXDecoder | P2 | Show node graph thumbnail |

### 1.4 Architecture, CAD & BIM

| Format | Extensions | Library | Priority | Notes |
|--------|-----------|---------|----------|-------|
| AutoCAD DWG | `.dwg` | Open Design Alliance (ODA) Teigha or LibreDWG | **P0** | #1 most-requested missing format |
| AutoCAD DXF | `.dxf` | ODA or LibreDWG | **P0** | Text-based; easier than DWG |
| IFC (BIM) | `.ifc` | IfcOpenShell / IFC++ | P1 | Architects need this; render first floor plan |
| STEP/IGES | `.step`, `.stp`, `.iges`, `.igs` | OpenCASCADE (OCCT) | P1 | Mechanical engineering standard |
| Revit | `.rvt` | ODA | P2 | Proprietary Autodesk; fallback to embedded preview |
| SketchUp | `.skp` | SketchUp C SDK (free) | P1 | Widely used; render first viewport |
| Rhino 3DM | `.3dm` | openNURBS (free, MIT) | P1 | Industrial design; NURBS rendering |
| Blender | `.blend` | Python parser or embedded preview | P2 | Extract embedded thumbnail from header |
| SolidWorks | `.sldprt`, `.sldasm` | ODA or fallback | P2 | Proprietary; extract embedded JPEG |
| CATIA | `.catpart` | ODA or fallback | P3 | Enterprise CAD; fallback to type icon overlay |

### 1.5 Electronics & PCB Design

| Format | Extensions | Library | Priority | Notes |
|--------|-----------|---------|----------|-------|
| Gerber (RS-274X) | `.gbr`, `.ger`, `.gtl`, `.gbl` | gerbv or custom parser | P1 | PCB manufacturing; render copper layer |
| KiCad | `.kicad_pcb`, `.kicad_sch` | S-expression parser | P1 | Popular open-source; render board outline |
| Altium | `.pcbdoc`, `.schdoc` | OLE compound + custom | P2 | Proprietary; extract embedded preview |
| Eagle | `.brd`, `.sch` | XML parser | P2 | Legacy Autodesk; XML since v6+ |
| SPICE netlist | `.cir`, `.spice` | Text preview | P3 | Show circuit description text |

### 1.6 Scientific & Medical (Enhancement)

| Format | Extensions | Library | Priority | Notes |
|--------|-----------|---------|----------|-------|
| DICOM (Enhanced MR/CT) | `.dcm`, `.ima` | Already have DICOMDecoderV2 | P0 | Add windowing presets (lung/bone/brain) |
| NIfTI-2 | `.nii`, `.nii.gz` | Already have NIfTIDecoder | P1 | Show middle axial/sagittal/coronal slice |
| FITS (Astronomy) | `.fits`, `.fit`, `.fts` | Already have FITSDecoderV2 | P1 | Auto-stretch with zscale; show WCS coords |
| HDF5 (Large datasets) | `.hdf5`, `.h5`, `.he5` | Already have HDF5ThumbnailDecoder | P1 | Show first 2D dataset as heatmap |
| NetCDF Climate | `.nc`, `.nc4` | Already have NetCDFDecoder | P2 | Geo-overlay with coastline vectors |
| MHA/MHD (ITK) | `.mha`, `.mhd` | SimpleITK header parse | P2 | Medical 3D volume; show middle slice |
| Zarr (Array store) | `.zarr` | Custom directory reader | P3 | Cloud-native array; popular in genomics |
| OME-TIFF (Microscopy) | `.ome.tif`, `.ome.tiff` | libtiff + OME-XML | P2 | Multi-channel fluorescence; show composite |

### 1.7 Document & eBook Expansion

| Format | Extensions | Library | Priority | Notes |
|--------|-----------|---------|----------|-------|
| LaTeX | `.tex` | Already have LaTeXPreviewDecoder | P1 | Render first equation/page preview |
| Jupyter Notebook | `.ipynb` | JSON parse → render first code + output cell | P1 | Show first plot output as thumbnail |
| Markdown | `.md` | Already have MarkdownPreviewRenderer | P1 | Render with syntax highlighting |
| AsciiDoc | `.adoc`, `.asciidoc` | Text preview with heading extraction | P2 | Popular in Java/Ruby docs |
| reStructuredText | `.rst` | Text preview | P3 | Python docs standard |
| Org-mode | `.org` | Text preview | P3 | Emacs ecosystem |
| LibreOffice Draw | `.odg` | ZIP + content.xml → SVG | P2 | Vector graphics document |
| Visio | `.vsdx` | ZIP + OLE + EMF extraction | P2 | Diagram; extract embedded EMF preview |
| Apple Pages/Numbers/Keynote | `.pages`, `.numbers`, `.key` | ZIP + QuickLook preview extract | P1 | macOS users; embedded preview in ZIP |

### 1.8 Audio & Music Notation

| Format | Extensions | Library | Priority | Notes |
|--------|-----------|---------|----------|-------|
| WAV waveform | `.wav` | Direct PCM read | P1 | Show waveform preview strip |
| OPUS | `.opus` | Already handled by AudioDecoder | P1 | Verify album art extraction |
| MIDI visualization | `.mid`, `.midi` | Already have MIDIVisualizer | P1 | Piano roll or notation preview |
| MusicXML | `.musicxml`, `.mxl` | Already have NotationDecoder | P2 | Show first measure of score |
| ABC notation | `.abc` | Text + simple note rendering | P3 | Folk music standard |
| LilyPond | `.ly` | Text + render first system | P3 | Engraving; compile is slow → text preview |

### 1.9 Source Code & Configuration

| Format | Extensions | Library | Priority | Notes |
|--------|-----------|---------|----------|-------|
| Source code (all languages) | `.py`, `.js`, `.ts`, `.cpp`, `.rs`, `.go`, `.java`, `.cs`, `.rb`, `.swift`, `.kt`, `.zig`, `.c`, `.h` | Already have SourceCodeThumbnail | P0 | Syntax-highlighted first 40 lines |
| Shader files | `.glsl`, `.hlsl`, `.wgsl`, `.metal`, `.frag`, `.vert`, `.comp` | SourceCodeThumbnail | P1 | GPU shader preview with GLSL coloring |
| Config files | `.toml`, `.ini`, `.cfg`, `.conf`, `.env`, `.properties` | TextPreviewDecoder | P2 | Structured text preview |
| Docker | `Dockerfile`, `docker-compose.yml` | TextPreviewDecoder | P2 | Show service graph or text preview |
| Terraform/HCL | `.tf`, `.hcl` | TextPreviewDecoder | P2 | Infrastructure-as-code |

---

## T2 — GPU-First Decode Pipeline (v34.1.0 "Arcturus-R")

**Theme:** Move decode hot-path from CPU → GPU for the top 10 formats by volume.

### Performance Targets

| Metric | v33.0.0 Baseline | v34.1.0 Target | Improvement |
|--------|-----------------|----------------|-------------|
| JPEG 6MP P50 | 4.2 ms | **1.5 ms** | 2.8× |
| PNG 4K P50 | 5.8 ms | **2.0 ms** | 2.9× |
| WebP P50 | 6.3 ms | **2.5 ms** | 2.5× |
| AVIF P50 | 9.1 ms | **3.5 ms** | 2.6× |
| JPEG XL P50 | 11.2 ms | **5.0 ms** | 2.2× |
| PDF first-page P50 | 17.0 ms | **8.0 ms** | 2.1× |
| RAW 24MP P50 | 22.5 ms | **9.0 ms** | 2.5× |
| Batch throughput | 235 img/s | **600 img/s** | 2.6× |
| Cache hit P50 | 0.8 ms | **0.3 ms** | 2.7× |
| Cache hit P99 | 4.5 ms | **1.0 ms** | 4.5× |

### GPU Decode Routing By Format

| Format | GPU Path | CPU Fallback |
|--------|----------|-------------|
| JPEG | NVJPEG (NVIDIA) / Intel QSV JPEG / WIC GPU | libjpeg-turbo SIMD |
| PNG | DirectCompute inflate + unfilter | libpng NEON/SSE |
| WebP | Custom VP8 compute shader | libwebp SIMD |
| AVIF | NVDEC AV1 (NVIDIA) / Intel QSV AV1 / dav1d SIMD | dav1d multithreaded |
| HEVC/HEIC | NVDEC HEVC / Intel QSV HEVC | libde265 SIMD |
| H.264 video | NVDEC H.264 / Intel QSV / AMD AMF | Media Foundation |
| H.265 video | NVDEC HEVC / Intel QSV / AMD AMF | Media Foundation |
| AV1 video | NVDEC AV1 / Intel QSV AV1 / AMD AMF | dav1d |
| PDF | GPU rasterizer (D2D + DirectWrite) | MuPDF CPU |
| RAW | GPU demosaic + WB compute shader | LibRaw SIMD |

### Architecture

```
File → DirectStorage NVMe Queue → GPU Staging Buffer
  │
  ├── GPU Decompress (GDeflate / ZStd compute kernel)
  ├── GPU Decode (NVJPEG / NVDEC / custom compute)
  ├── GPU Resize (bilinear/Lanczos compute shader)
  ├── GPU Tonemap (PQ→sRGB or HLG→sRGB)
  └── GPU → Thumbnail BGRA surface (zero-copy to Shell)
                                    │
                     CPU fallback ◄──┘ (if no GPU / driver issue)
```

### Key Libraries

| Library | Version | Purpose |
|---------|---------|---------|
| NVJPEG | CUDA 12.6+ | NVIDIA hardware JPEG decode |
| NVDEC (Video Codec SDK) | 12.2+ | NVIDIA H.264/HEVC/AV1/VP9 |
| Intel oneVPL (QSV) | 2024.2+ | Intel GPU decode (JPEG/HEVC/AV1) |
| AMD AMF | 1.4.35+ | AMD GPU video decode |
| DirectStorage SDK | 1.2+ | NVMe → GPU zero-copy |
| D3D12MA | 2.1+ | GPU memory allocator |

---

## T3 — HDR & Wide Color Gamut Mastery (v34.2.0 "Arcturus-S")

**Problem:** HDR content (iPhone gains, AVIF/JXL HDR, OpenEXR) renders washed-out
in sRGB thumbnails. Users see flat, low-contrast previews of their best photos.

### Features

| Feature | Description |
|---------|-------------|
| **Gainmap JPEG rendering** | Parse Google Ultra HDR gainmap → tone-map to SDR with preserved local contrast |
| **Apple ProRAW gainmaps** | Detect DNG gainmap tag → apply Apple HDR intent for SDR preview |
| **PQ → sRGB tonemapping** | SMPTE ST.2084 (PQ) curve → Hable/ACES filmic tonemap → sRGB thumbnail |
| **HLG → sRGB tonemapping** | ITU-R BT.2100 HLG → BT.709 with system-gamma adaptation |
| **ICC v5 profile support** | iccMAX (ICC.2) profiles; multi-spectral color management |
| **Display P3 rendering** | Render in Display P3 gamut when Windows HDR / macOS EDR is active |
| **Rec.2020 HDR thumbnails** | For HDR-capable displays, output HDR10 thumbnail surface (D3D12 HDR swap chain) |
| **EXR ACES workflow** | Detect ACES color space → apply appropriate ODT (Output Device Transform) |
| **Auto-exposure for RAW** | Histogram-based auto-exposure; avoid clipped highlights in thumbnail |

### Performance Targets

| Operation | Target |
|-----------|--------|
| Gainmap tonemap | < 1 ms (GPU compute) |
| PQ → sRGB | < 0.5 ms (LUT + compute) |
| ICC v5 transform | < 2 ms (per thumbnail) |
| EXR ACES ODT | < 3 ms |

---

## T4 — Predictive Pre-Generation Engine (v34.3.0 "Arcturus-T")

**Theme:** Generate thumbnails before the user sees them.

### How It Works

```
User navigates to folder C:\Photos\Vacation\
  │
  ├── Explorer sends IExtractImage request for visible files (20-40)
  │
  ├── ExplorerLens ALSO: reads directory listing → identifies ALL files (2000)
  │   ├── Priority queue: visible files → adjacent files → deep scan
  │   ├── Background thread pool: 4 worker threads pre-generating at LOW priority
  │   ├── Cache fills silently; next scroll is instant (cache hit < 0.3 ms)
  │   └── Adaptive rate: throttle on battery / thermal pressure
  │
  └── Result: 97% cache-hit rate during folder browsing
```

### Features

| Feature | Description |
|---------|-------------|
| **Directory pre-scan** | On folder open, enumerate all files and queue for background thumbnailing |
| **Adjacency prediction** | Predict next folder (MRU, sibling directories) and pre-generate |
| **Scroll prediction** | Track scroll velocity → speculatively generate thumbnails ahead of viewport |
| **Idle-time generation** | When CPU/GPU is idle (< 5% usage), opportunistically fill cache |
| **Battery awareness** | Disable pre-generation on battery < 20%; reduce at < 50% |
| **Network drive skip** | Do not pre-generate for UNC/network paths (latency too high) |
| **Priority escalation** | If user scrolls over a pre-queued file, promote to immediate |

### Targets

| Metric | Target |
|--------|--------|
| Cache hit rate (folder browse) | **> 95%** |
| Pre-generation throughput | **800 img/s** (background, low-priority) |
| Time to fill 2000-file folder | **< 3 seconds** (NVMe SSD) |
| Memory overhead for pre-gen queue | **< 8 MB** |

---

## T5 — Animated & Sequence Format Suite (v34.4.0 "Arcturus-U")

**Theme:** Animated thumbnails that *move* on hover — Windows Explorer supports this
via IThumbnailProvider returning multiple frames.

### Animated Format Decode Matrix

| Format | Extensions | Frames | Decode Approach |
|--------|-----------|--------|----------------|
| Animated GIF | `.gif` | N frames | GIF LZW + frame disposal |
| Animated PNG (APNG) | `.apng`, `.png` | N frames | Already have APNGDecoder |
| Animated WebP | `.webp` | N frames | Already have WebPAnimationDecoder |
| Animated AVIF | `.avif` | N frames | Already have AVIFSequenceDecoder |
| Animated JXL | `.jxl` | N frames | Already have JXLAnimationDecoder |
| MNG | `.mng` | N frames | Already have MNGDecoder |
| HEIC Live Photo | `.heic` + `.mov` | Still + 3s | Key frame + first 5 video frames |
| FLIF animated | `.flif` | N frames | Already have FLIFDecoderV2 |
| Video scrub strip | `.mp4`, `.mkv`, etc. | 5 keyframes | Already have VideoKeyframeExtractor |

### Hover-Scrub Behavior

```
Mouse enters thumbnail → show static frame (frame 0)
Mouse moves left→right across thumbnail → scrub through keyframes
  Position 0%:   Frame 0
  Position 25%:  Frame N/4
  Position 50%:  Frame N/2
  Position 75%:  Frame 3N/4
  Position 100%: Frame N-1
Mouse leaves → return to frame 0
```

### Targets

| Metric | Target |
|--------|--------|
| First animated frame | < 5 ms |
| Frame-to-frame transition | < 3 ms |
| Memory per animation cache | < 2 MB |
| Hover-scrub latency | < 16 ms (60 fps) |

---

## T6 — Industrial & Scientific Formats v2 (v34.5.0 "Arcturus-V")

**Theme:** Deepen existing scientific decoder quality and add missing industrial formats.

### Enhancements to Existing Decoders

| Decoder | Enhancement |
|---------|-------------|
| DICOMDecoderV2 | Add windowing presets (CT Lung: W=1500/L=-500, CT Bone: W=2500/L=500, Brain: W=80/L=40) |
| FITSDecoderV2 | ZScale auto-stretch + WCS coordinate overlay + multi-HDU navigation |
| HDF5ThumbnailDecoder | Auto-detect 2D datasets → heatmap; 3D → middle slice; tables → stat summary |
| NetCDFDecoder | Geo-referenced overlay with coastline vectors (Natural Earth 50m) |
| NIfTIDecoder | Show 3-plane (axial/sagittal/coronal) composite; detect atlas coordinates |
| GeoTIFFDecoder | Hillshade rendering; detect DEM → elevation colormap |

### New Industrial Formats

| Format | Extensions | Library | Notes |
|--------|-----------|---------|-------|
| LAS/LAZ Point Cloud | `.las`, `.laz` | LAStools or PDAL | Render top-down point density map |
| E57 Point Cloud | `.e57` | libe57 | 3D scan data; render first scan position |
| Zarr Array Store | `.zarr/` | Custom directory reader | Cloud-native array; show first 2D slice |
| OME-TIFF Microscopy | `.ome.tif` | libtiff + OME-XML parser | Multi-channel fluorescence composite |
| MHA/MHD Volume | `.mha`, `.mhd` | Custom header parser | ITK-format 3D medical volume |
| CZI (Zeiss Microscopy) | `.czi` | libCZI | Multi-scale tiled microscopy |

---

## T7 — CAD / BIM / EDA Native Decode (v34.6.0 "Arcturus-W")

**Theme:** Native rendering for the most-requested professional formats.

### CAD Rendering Pipeline

```
.dwg/.step/.ifc file
  │
  ├── Parse geometry (OpenCASCADE / ODA Teigha / IfcOpenShell)
  ├── Extract bounding box → compute view transform
  ├── Tessellate to triangle mesh
  ├── GPU rasterize (D3D12 / Vulkan):
  │   ├── Wireframe mode (line primitives, depth test)
  │   ├── Solid mode (Phong shading, ambient occlusion)
  │   └── Orthographic top/front/iso view auto-select
  └── Composite → 256×256 BGRA thumbnail
```

### Dependency Matrix

| Format Group | Library | License | Size | Notes |
|-------------|---------|---------|------|-------|
| DWG/DXF | ODA Drawings SDK | Commercial (free for OSS) | ~15 MB | Best DWG coverage |
| DWG/DXF (fallback) | LibreDWG | GPL-3.0 | ~3 MB | GPL; process isolation needed |
| STEP/IGES | OpenCASCADE (OCCT) | LGPL-2.1 | ~40 MB | Gold standard for BREP |
| IFC (BIM) | IfcOpenShell | LGPL-3.0 | ~12 MB | IFC4 + IFC2x3 support |
| SketchUp | SketchUp C SDK | Proprietary (free) | ~8 MB | .skp read-only API |
| Rhino 3DM | openNURBS | MIT | ~5 MB | McNeel open source |
| Gerber/Excellon | Custom parser | — | < 100 KB | RS-274X is text-based |
| KiCad | S-expression parser | — | < 100 KB | KiCad 8 format |

### Targets

| Metric | Target |
|--------|--------|
| DWG decode P50 | < 50 ms (simple drawings), < 200 ms (complex assemblies) |
| STEP tessellation P50 | < 100 ms |
| IFC floor plan P50 | < 80 ms |
| Gerber render P50 | < 10 ms |
| Auto-view selection accuracy | > 90% (chooses best viewing angle) |

---

## T8 — Performance Hardening + LTS Gate (v34.7.0 "Arcturus-X")

**Theme:** Lock in all performance gains; zero regression from v34.0.0 through v34.7.0.

### Automated Performance Regression Gates

```yaml
# .github/workflows/perf-gate.yml fires on every PR
perf-gate:
  conditions:
    jpeg_p95_ms:   { max: 3.0, block_on_fail: true }
    png_p95_ms:    { max: 4.0, block_on_fail: true }
    webp_p95_ms:   { max: 5.0, block_on_fail: true }
    avif_p95_ms:   { max: 7.0, block_on_fail: true }
    batch_img_sec: { min: 550, block_on_fail: true }
    cache_p99_ms:  { max: 1.5, block_on_fail: true }
    peak_mem_mb:   { max: 120, block_on_fail: true }
    idle_mem_mb:   { max: 22, block_on_fail: true }
```

### Final v34.7.0 Performance Targets

| Metric | v33.0.0 | v34.7.0 Target | Improvement |
|--------|---------|----------------|-------------|
| JPEG 6MP P50 | 4.2 ms | **< 1.5 ms** | 2.8× |
| PNG 4K P50 | 5.8 ms | **< 2.0 ms** | 2.9× |
| WebP P50 | 6.3 ms | **< 2.5 ms** | 2.5× |
| AVIF P50 | 9.1 ms | **< 3.5 ms** | 2.6× |
| JPEG XL P50 | 11.2 ms | **< 5.0 ms** | 2.2× |
| PDF P50 | 17.0 ms | **< 8.0 ms** | 2.1× |
| RAW 24MP P50 | 22.5 ms | **< 9.0 ms** | 2.5× |
| DWG/DXF P50 | N/A | **< 50 ms** | New |
| STEP/IFC P50 | N/A | **< 100 ms** | New |
| DICOM + windowing P50 | ~15 ms | **< 8 ms** | 1.9× |
| Batch throughput | 235 img/s | **> 600 img/s** | 2.6× |
| Cache hit P50 | 0.8 ms | **< 0.3 ms** | 2.7× |
| Cache hit P99 | 4.5 ms | **< 1.0 ms** | 4.5× |
| Pre-gen cache-hit rate | 0% | **> 95%** | ∞ |
| Idle memory | ~24 MB | **< 20 MB** | 17% lower |
| Peak memory (100 concurrent) | ~140 MB | **< 100 MB** | 29% lower |
| File extensions supported | 200+ | **350+** | 75% more |
| Decoder classes | 179 | **220+** | 23% more |

### Reliability Targets

| Metric | Target |
|--------|--------|
| Crash-free sessions | ≥ 99.999% (< 1 crash per 100K sessions) |
| Test coverage (line) | ≥ 95% |
| Fuzz corpus branch coverage | ≥ 87% across all decoders |
| Build reproducibility | 100% bit-identical |
| P99 latency regression gate | < 5% per PR (automated) |
| Zero-warning build | 0 warnings on MSVC v145 + Clang-tidy |
| Mean time to thumbnail (MTTT) | < 3 ms for cached, < 15 ms cold (mixed format) |

---

## Competitive Landscape

### Thumbnail Generation Comparison

| Feature | ExplorerLens v34 | Windows Shell | macOS QL | IrfanView | XnView | FastStone |
|---------|-----------------|---------------|----------|-----------|--------|-----------|
| File extensions | **350+** | ~40 | ~60 | ~120 | ~150 | ~70 |
| GPU decode | **Full pipeline** | Limited (WIC) | Metal only | None | None | None |
| HDR tonemap | **Gainmap + PQ + HLG** | Basic HDR10 | Gainmap | None | None | None |
| CAD formats | **DWG/STEP/IFC** | None | None | None | None | None |
| Scientific | **DICOM/FITS/HDF5** | None | None | Limited | Limited | None |
| Animated preview | **Hover-scrub** | None | None | None | None | None |
| Predictive pre-gen | **Yes** | No | No | No | No | No |
| Cross-platform | **Win + macOS + Linux** | Windows only | macOS only | Windows | Win + Linux | Windows |
| Batch P50 | **600 img/s** | ~50 img/s | ~80 img/s | ~100 img/s | ~80 img/s | ~60 img/s |
| Cache hit latency | **0.3 ms** | ~5 ms | ~3 ms | N/A | N/A | N/A |

---

## New External Library Dependencies

| Library | Version | Formats | License | Size |
|---------|---------|---------|---------|------|
| libultrahdr | 1.3+ | Google Ultra HDR JPEG | Apache-2.0 | ~2 MB |
| basisu (transcoder) | 1.16+ | Basis Universal / KTX2 | Apache-2.0 | ~1 MB |
| OpenCASCADE (OCCT) | 7.8+ | STEP/IGES | LGPL-2.1 | ~40 MB |
| IfcOpenShell | 0.8+ | IFC (BIM) | LGPL-3.0 | ~12 MB |
| openNURBS | 8.x | Rhino 3DM | MIT | ~5 MB |
| ODA Drawings SDK | 25.x | DWG/DXF | Commercial/OSS | ~15 MB |
| LAStools / PDAL | 2.7+ | LAS/LAZ point cloud | BSD-3 | ~8 MB |
| libe57 | 2.3+ | E57 point cloud | MIT | ~3 MB |
| libCZI | 0.60+ | Zeiss CZI microscopy | MIT | ~2 MB |
| NVJPEG | CUDA 12.6+ | GPU JPEG decode | NVIDIA EULA | Runtime dep |
| Intel oneVPL | 2024.2+ | QSV decode | MIT | Runtime dep |
| AMD AMF | 1.4.35+ | AMD GPU decode | MIT | Runtime dep |

---

## Risks & Mitigations

| # | Risk | Likelihood | Impact | Mitigation |
|---|------|-----------|--------|------------|
| 1 | ODA licensing cost for DWG | Medium | High | Negotiate OSS tier; fallback to LibreDWG in GPL-isolated process |
| 2 | OpenCASCADE binary size (40 MB) | Low | Medium | Conditionally link; lazy-load on first STEP/IGES file; split into separate DLL |
| 3 | GPU driver bugs in decode path | Medium | Medium | CPU fallback for every GPU path; driver version allowlist |
| 4 | Pre-generation battery drain on laptops | Medium | Low | Aggressive throttling on battery; disable below 20% |
| 5 | HEIC Live Photo decode latency (MOV demux) | Low | Low | Extract key frame only; skip video unless hover |
| 6 | ICC v5 (iccMAX) library maturity | Medium | Low | Fallback to ICC v4 transform; log warning |
| 7 | Fuzz coverage regression for new decoders | Medium | High | Mandatory fuzz corpus for each new format before merge |

---

## Success Criteria for v34.7.0 Release

| Gate | Requirement |
|------|-------------|
| ✅ Format | 350+ file extensions with green-light test coverage |
| ✅ Performance | All P95 targets met on CI benchmark runner |
| ✅ GPU | NVIDIA + Intel + AMD decode paths verified on hardware |
| ✅ HDR | Gainmap, PQ, HLG all produce visually-correct SDR thumbnails |
| ✅ CAD | DWG + STEP + IFC render at ISO-standard view angles |
| ✅ Pre-Gen | 95%+ cache hit rate in folder-browse scenario |
| ✅ Memory | Idle < 20 MB, Peak < 100 MB |
| ✅ Stability | 0 crashes in 100K-session soak test |
| ✅ Build | 0 errors, 0 warnings, all platforms green |
| ✅ Tests | 5,500+ unit tests, 100% pass rate |
| ✅ Security | All new decoders pass 72-hour fuzz campaign |

---

## Relationship to Previous Plans

| Plan | Versions | Status |
|------|----------|--------|
| `ROADMAP_V30.md` | v25.x–v33.x | v30–v32 Completed, v33.x Active |
| `ROADMAP_V34.md` | v34.x "Arcturus" | **This document** |
| Future: `ROADMAP_V35.md` | v35.x "Vega" | Streaming & Cloud-Native Thumbnails |
