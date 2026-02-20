# Sprint 256: APNG & Animated Format Enhancement

**Date:** 2026-02-20
**Version:** v11.0.0
**Phase:** Phase 2 — New Format Decoders

## Objective
Validate APNG handling via WIC. Animated WebP/JXL first-frame extraction improvement. Unified animated format detection and frame selection strategy for thumbnails.

## Deliverables
- `Engine/Decoders/AnimatedFormatHandler.h` — Unified animated format handler
- AnimatedFormat enum (5 formats: APNG, AnimatedWebP, AnimatedJXL, AnimatedGIF, AnimatedAVIF)
- FrameStrategy enum (FirstFrame, KeyFrame, MiddleFrame, LargestFrame, Custom)
- IsAPNG() — detects animated PNG via acTL chunk search in PNG data
- SelectFrame() — chooses best thumbnail frame based on strategy
- 5 unit tests validating detection, naming, frame selection

## Technical Details
- APNG detection: PNG magic + search for 'acTL' chunk within first 4096 bytes
- Frame selection: MiddleFrame strategy selects frameCount/2 for better representative thumbnail
- Supports 5 animated formats with unified API

## Test Results
- TestAnim_DetectAPNG ✅
- TestAnim_FormatNames ✅
- TestAnim_StrategyNames ✅
- TestAnim_SelectFrame ✅
- TestAnim_FormatCount ✅
