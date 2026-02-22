# Sprint 326: Smart Crop V2

**Status:** ✅ Complete
**Component:** `Engine/AI/SmartCropV2.h`
**Tests:** 5 (TestSmartCropV2_StrategyNames, TestSmartCropV2_AspectRatioNames, TestSmartCropV2_PaddingNames, TestSmartCropV2_StrategyCount, TestSmartCropV2_AspectCount)

## Overview
AI saliency-map based auto-crop that identifies the most visually important region across 6 crop strategies and adjustable aspect ratios.

## Key Features
- CropStrategy: Center, FaceDetect, SaliencyMap, RuleOfThirds, Golden, LargestObject (6 strategies)
- CropAspectRatio: Square, Wide16x9, Portrait3x4, Panoramic, Original, Custom
- CropPaddingMode: NoPad, Symmetric, ContentAware, Blur, SolidColor
- Integrated with SceneUnderstandingEngine to bias crop toward scene-relevant content
