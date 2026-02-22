# Sprint 325: Scene Understanding Engine

**Status:** ✅ Complete
**Component:** `Engine/AI/SceneUnderstandingEngine.h`
**Tests:** 5 (TestSceneUnd_CategoryNames, TestSceneUnd_BackendNames, TestSceneUnd_ConfidenceNames, TestSceneUnd_CategoryCount, TestSceneUnd_BackendCount)

## Overview
ML-based scene classification producing semantic labels (indoor, nature, document, person, product, etc.) to drive intelligent thumbnail crop and layout decisions.

## Key Features
- SceneCategory: Indoor, Outdoor, Nature, Document, Person, Product, Abstract, Diagram (8 categories)
- SceneMLBackend: DirectML, ONNX, OpenVINO, CoreML, CPUFallback
- SceneConfidence: VeryLow, Low, Medium, High, VeryHigh
- Asynchronous inference with confidence threshold gating before applying scene hints
