# Sprint 305: Smart Format Detector V2

**Status:** ✅ Complete
**Component:** `Engine/Pipeline/SmartFormatDetectorV2.h`
**Tests:** 5 (TestSmartDetV2_MethodNames, TestSmartDetV2_ConfNames, TestSmartDetV2_HintNames, TestSmartDetV2_MethodCount, TestSmartDetV2_ConfCount)

## Overview
Multi-signal heuristic format detection combining magic bytes, extension mapping, MIME types, and entropy analysis for high-confidence file-format identification.

## Key Features
- DetectionMethod: Extension, MagicBytes, MimeType, Entropy, Combined
- DetectionConfidence: Unknown, Low, Medium, High, Definitive
- DetectionHint: TrustExtension, CheckMagic, NeedMime, RunEntropy
- `DetectionResult` aggregates method, confidence, hint, and raw score
- `IsReliable()` helper returns true when confidence ≥ High and score ≥ 0.8
