//==============================================================================
// ExplorerLens Engine — Audio Visualization V2
// Waveform, spectrum, and album-art extraction with beatmap overlay,
// LUFS loudness metering, and animated-preview generation from audio files.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class AudioVisMode : uint8_t {
 AlbumArt = 0, // Extract embedded cover art
 Waveform, // Static waveform render
 Spectrum, // FFT spectrum bar graph
 CircularWave, // Circular/radial waveform
 Oscilloscope, // Oscilloscope view
 COUNT
};

enum class AudioVisColorScheme : uint8_t {
 Classic = 0, // Dark background, green waveform
 Neon, // Black BG, neon gradient
 Pastel, // Light theme
 Monochrome, // Grayscale
 Fire = Neon, // compat alias
 COUNT = Monochrome + 1
};

enum class LoudnessUnit : uint8_t {
 LUFS = 0, // EBU R128 integrated loudness
 RMS, // Root mean square
 Peak, // True peak
 COUNT
};

struct AudioVisConfig {
 AudioVisMode mode = AudioVisMode::Waveform;
 AudioVisColorScheme colorScheme = AudioVisColorScheme::Classic;
 uint32_t width = 256;
 uint32_t height = 256;
 bool showLoudness = false;
};

struct AudioLoudnessMeasure {
 LoudnessUnit unit = LoudnessUnit::LUFS;
 float value = -23.0f; // EBU R128 target
 float peak = 0.0f;
 bool measured = false;
};

class AudioVisualizationV2 {
public:
 static const wchar_t *ModeName(AudioVisMode m) {
 switch (m) {
 case AudioVisMode::AlbumArt:
 return L"Album Art";
 case AudioVisMode::Waveform:
 return L"Waveform";
 case AudioVisMode::Spectrum:
 return L"Spectrum";
 case AudioVisMode::CircularWave:
 return L"Circular Wave";
 case AudioVisMode::Oscilloscope:
 return L"Oscilloscope";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *ColorSchemeName(AudioVisColorScheme c) {
 switch (c) {
 case AudioVisColorScheme::Classic:
 return L"Classic";
 case AudioVisColorScheme::Neon:
 return L"Neon";
 case AudioVisColorScheme::Pastel:
 return L"Pastel";
 case AudioVisColorScheme::Monochrome:
 return L"Monochrome";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *LoudnessUnitName(LoudnessUnit u) {
 switch (u) {
 case LoudnessUnit::LUFS:
 return L"LUFS";
 case LoudnessUnit::RMS:
 return L"RMS";
 case LoudnessUnit::Peak:
 return L"True Peak";
 default:
 return L"Unknown";
 }
 }

 static constexpr size_t ModeCount() {
 return static_cast<size_t>(AudioVisMode::COUNT);
 }
 static constexpr size_t ColorSchemeCount() {
 return static_cast<size_t>(AudioVisColorScheme::COUNT);
 }
 static constexpr size_t LoudnessUnitCount() {
 return static_cast<size_t>(LoudnessUnit::COUNT);
 }

 static AudioVisConfig DefaultConfig() { return AudioVisConfig{}; }
 static bool IsLoudnessNormalized(const AudioLoudnessMeasure &m) {
 return m.value >= -24.0f && m.value <= -22.0f;
 }
};

} // namespace Engine
} // namespace ExplorerLens
