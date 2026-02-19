# Sprint 176: Shell Registration Expansion

## Objective
Expand CBXShell.rgs to register shell thumbnail handlers for all supported file extensions that were previously missing.

## Changes

### CBXShell/CBXShell.rgs
- **Previous count:** 47 registered extensions
- **New count:** 93 registered extensions (+46 new)

### New Extensions Added by Category

#### Archive/Compression (+9)
- `.tar` — TAR archives
- `.tgz` — Gzipped TAR archives
- `.bz2` — Bzip2 compressed files
- `.xz` — XZ compressed files
- `.zst` — Zstandard compressed files
- `.iso` — ISO disc images
- `.cab` — Windows Cabinet archives
- `.cpio` — CPIO archives
- `.deb` — Debian packages

#### eBook/Document (+1)
- `.djv` — DjVu alternate extension (was missing, .djvu was present)

#### Modern Image Formats (+2)
- `.avifs` — AVIF sequence format
- `.hif` — Hasselblad HEIF variant

#### Icons (+1)
- `.cur` — Windows cursor files

#### Netpbm (+2)
- `.pam` — Portable Arbitrary Map
- `.pfm` — Portable Float Map

#### Camera RAW (+14)
- `.crw` — Canon RAW (older)
- `.nrw` — Nikon RAW (compact)
- `.srf` — Sony RAW (older)
- `.sr2` — Sony RAW v2
- `.dcr` — Kodak RAW
- `.mrw` — Minolta RAW
- `.x3f` — Sigma RAW (Foveon)
- `.srw` — Samsung RAW
- `.rwl` — Leica RAW
- `.3fr` — Hasselblad RAW
- `.iiq` — Phase One RAW
- `.erf` — Epson RAW
- `.kdc` — Kodak DC RAW
- `.mef` — Mamiya RAW
- `.gpr` — GoPro RAW

#### Document Formats (+7)
- `.doc` — Microsoft Word (legacy)
- `.ppt` — Microsoft PowerPoint (legacy)
- `.xls` — Microsoft Excel (legacy)
- `.rtf` — Rich Text Format
- `.odt` — OpenDocument Text
- `.odp` — OpenDocument Presentation
- `.ods` — OpenDocument Spreadsheet
- `.xps` — XML Paper Specification

#### Font Formats (+1)
- `.ttc` — TrueType Collection

#### 3D Model Formats (+8)
- `.obj` — Wavefront OBJ
- `.stl` — Stereolithography
- `.gltf` — GL Transmission Format
- `.glb` — GL Binary
- `.fbx` — Autodesk FBX
- `.3ds` — 3D Studio
- `.ply` — Stanford PLY
- `.dae` — COLLADA

## Testing
- Shell registration script syntax verified
- All entries follow standard ATL .rgs pattern with IExtractImage + IQueryInfo handlers
- HKLM section unchanged (class registration remains the same)

## Impact
- Windows Explorer will now show DarkThumbs thumbnails for 93 file extensions (up from 47)
- No code changes required in the thumbnail provider itself — format routing was already present in cbxArchive.h
