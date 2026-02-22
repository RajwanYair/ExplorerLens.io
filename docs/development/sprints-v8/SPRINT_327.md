# Sprint 327: Image Quality Assessor

**Status:** ✅ Complete
**Component:** `Engine/AI/ImageQualityAssessor.h`
**Tests:** 5 (TestIQA_MetricNames, TestIQA_DefectNames, TestIQA_GradeNames, TestIQA_MetricCount, TestIQA_DefectCount)

## Overview
No-reference image quality assessment scoring thumbnails on blur, noise, exposure, and composition to flag or re-render substandard outputs.

## Key Features
- IQAMetric: BRISQUE, NIQE, Sharpness, NoiseLevel, Exposure, Saturation (6 metrics)
- IQADefect: Blur, Overexposed, Underexposed, HighNoise, Compression, Moire
- IQAGrade: Reject, Poor, Acceptable, Good, Excellent
- Score-weighted ensemble across metrics; grade threshold configurable per format
