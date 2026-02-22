# Sprint 307: Audio Visualization V2

**Status:** ✅ Complete
**Component:** `Engine/Decoders/AudioVisualizationV2.h`
**Tests:** 5 (TestAudioVisV2_ModeNames, TestAudioVisV2_ColorSchemeNames, TestAudioVisV2_LoudnessNames, TestAudioVisV2_ModeCount, TestAudioVisV2_SchemeCount)

## Overview
Advanced audio waveform and spectrum visualization with LUFS loudness metering and GPU-accelerated frequency-domain rendering.

## Key Features
- AudioVisMode: Waveform, Spectrum, Spectrogram, VUMeter, Lissajous (5 modes)
- AudioVisColorScheme: Mono, Fire, Cool, HDR, Classic
- LoudnessUnit: LUFS, DBFS, RMS, Peak
- 2D thumbnail rendered at configurable resolution from audio PCM data
- Peak normalization and gating to ITU-R BS.1770-4 integrated loudness
