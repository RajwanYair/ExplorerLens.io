# Sprint 255: DPX/Cineon Film Format Decoder

**Date:** 2026-02-20
**Version:** v11.0.0
**Phase:** Phase 2 — New Format Decoders

## Objective
Production DPX (SMPTE 268M) and Cineon decoder with 10-bit film log support. Magic byte detection for both big/little-endian DPX and Cineon formats. Logarithmic-to-linear conversion for proper film scan display.

## Deliverables
- `Engine/Decoders/DPXDecoder.h` — DPX/Cineon decoder with DPXHeader, CineonHeader, IThumbnailDecoder interface
- DPXTransfer enum (6 transfer types: UserDefined, PrintingDensity, Linear, Log, UnspecifiedVideo, LogFilm)
- IsDPXFile() and IsCineonFile() magic byte detection
- LogToLinear() 10-bit log to 8-bit linear conversion
- 5 unit tests validating magic, transfer, conversion

## Technical Details
- DPX magic: 0x53445058 (BE "SDPX") or 0x58504453 (LE "XPDS")
- Cineon magic: 0x802A5FD7 (BE) or 0xD75F2A80 (LE)
- Log-to-linear uses gamma 2.2 conversion from 10-bit [0,1023] to 8-bit [0,255]

## Test Results
- TestDPX_MagicBytes ✅
- TestDPX_CineonMagic ✅
- TestDPX_TransferNames ✅
- TestDPX_LogToLinear ✅
- TestDPX_TransferCount ✅
