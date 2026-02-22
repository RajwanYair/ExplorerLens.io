# Sprint 321: Thumbnail Animation Engine V2

**Status:** ✅ Complete
**Component:** `Engine/Core/ThumbnailAnimationEngineV2.h`
**Tests:** 5 (TestAnimEngV2_FormatNames, TestAnimEngV2_LoopModeNames, TestAnimEngV2_InterpNames, TestAnimEngV2_FormatCount, TestAnimEngV2_LoopCount)

## Overview
Animated thumbnail playback for GIF, APNG, WebP animated, AVIF sequences, and HEIF image collections with hover-to-play shell integration.

## Key Features
- AnimThumbnailFormat: GIF, APNG, WebPAnim, AVIFSeq, HEIFBurst, LottieJSON (6 formats)
- AnimLoopMode: Once, Loop, PingPong, FirstFrame, LastFrame
- AnimInterpolation: None, Linear, Ease, EaseInOut, Spring
- Hover-triggered playback via IShellItemImageFactory extended interface
