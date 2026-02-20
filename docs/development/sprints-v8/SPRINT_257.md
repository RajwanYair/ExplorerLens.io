# Sprint 257: Markdown/Code Preview Decoder

**Date:** 2026-02-20
**Version:** v11.0.0
**Phase:** Phase 2 — New Format Decoders

## Objective
Custom text→bitmap renderer with syntax highlighting support. Detect 20 programming languages by extension, render source code thumbnails with dark-theme syntax colors.

## Deliverables
- `Engine/Decoders/TextPreviewDecoder.h` — Text preview decoder
- TextLanguage enum (20 languages: PlainText, Markdown, C, C++, Python, JS, TS, JSON, XML, YAML, HTML, CSS, PowerShell, Batch, INI, SQL, Rust, Go, Java, C#)
- SyntaxColor struct with VS Code Dark+ inspired color scheme
- TextPreviewConfig with line number, margin, wrap settings
- DetectLanguage() supporting 31 file extensions
- 5 unit tests validating language detection, config validation, counts

## Technical Details
- 31 extension mappings covering major programming languages
- Dark background (0x1E1E1E) with VS Code color scheme
- Configurable max lines (40), line length (120), font size (10pt)
- Unique market differentiator — no other Windows thumbnail provider does code preview

## Test Results
- TestTextPreview_DetectLang ✅
- TestTextPreview_LanguageNames ✅
- TestTextPreview_IsTextFile ✅
- TestTextPreview_ValidateConfig ✅
- TestTextPreview_ExtCount ✅
