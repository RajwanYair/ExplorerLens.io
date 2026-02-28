#pragma once
// HDRToneMappingPipeline.h — HDR Tone-Mapping Pipeline
// Maps HDR (Rec.2020/PQ, scRGB, EXR) content to SDR thumbnails with
// perceptually accurate brightness and color preservation.
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

/// Tone-mapping operator
enum class HDRToneMapOp : uint8_t {
 Reinhard = 0, // Global Reinhard (simple, fast)
 ReinhardExtended, // Extended Reinhard with white point
 ACES, // Academy Color Encoding System filmic
 Hable, // Uncharted 2 / Hable filmic curve
 Uchimura, // Gran Turismo tone mapper
 AgX, // AgX (modern, preserves hues)
 COUNT
};

/// HDR transfer function
enum class HDRTransferFunction : uint8_t {
 PQ = 0, // SMPTE ST 2084 Perceptual Quantizer
 HLG, // Hybrid Log-Gamma (BBC/NHK)
 Linear, // Scene-referred linear
 scRGB, // Extended sRGB (negative/super-white)
 Gamma22, // Power 2.2
 sRGB, // IEC 61966-2-1 piecewise
 COUNT
};

/// Tone-mapping configuration
struct ToneMappingConfig {
 HDRToneMapOp op = HDRToneMapOp::ACES;
 HDRTransferFunction input = HDRTransferFunction::PQ;
 float exposure = 0.0f; // EV offset (-5 to +5)
 float whitePoint = 1000.0f; // nits
 float saturation = 1.0f;
 bool adaptiveExposure = true;
 bool localToneMap = false; // local Reinhard (slower)
};

/// Tone-mapping statistics
struct ToneMappingStats {
 float peakLuminance = 0.0f; // measured peak nits
 float avgLuminance = 0.0f; // geometric mean nits
 float exposureUsed = 0.0f; // final EV
 double processingTimeMs = 0.0;
 bool wasHDR = false;
};

class HDRToneMappingPipeline {
public:
 static constexpr size_t OperatorCount() {
 return static_cast<size_t>(HDRToneMapOp::COUNT);
 }

 static const wchar_t *OperatorName(HDRToneMapOp op) {
 switch (op) {
 case HDRToneMapOp::Reinhard:
 return L"Reinhard";
 case HDRToneMapOp::ReinhardExtended:
 return L"Reinhard Extended";
 case HDRToneMapOp::ACES:
 return L"ACES Filmic";
 case HDRToneMapOp::Hable:
 return L"Hable (Uncharted 2)";
 case HDRToneMapOp::Uchimura:
 return L"Uchimura (GT)";
 case HDRToneMapOp::AgX:
 return L"AgX";
 default:
 return L"Unknown";
 }
 }

 static constexpr size_t TransferFunctionCount() {
 return static_cast<size_t>(HDRTransferFunction::COUNT);
 }

 static const wchar_t *TransferFunctionName(HDRTransferFunction tf) {
 switch (tf) {
 case HDRTransferFunction::PQ:
 return L"PQ (ST 2084)";
 case HDRTransferFunction::HLG:
 return L"HLG";
 case HDRTransferFunction::Linear:
 return L"Linear";
 case HDRTransferFunction::scRGB:
 return L"scRGB";
 case HDRTransferFunction::Gamma22:
 return L"Gamma 2.2";
 case HDRTransferFunction::sRGB:
 return L"sRGB";
 default:
 return L"Unknown";
 }
 }

 /// Reinhard global tonemapper: L_out = L / (1 + L)
 static float ReinhardSimple(float luminance) {
 return luminance / (1.0f + luminance);
 }

 /// ACES approximation (Narkowicz 2015)
 static float ACESFilmic(float x) {
 float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
 float result = (x * (a * x + b)) / (x * (c * x + d) + e);
 return result < 0.0f ? 0.0f : (result > 1.0f ? 1.0f : result);
 }

 /// PQ EOTF: ST 2084 linearize (10000 nit peak)
 static float PQToLinear(float pq) {
 float m1 = 0.1593017578125f, m2 = 78.84375f;
 float c1 = 0.8359375f, c2 = 18.8515625f, c3 = 18.6875f;
 float pw = powf(pq, 1.0f / m2);
 float num = pw - c1;
 if (num < 0.0f)
 num = 0.0f;
 float den = c2 - c3 * pw;
 return 10000.0f * powf(num / den, 1.0f / m1);
 }

 static ToneMappingConfig DefaultConfig() { return ToneMappingConfig{}; }
};

} // namespace Engine
} // namespace ExplorerLens
