# Sprint 136 — Extended eBook Support

## Objective
Extend eBook format coverage with MOBI/AZW3/FB2/DjVu cover extraction for thumbnail generation, complementing existing EPUB support.

## Scope
- **File**: `Engine/Decoders/EBookCoverExtractor.h`
- **Tests**: `tests/Sprint136_EBookCoverExtractor.cpp` (12 tests)

## Key Components
| Component | Purpose |
|-----------|---------|
| `EBookFormat` | 9 format enum entries (EPUB through DjVu) |
| `MOBIHeaderInfo` | PalmDB/MOBI/EXTH metadata for cover location |
| `CoverImageResult` | Extracted cover with MIME type and dimensions |
| `EBookCoverExtractor` | Unified cover extraction routing by format |

## New Format Support
| Format | Extension | Cover Source |
|--------|-----------|-------------|
| Mobipocket | .mobi | EXTH cover record (JPEG) |
| Kindle AZW | .azw | MOBI-compatible cover record |
| Kindle KF8 | .azw3 | KF8 cover resource |
| FictionBook | .fb2 | XML `<binary>` base64 image |
| DjVu | .djvu | First page render |

## Design Decisions
- DRM detection prevents wasted decode attempts on protected files
- Format-specific extraction pipelines with shared result type
- EPUB routed to existing decoder (backward compatible)
- Cover image returned as raw JPEG/PNG bytes for downstream processing

## Status: COMPLETE
