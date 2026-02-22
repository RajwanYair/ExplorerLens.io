# Sprint 341: GPU Decode Acceleration V2

**Status:** ✅ Complete
**Component:** `Engine/GPU/GPUDecodeAccelerationV2.h`
**Tests:** 5 (TestGPUDecAccV2_VendorNames, TestGPUDecAccV2_APINames, TestGPUDecAccV2_CodecNames, TestGPUDecAccV2_VendorCount, TestGPUDecAccV2_APICount)

## Overview
Hardware-accelerated video and image decode routing via NVDEC (NVIDIA), Intel Quick Sync, AMD AMF, and MediaFoundation platform decoder.

## Key Features
- GPUDecodeVendor: NVIDIA, AMD, Intel, Qualcomm, Apple, Generic (6 vendors)
- GPUDecodeAPI: DXVA2, D3D11VA, NVDEC, QuickSync, AMF, MediaFoundation, Vulkan
- GPUDecodeCodec: H264, H265, AV1, VP9, JPEG, TIFF, WebP, RAW (8 codecs)
- Capability discovery at startup with vendor-specific quirk table
