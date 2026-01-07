# Format Strategy: The Path to 100+ Formats (Sprint 15)

**Date:** January 6, 2026
**Status:** Policy

## 1. Classification Philosophy

We classify formats into tiers based on **Frequency**, **Complexity**, and **Security Risk**.

- **Tier 1 (Core):** Built-in to `DarkThumbs.Engine.dll`. Zero-dependency (statically linked or OS/COM provided). Highest optimizations.
- **Tier 2 (Official Plugins):** Maintained by DarkThumbs team, but shipped as separate `.dtplugin` packages to keep the core light and updatable.
- **Tier 3 (Community/Experimental):** Third-party or niche formats. Sandboxed by default (Restricted Worker).

## 2. The 100+ Format Map

### 2.1 Images (Raster)

| Format | Extension | Tier | Implementation |
|---|---|---|---|
| JPEG | .jpg, .jpeg | Core | LibJpeg-Turbo (SIMD) |
| PNG | .png | Core | LibPng (Hardened) |
| WebP | .webp | Core | LibWebP (Google) |
| AVIF | .avif | Core | LibAvif (AOM) |
| JPEG-XL | .jxl | Core | LibJxl |
| HEIC/HEIF | .heic | Core | OS Codec / LibHeif |
| GIF | .gif | Core | LibNsGif |
| BMP | .bmp | Core | Internal |
| TIFF | .tiff, .tif | Tier 2 | LibTiff (Plugin) |
| TGA | .tga | Tier 2 | Plugin |
| ICO | .ico | Core | Internal |
| DDS | .dds | Tier 2 | DirectXTex (Plugin) |
| EXR | .exr | Tier 2 | TinyEXR (Plugin) |
| HDR | .hdr | Tier 2 | StbImage (Plugin) |

### 2.2 Raw Photography

*Strategy: Use LibRaw via Plugin to handle frequent camera updates.*

| Format | Extension | Tier | Implementation |
|---|---|---|---|
| Cannon | .cr2, .cr3 | Tier 2 | Pugin (LibRaw) |
| Nikon | .nef | Tier 2 | Plugin (LibRaw) |
| Sony | .arw | Tier 2 | Plugin (LibRaw) |
| Adobe | .dng | Tier 2 | Plugin (LibRaw) |
| Fuji | .raf | Tier 2 | Plugin (LibRaw) |
| Olympus | .orf | Tier 2 | Plugin (LibRaw) |
| Panasonic | .rw2 | Tier 2 | Plugin (LibRaw) |

### 2.3 Creative & Design

| Format | Extension | Tier | Implementation |
|---|---|---|---|
| Photoshop | .psd, .psb | Tier 2 | **Reference Plugin** |
| GIMP | .xcf | Tier 2 | Plugin (LibGimp) |
| Krita | .kra | Tier 2 | Plugin (Zip+Merge) |
| Clip Studio | .clip | Tier 2 | Plugin (SQLite parse) |
| Paint.NET | .pdn | Tier 2 | Plugin |
| SVG | .svg | Core | Resvg / NanoSVG |
| AI (PDF) | .ai | Tier 2 | Plugin (PDF Lib) |
| Sketch | .sketch | Tier 3 | Plugin |
| Figma | .fig | Tier 3 | Plugin (Local only) |

### 2.4 Archives (Comic Books)

| Format | Extension | Tier | Implementation |
|---|---|---|---|
| Zip Comic | .cbz | Core | Internal Zip |
| Rar Comic | .cbr | Tier 2 | UnRar Plugin (License restrictions) |
| 7z Comic | .cb7 | Tier 2 | LZMA SDK Plugin |
| Tar Comic | .cbt | Tier 2 | LibArchive Plugin |
| PDF Comic | .pdf | Core | PDFium / OS |
| EPUB | .epub | Tier 2 | Plugin (Zip+HTML) |

### 2.5 3D & CAD

| Format | Extension | Tier | Implementation |
|---|---|---|---|
| GL Transmission | .gltf, .glb | Tier 2 | Plugin (TinyGLTF) |
| Wavefront | .obj | Tier 2 | Plugin |
| FBX | .fbx | Tier 3 | Plugin (FBX SDK) |
| STL | .stl | Tier 2 | Plugin |
| Blender | .blend | Tier 2 | Plugin (BlendThumb) |
| Autodesk | .dwg, .dxf | Tier 3 | Plugin (OpenDesign) |

### 2.6 Video (Frames)

| Format | Extension | Tier | Implementation |
|---|---|---|---|
| MP4 | .mp4, .m4v | Core | MediaFoundation / FFmpeg |
| MKV | .mkv | Tier 2 | Plugin (FFmpeg) |
| WEBM | .webm | Core | Internal |
| AVI | .avi | Core | OS |
| MOV | .mov | Core | OS |

### 2.7 Documents & Code

| Format | Extension | Tier | Implementation |
|---|---|---|---|
| Markdown | .md | Tier 2 | Rendered Text Plugin |
| Source | .cpp, .cs, .js | Tier 2 | Syntax Highlight Plugin |
| Font | .ttf, .otf | Tier 2 | FreeType Plugin |

## 3. Maintenance Policy

1. **Security Updates:** Core formats must be updated within 48h of upstream CVE.
2. **Plugin Signing:** Tier 2 plugins must be signed by "DarkThumbs Official".
3. **Deprecation:** Formats under 0.1% usage may be moved from Core to Plugin to save binary size.
