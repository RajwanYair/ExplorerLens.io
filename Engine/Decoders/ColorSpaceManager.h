#pragma once
//==============================================================================
// DarkThumbs — Sprint 40: Color Space Awareness & HDR Tone Mapping
// ICC profile extraction, gamut mapping, HDR→SDR conversion, color accuracy.
//==============================================================================

#include <string>
#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <array>
#include <functional>

namespace DarkThumbs::Engine::Decoders {

//------------------------------------------------------------------------------
// Color Space Identifiers
//------------------------------------------------------------------------------
enum class ColorSpace : uint8_t {
    Unknown = 0,
    sRGB,           // Standard web/display (IEC 61966-2-1)
    DisplayP3,      // Apple wide gamut (DCI-P3 primaries, sRGB gamma)
    AdobeRGB,       // Adobe RGB (1998)
    ProPhotoRGB,    // ROMM RGB — Lightroom default
    Rec709,         // ITU-R BT.709 (HDTV)
    Rec2020,        // ITU-R BT.2020 (UHDTV / HDR)
    ACES,           // Academy Color Encoding System (scene-referred)
    LinearRGB,      // Linear sRGB (no gamma)
    CMYK_Fogra39,   // CMYK (print, Fogra39 characterization)
    LAB_D50,        // CIE L*a*b* under D50 illuminant
    XYZ_D50,        // CIE XYZ under D50 illuminant
    Custom          // Unknown ICC profile — use profile bytes directly
};

inline const char* ColorSpaceName(ColorSpace cs) {
    switch (cs) {
        case ColorSpace::sRGB:          return "sRGB";
        case ColorSpace::DisplayP3:     return "Display P3";
        case ColorSpace::AdobeRGB:      return "Adobe RGB (1998)";
        case ColorSpace::ProPhotoRGB:   return "ProPhoto RGB";
        case ColorSpace::Rec709:        return "Rec.709";
        case ColorSpace::Rec2020:       return "Rec.2020";
        case ColorSpace::ACES:          return "ACES";
        case ColorSpace::LinearRGB:     return "Linear sRGB";
        case ColorSpace::CMYK_Fogra39:  return "CMYK (Fogra39)";
        case ColorSpace::LAB_D50:       return "CIE L*a*b*";
        case ColorSpace::XYZ_D50:       return "CIE XYZ";
        case ColorSpace::Custom:        return "Custom ICC Profile";
        default:                        return "Unknown";
    }
}

inline bool IsWideGamut(ColorSpace cs) {
    return cs == ColorSpace::DisplayP3 ||
           cs == ColorSpace::AdobeRGB ||
           cs == ColorSpace::ProPhotoRGB ||
           cs == ColorSpace::Rec2020 ||
           cs == ColorSpace::ACES;
}

inline bool IsHDR(ColorSpace cs) {
    return cs == ColorSpace::Rec2020 ||
           cs == ColorSpace::ACES ||
           cs == ColorSpace::LinearRGB;
}

//------------------------------------------------------------------------------
// ICC Profile Data
//------------------------------------------------------------------------------
struct ICCProfile {
    std::string description;
    ColorSpace colorSpace = ColorSpace::Unknown;
    std::vector<uint8_t> rawData;          // Raw ICC profile bytes
    uint32_t version = 0;                  // ICC version (e.g., 0x04300000 = v4.3)
    std::string renderingIntent = "Perceptual";  // Perceptual, Relative, Saturation, Absolute

    bool IsValid() const { return !rawData.empty() && rawData.size() >= 128; }
    uint32_t SizeBytes() const { return static_cast<uint32_t>(rawData.size()); }

    // Identify a known color space from ICC profile description
    ColorSpace IdentifyColorSpace() const {
        if (description.find("sRGB") != std::string::npos) return ColorSpace::sRGB;
        if (description.find("Display P3") != std::string::npos) return ColorSpace::DisplayP3;
        if (description.find("Adobe RGB") != std::string::npos) return ColorSpace::AdobeRGB;
        if (description.find("ProPhoto") != std::string::npos) return ColorSpace::ProPhotoRGB;
        if (description.find("Rec. 2020") != std::string::npos ||
            description.find("BT.2020") != std::string::npos) return ColorSpace::Rec2020;
        if (description.find("Rec. 709") != std::string::npos ||
            description.find("BT.709") != std::string::npos) return ColorSpace::Rec709;
        if (description.find("ACES") != std::string::npos) return ColorSpace::ACES;
        return ColorSpace::Custom;
    }
};

//------------------------------------------------------------------------------
// ICC Profile Extractor — format-specific extraction
//------------------------------------------------------------------------------
struct ICCExtractionResult {
    bool found = false;
    ICCProfile profile;
    std::string sourceFormat;  // "JPEG", "TIFF", "PNG", "PSD", "HEIF", "WebP"
    size_t profileOffset = 0;  // Byte offset in file
    std::string error;
};

// Supported extraction formats
inline std::vector<std::string> ICCSupportedFormats() {
    return { "JPEG", "TIFF", "PNG", "PSD", "HEIF", "WebP", "JXL", "AVIF", "EXR", "DNG" };
}

// JPEG ICC: APP2 marker with "ICC_PROFILE" header (multi-chunk support)
// TIFF ICC: Tag 34675 (InterColorProfile)
// PNG ICC:  iCCP chunk
// PSD ICC:  Image Resources section, resource ID 0x040F
// HEIF/AVIF: "colr" box with "prof" or "rICC" types
// JXL: color_encoding box or embedded ICC
// EXR: chromaticities attribute → derive profile
// DNG: ICC profile tag (same as TIFF)

class ICCProfileExtractor {
public:
    // Extract ICC profile from file based on format
    ICCExtractionResult Extract(const std::string& filePath, const std::string& format) const {
        ICCExtractionResult result;
        result.sourceFormat = format;

        // Format-specific extraction would happen here using real file I/O
        // For design spec, we define the contract
        auto supported = ICCSupportedFormats();
        bool isSupported = std::find(supported.begin(), supported.end(), format) != supported.end();
        if (!isSupported) {
            result.error = "Unsupported format: " + format;
            return result;
        }

        // Contract: real implementation populates result.profile from file data
        return result;
    }

    // Check if format supports ICC extraction
    bool SupportsFormat(const std::string& format) const {
        auto formats = ICCSupportedFormats();
        return std::find(formats.begin(), formats.end(), format) != formats.end();
    }
};

//------------------------------------------------------------------------------
// Color Conversion Utilities
//------------------------------------------------------------------------------

// sRGB gamma curve
inline double SRGBToLinear(double v) {
    if (v <= 0.04045) return v / 12.92;
    return std::pow((v + 0.055) / 1.055, 2.4);
}

inline double LinearToSRGB(double v) {
    if (v <= 0.0031308) return v * 12.92;
    return 1.055 * std::pow(v, 1.0 / 2.4) - 0.055;
}

// PQ (Perceptual Quantizer, SMPTE ST 2084) transfer function for HDR10
inline double PQToLinear(double v) {
    const double m1 = 0.1593017578125;
    const double m2 = 78.84375;
    const double c1 = 0.8359375;
    const double c2 = 18.8515625;
    const double c3 = 18.6875;

    double vp = std::pow(v, 1.0 / m2);
    double num = std::max(vp - c1, 0.0);
    double den = c2 - c3 * vp;
    return std::pow(num / den, 1.0 / m1);
}

// HLG (Hybrid Log-Gamma, BT.2100) transfer function
inline double HLGToLinear(double v) {
    const double a = 0.17883277;
    const double b = 0.28466892;
    const double c = 0.55991073;
    if (v <= 0.5) return (v * v) / 3.0;
    return (std::exp((v - c) / a) + b) / 12.0;
}

//------------------------------------------------------------------------------
// Gamut Mapping Engine — Wide Gamut → sRGB
//------------------------------------------------------------------------------

// 3x3 matrix for color space conversion
struct ColorMatrix3x3 {
    double m[3][3] = {};

    std::array<double, 3> Transform(double r, double g, double b) const {
        return {
            m[0][0] * r + m[0][1] * g + m[0][2] * b,
            m[1][0] * r + m[1][1] * g + m[1][2] * b,
            m[2][0] * r + m[2][1] * g + m[2][2] * b
        };
    }
};

// Standard gamut mapping matrices (verified against ICC specification)
inline ColorMatrix3x3 DisplayP3ToSRGB() {
    return {{ {
        { 1.2249,  -0.2247, 0.0 },
        { -0.0420,  1.0419, 0.0 },
        { -0.0197, -0.0786, 1.0984 }
    } }};
}

inline ColorMatrix3x3 AdobeRGBToSRGB() {
    return {{ {
        { 1.3982,  -0.3982, 0.0 },
        { 0.0,      1.0,    0.0 },
        { 0.0,     -0.0423, 1.0423 }
    } }};
}

inline ColorMatrix3x3 Rec2020ToSRGB() {
    return {{ {
        { 1.6605, -0.5877, -0.0728 },
        { -0.1246, 1.1330, -0.0084 },
        { -0.0182, -0.1006, 1.1187 }
    } }};
}

enum class GamutMappingMethod : uint8_t {
    Clip = 0,           // Simple clip to [0,1] — fast, may lose saturation
    Perceptual,         // Compress entire gamut proportionally — preserves relationships
    RelativeColorimetric, // Adjust white point, clip out-of-gamut — most accurate
    Saturation          // Maximize saturation — best for graphics/charts
};

inline const char* GamutMappingMethodName(GamutMappingMethod m) {
    switch (m) {
        case GamutMappingMethod::Clip:                  return "Clip";
        case GamutMappingMethod::Perceptual:            return "Perceptual";
        case GamutMappingMethod::RelativeColorimetric:  return "Relative Colorimetric";
        case GamutMappingMethod::Saturation:            return "Saturation";
        default:                                        return "Unknown";
    }
}

class GamutMapper {
public:
    GamutMapper(ColorSpace source, ColorSpace target = ColorSpace::sRGB,
                GamutMappingMethod method = GamutMappingMethod::Perceptual)
        : m_source(source), m_target(target), m_method(method) {}

    // Get the conversion matrix for source → target
    ColorMatrix3x3 GetMatrix() const {
        if (m_source == ColorSpace::DisplayP3 && m_target == ColorSpace::sRGB)
            return DisplayP3ToSRGB();
        if (m_source == ColorSpace::AdobeRGB && m_target == ColorSpace::sRGB)
            return AdobeRGBToSRGB();
        if (m_source == ColorSpace::Rec2020 && m_target == ColorSpace::sRGB)
            return Rec2020ToSRGB();
        // Identity for same-space or unsupported conversions
        return {{ { {1,0,0}, {0,1,0}, {0,0,1} } }};
    }

    // Map a single pixel (linear RGB values)
    std::array<double, 3> MapPixel(double r, double g, double b) const {
        auto matrix = GetMatrix();
        auto result = matrix.Transform(r, g, b);
        // Apply gamut mapping method
        if (m_method == GamutMappingMethod::Clip) {
            result[0] = std::clamp(result[0], 0.0, 1.0);
            result[1] = std::clamp(result[1], 0.0, 1.0);
            result[2] = std::clamp(result[2], 0.0, 1.0);
        }
        return result;
    }

    bool NeedsConversion() const { return m_source != m_target; }
    ColorSpace Source() const { return m_source; }
    ColorSpace Target() const { return m_target; }
    GamutMappingMethod Method() const { return m_method; }

private:
    ColorSpace m_source;
    ColorSpace m_target;
    GamutMappingMethod m_method;
};

//------------------------------------------------------------------------------
// HDR Tone Mapping — HDR → SDR
//------------------------------------------------------------------------------

enum class ToneMappingOperator : uint8_t {
    Reinhard = 0,       // Simple Reinhard (x / (1 + x))
    ReinhardExtended,   // Extended Reinhard with white point
    ACES_Filmic,        // Academy Color Encoding System filmic curve
    Hable,              // Uncharted 2 / Hable tone map
    Lottes,             // Timothy Lottes (AMD) curve
    Linear_Clamp        // Linear scale + clamp (baseline)
};

inline const char* ToneMappingOperatorName(ToneMappingOperator op) {
    switch (op) {
        case ToneMappingOperator::Reinhard:         return "Reinhard";
        case ToneMappingOperator::ReinhardExtended:  return "Reinhard Extended";
        case ToneMappingOperator::ACES_Filmic:      return "ACES Filmic";
        case ToneMappingOperator::Hable:            return "Hable (Uncharted 2)";
        case ToneMappingOperator::Lottes:           return "Lottes (AMD)";
        case ToneMappingOperator::Linear_Clamp:     return "Linear Clamp";
        default:                                    return "Unknown";
    }
}

struct HDRMetadata {
    double maxContentLightLevel = 1000.0;  // MaxCLL (nits)
    double maxFrameAverage = 400.0;        // MaxFALL (nits)
    double displayMaxBrightness = 80.0;    // Target SDR display (nits)
    ColorSpace sourceSpace = ColorSpace::Rec2020;
    bool hasPQ = false;     // SMPTE ST 2084 (HDR10)
    bool hasHLG = false;    // Hybrid Log-Gamma (HLG)

    bool IsHDR() const { return maxContentLightLevel > 100.0 || hasPQ || hasHLG; }
    double DynamicRange() const {
        return (displayMaxBrightness > 0) ? maxContentLightLevel / displayMaxBrightness : 1.0;
    }
};

class ToneMapper {
public:
    ToneMapper(ToneMappingOperator op = ToneMappingOperator::ACES_Filmic,
               double exposure = 1.0)
        : m_operator(op), m_exposure(exposure) {}

    // Tone map a single luminance value (linear, scene-referred)
    double ToneMap(double luminance) const {
        double v = luminance * m_exposure;
        switch (m_operator) {
            case ToneMappingOperator::Reinhard:
                return v / (1.0 + v);
            case ToneMappingOperator::ReinhardExtended: {
                double Lw = m_whitePoint;
                return (v * (1.0 + v / (Lw * Lw))) / (1.0 + v);
            }
            case ToneMappingOperator::ACES_Filmic: {
                // Simplified ACES filmic curve (Krzysztof Narkowicz approximation)
                double a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
                return std::clamp((v * (a * v + b)) / (v * (c * v + d) + e), 0.0, 1.0);
            }
            case ToneMappingOperator::Hable: {
                auto hable = [](double x) {
                    double A = 0.15, B = 0.50, C = 0.10, D = 0.20, E = 0.02, F = 0.30;
                    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
                };
                double w = 11.2;
                return hable(v) / hable(w);
            }
            case ToneMappingOperator::Lottes: {
                double a = 1.6, d = 0.977, hdrMax = 8.0, midIn = 0.18, midOut = 0.267;
                double b = (-std::pow(midIn, a) + std::pow(hdrMax, a) * midOut) /
                           ((std::pow(hdrMax, a * d) - std::pow(midIn, a * d)) * midOut);
                double c_val = (std::pow(hdrMax, a * d) * std::pow(midIn, a) -
                           std::pow(hdrMax, a) * std::pow(midIn, a * d) * midOut) /
                           ((std::pow(hdrMax, a * d) - std::pow(midIn, a * d)) * midOut);
                return std::pow(v, a) / (std::pow(v, a * d) * b + c_val);
            }
            case ToneMappingOperator::Linear_Clamp:
            default:
                return std::clamp(v, 0.0, 1.0);
        }
    }

    // Tone map RGB triplet
    std::array<double, 3> ToneMapRGB(double r, double g, double b) const {
        return { ToneMap(r), ToneMap(g), ToneMap(b) };
    }

    void SetExposure(double e) { m_exposure = e; }
    double Exposure() const { return m_exposure; }
    void SetWhitePoint(double wp) { m_whitePoint = wp; }
    double WhitePoint() const { return m_whitePoint; }
    ToneMappingOperator Operator() const { return m_operator; }

private:
    ToneMappingOperator m_operator;
    double m_exposure = 1.0;
    double m_whitePoint = 4.0; // For extended Reinhard
};

//------------------------------------------------------------------------------
// Color Accuracy Metrics — dE2000
//------------------------------------------------------------------------------

struct LABColor {
    double L = 0, a = 0, b = 0;
};

// CIE dE2000 color difference (simplified)
inline double DeltaE2000(const LABColor& ref, const LABColor& sample) {
    double dL = sample.L - ref.L;
    double da = sample.a - ref.a;
    double db = sample.b - ref.b;
    // Simplified Euclidean approximation (full dE2000 is ~100 lines)
    return std::sqrt(dL * dL + da * da + db * db);
}

struct ColorAccuracyResult {
    double dE2000_mean = 0;
    double dE2000_max = 0;
    double dE2000_p95 = 0;
    uint32_t sampleCount = 0;
    bool passesThreshold = false;
    double threshold = 2.0; // dE2000 < 2.0 = visually indistinguishable

    std::string Summary() const {
        return "dE2000: mean=" + std::to_string(dE2000_mean).substr(0, 5) +
               " max=" + std::to_string(dE2000_max).substr(0, 5) +
               " p95=" + std::to_string(dE2000_p95).substr(0, 5) +
               " (" + std::to_string(sampleCount) + " samples)" +
               (passesThreshold ? " PASS" : " FAIL");
    }
};

//------------------------------------------------------------------------------
// Windows Color System (WCS) Integration
//------------------------------------------------------------------------------

struct WCSConfig {
    bool useWCS = true;          // Use Windows Color Management API
    bool preserveBlackPoint = true;
    std::string targetProfile = "sRGB";  // Output profile
    GamutMappingMethod intent = GamutMappingMethod::Perceptual;

    // Windows HDR display detection
    bool isHDRDisplayActive = false;  // AdvancedColorInfo.CurrentAdvancedColorKind
    double sdrWhiteLevel = 80.0;      // SDR content brightness on HDR display
};

//------------------------------------------------------------------------------
// Color Pipeline Configuration
//------------------------------------------------------------------------------

struct ColorPipelineConfig {
    bool enableColorManagement = true;
    bool extractICCProfiles = true;
    bool applyGamutMapping = true;
    bool applyToneMapping = true;
    ColorSpace outputSpace = ColorSpace::sRGB;
    ToneMappingOperator toneMapper = ToneMappingOperator::ACES_Filmic;
    GamutMappingMethod gamutMethod = GamutMappingMethod::Perceptual;
    double hdrExposure = 1.0;
    double maxDeltaE = 2.0; // Accuracy threshold
    WCSConfig wcs;

    static ColorPipelineConfig Default() {
        return {};
    }

    static ColorPipelineConfig Disabled() {
        ColorPipelineConfig c;
        c.enableColorManagement = false;
        c.extractICCProfiles = false;
        c.applyGamutMapping = false;
        c.applyToneMapping = false;
        return c;
    }

    static ColorPipelineConfig HDR() {
        ColorPipelineConfig c;
        c.toneMapper = ToneMappingOperator::ACES_Filmic;
        c.hdrExposure = 1.2;
        c.outputSpace = ColorSpace::sRGB;
        return c;
    }

    static ColorPipelineConfig HighAccuracy() {
        ColorPipelineConfig c;
        c.gamutMethod = GamutMappingMethod::RelativeColorimetric;
        c.maxDeltaE = 1.0;
        return c;
    }
};

//------------------------------------------------------------------------------
// Per-Format Color Space Defaults
//------------------------------------------------------------------------------
inline ColorSpace DefaultColorSpaceForFormat(const std::string& format) {
    if (format == "HEIF" || format == "HEIC") return ColorSpace::DisplayP3;
    if (format == "EXR") return ColorSpace::LinearRGB;
    if (format == "HDR") return ColorSpace::LinearRGB;
    if (format == "DNG") return ColorSpace::ProPhotoRGB;
    if (format == "PSD") return ColorSpace::AdobeRGB;
    if (format == "AVIF") return ColorSpace::sRGB; // Can be P3 too
    if (format == "JXL") return ColorSpace::sRGB;
    return ColorSpace::sRGB;
}

} // namespace DarkThumbs::Engine::Decoders
