# Sprint 213 — Auto Documentation Generator

**Sprint Number:** 213
**Version:** v10.1.0
**Status:** ✅ Complete

## Objective
Implement an automatic documentation generator that produces Markdown, HTML, reStructuredText, and AsciiDoc from decoder metadata and project configuration.

## Files Changed
- `Engine/Utils/AutoDocGenerator.h` — Header with DocSection, DocFormat enums, DecoderDocEntry struct, AutoDocGenerator class
- `Engine/Utils/AutoDocGenerator.cpp` — Full implementation with section generation and format support
- `Engine/CMakeLists.txt` — Registered header and source
- `Engine/Tests/EngineTests.cpp` — 5 unit tests added

## Tests Added (5)
1. `TestAutoDoc_SectionNames` — Section name resolution
2. `TestAutoDoc_FormatNames` — Format name resolution
3. `TestAutoDoc_FormatExtensions` — Format extension mapping
4. `TestAutoDoc_DecoderRegistration` — Decoder entry registration
5. `TestAutoDoc_SectionGeneration` — Markdown section generation

## Key Features
- DocSection enum: Overview, Installation, Configuration, Decoders, GPUPipeline, PluginAPI, Testing, Performance, Changelog (9 sections)
- DocFormat enum: Markdown, HTML, ReStructuredText, AsciiDoc
- DecoderDocEntry: name, description, extensions, test count, GPU acceleration flag
- Markdown table generation for decoder documentation
- Extension list aggregation and test count totals
