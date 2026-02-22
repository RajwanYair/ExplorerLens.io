# Sprint 306: Extended Video Decoder

**Status:** ✅ Complete
**Component:** `Engine/Decoders/ExtendedVideoDecoder.h`
**Tests:** 5 (TestExtVideoDecoder_CodecNames, TestExtVideoDecoder_AccelNames, TestExtVideoDecoder_FrameSelectNames, TestExtVideoDecoder_CodecCount, TestExtVideoDecoder_AccelCount)

## Overview
Extended video format decoder adding HEVC 10-bit, VP9, AV1, and ProRes support with hardware-accelerated decode via DXVA2, D3D11VA, and D3D12VA.

## Key Features
- ExtVideoCodec: H264, H265, VP8, VP9, AV1, ProRes, MPEG2, MPEG4 (8 codecs)
- VideoDecodeAccel: Software, DXVA2, D3D11VA, D3D12VA, MediaFoundation
- VideoFrameSelect: FirstKeyframe, MiddleFrame, LastFrame, BestQuality
- Thumbnail extraction from compressed video without full demux
