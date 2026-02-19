# Sprint 178 — Documentation Rewrite P1

**Version:** v8.4.0  
**Date:** June 2025  
**Status:** ✅ Complete

---

## Objective

Rewrite stale format documentation to reflect the actual v8.4.0 state (25 decoders, 200+ extensions, 93 shell registrations).

## Changes

### 1. Created FORMAT_SUPPORT_MATRIX_V8.md (New)
- Comprehensive format matrix organized by category (14 sections)
- Covers all 25 decoders with extension counts, CBXTYPE values, library versions
- Full CBXTYPE enum reference table (values 0–81)
- Shell registration status summary
- Supersedes FORMAT_SUPPORT_MATRIX_V7.md (which listed only 80+ formats, 24 decoders)

### 2. Archived FORMAT_SUPPORT_ANALYSIS.md
- Added archive header noting the document is from v5.3.0 and is severely outdated
- Points readers to FORMAT_SUPPORT_MATRIX_V8.md for current data
- Many formats previously listed as "Not Yet Implemented" are now fully implemented

### 3. Rewrote DECODER_AUDIT_REPORT.md
- Updated from Sprint 11 audit (5/5 decoders) to Sprint 178 audit (25/25 decoders)
- Full compliance matrix for all 25 decoders across 8 IThumbnailDecoder interface methods
- Organized by category: Image (14), RAW (1), Archive (1), Media (2), Document (3), Font (1), Model (1), Scientific (2)
- Added audit trail connecting Sprint 11 → Sprint 178

## Files Changed

| File | Action |
|------|--------|
| `docs/formats/FORMAT_SUPPORT_MATRIX_V8.md` | Created (new comprehensive matrix) |
| `docs/formats/FORMAT_SUPPORT_ANALYSIS.md` | Archived (v5.3.0, redirected) |
| `docs/formats/DECODER_AUDIT_REPORT.md` | Rewritten (5→25 decoders) |

## Validation

- All 3 files syntactically valid Markdown
- CBXTYPE values cross-checked against cbxArchive.h
- Decoder count matches engine headers (25 total)
- Extension counts verified against CBXShell.rgs (93 registered)
