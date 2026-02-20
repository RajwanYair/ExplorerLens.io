# Sprint 215 — Animated Thumbnail Engine

**Sprint Number:** 215  
**Version:** v10.1.0  
**Status:** ✅ Complete

## Objective
Extract representative frames from animated image formats (GIF, APNG, WebP, AVIF sequences, JPEG XL animations) for thumbnail generation.

## Files Changed
- `Engine/Core/AnimatedThumbnailEngine.h` — AnimatedFormat, FrameStrategy enums, AnimationInfo struct
- `Engine/Core/AnimatedThumbnailEngine.cpp` — Frame extraction, format probing, strategy selection
- `Engine/CMakeLists.txt` — Registered header and source
- `Engine/Tests/EngineTests.cpp` — 5 unit tests added

## Tests Added (5)
1. `TestAnim_FormatNames` — Format name strings
2. `TestAnim_StrategyNames` — Strategy name strings
3. `TestAnim_FrameSelection` — Frame selection for First/Middle/MostDetail
4. `TestAnim_FormatDetection` — Extension-based format detection
5. `TestAnim_FormatCount` — Format count and defaults

## Key Features
- 6 animated formats: GIF, APNG, WebP Animation, AVIF Sequence, JXL Animation, FLIF
- 5 frame selection strategies: First, Middle, Keyframe, MostDetail, Composite
- GIF header probing for dimensions
- Configurable max frame scan limit (default 100)
