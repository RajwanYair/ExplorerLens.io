# Sprint 320: Progressive Thumbnail Loader

**Status:** ✅ Complete
**Component:** `Engine/Core/ProgressiveThumbnailLoader.h`
**Tests:** 5 (TestProgLoad_StageNames, TestProgLoad_StrategyNames, TestProgLoad_PlaceholderNames, TestProgLoad_StageCount, TestProgLoad_StrategyCount)

## Overview
Multi-stage progressive loading showing instant low-resolution placeholders before requesting full-quality decode, improving perceived shell performance.

## Key Features
- ProgressiveLoadStage: Placeholder, LowRes (32×32), MedRes (128×128), FullRes, Enhanced
- ProgressiveLoadStrategy: AlwaysProgressive, OnlyLargeFiles, NetworkDrivesOnly, UserPreference
- ThumbnailPlaceholder: BlurredIcon, FileTypeIcon, DominantColor, LastKnown, Spinner
- Stage transitions triggered by viewport visibility and decode queue depth
