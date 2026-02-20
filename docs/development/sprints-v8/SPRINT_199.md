# Sprint 199: Scientific Format Suite (DICOM + FITS)

**Status:** ✅ Complete  
**Version:** v10.0.0  
**Phase:** Phase 5 — Advanced Features (Sprint 1 of 6)

## Objective
Implement scientific/medical imaging format decoders for DICOM and FITS files. These are high-value niche formats used in medical imaging and astronomy respectively.

## Deliverables

### Engine/Decoders/DICOMDecoder.h + .cpp
- `DICOMPhotometric` enum (6 types: Unknown, Monochrome1/2, RGB, YBR_Full, Palette)
- `DICOMTransferSyntax` enum (6 types: ImplicitVR, ExplicitVR LE/BE, JPEG, JPEG2000, Unsupported)
- `DICOMWindowLevel` — window center/width for CT/MR visualization
- `DICOMImageInfo` — rows, columns, bits, photometric, modality, pixel data location
- `DICOMDecoder` class:
  - `IsDICOMFile()` — validates 128-byte preamble + "DICM" magic
  - `ParseHeader()` — parses DICOM data elements (group/element/VR/length/value)
  - `ApplyWindowLevel()` — maps raw pixel to 0-255 with window/level
  - `ExtractThumbnail()` — generates BGRA thumbnail with nearest-neighbor resize
  - Supports 8-bit, 16-bit mono and 8-bit RGB pixel data

### Engine/Decoders/FITSDecoder.h + .cpp
- `FITSBitpix` enum (5 types: UInt8, Int16, Int32, Float32, Float64)
- `FITSImageInfo` — NAXIS dimensions, BZERO/BSCALE, object/instrument/telescope metadata
- `FITSStretch` enum (4 algorithms: Linear, Logarithmic, SquareRoot, Asinh)
- `FITSDecoder` class:
  - `IsFITSFile()` — validates "SIMPLE  =" header
  - `ParseHeader()` — parses 80-char keyword records in 2880-byte blocks
  - `ApplyStretch()` — 4 visualization stretch algorithms
  - `ComputeMinMax()` — auto-range from sampled data
  - `ExtractThumbnail()` — big-endian pixel reading with BZERO/BSCALE transform
  - Supports all BITPIX types (8/16/32/-32/-64)

## Test Coverage
10 tests: DICOM_IsDICOMFile, DICOM_Extensions, DICOM_PhotometricNames, DICOM_TransferSyntaxNames, DICOM_WindowLevel, FITS_IsFITSFile, FITS_Extensions, FITS_BitpixNames, FITS_BytesPerPixel, FITS_StretchAlgorithm

## Files Changed
- `Engine/Decoders/DICOMDecoder.h` (new)
- `Engine/Decoders/DICOMDecoder.cpp` (new)
- `Engine/Decoders/FITSDecoder.h` (new)
- `Engine/Decoders/FITSDecoder.cpp` (new)
- `Engine/CMakeLists.txt` (registered headers + sources)
- `Engine/Tests/EngineTests.cpp` (10 tests added)
- `docs/development/sprints-v8/SPRINT_199.md` (this file)
