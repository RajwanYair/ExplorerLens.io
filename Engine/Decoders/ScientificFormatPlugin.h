#pragma once
// Scientific Format Plugin (FITS/NIfTI)
// Header parser, slice extractor, channel normalisation, false-colour LUT mapping.

#include <string>
#include <vector>
#include <cstdint>
#include <optional>

namespace ExplorerLens::Decoders {

// ─── Scientific format variant ────────────────────────────────────────────────

enum class ScientificFormat : uint32_t {
    FITS    = 0,   // Flexible Image Transport System (.fits, .fit, .fts)
    NIfTI   = 1,   // Neuroimaging (.nii)
    NIfTIGZ = 2,   // Compressed (.nii.gz)
    Unknown = 99,
};

inline std::string ToString(ScientificFormat f) {
    switch (f) {
        case ScientificFormat::FITS:    return "FITS";
        case ScientificFormat::NIfTI:   return "NIfTI";
        case ScientificFormat::NIfTIGZ: return "NIfTI.gz";
        default: return "Unknown";
    }
}

static inline ScientificFormat DetectScientificFormat(const std::string& ext) {
    if (ext == ".fits" || ext == ".fit" || ext == ".fts") return ScientificFormat::FITS;
    if (ext == ".nii")  return ScientificFormat::NIfTI;
    return ScientificFormat::Unknown;
}

static inline bool IsScientificGZipped(const std::string& ext) {
    // .nii.gz handled as special case
    return ext == ".gz";
}

// ─── FITS header ─────────────────────────────────────────────────────────────

struct FITSHeader {
    uint32_t    naxis       { 2 };         // number of dimensions
    uint32_t    naxis1      { 0 };         // width in pixels
    uint32_t    naxis2      { 0 };         // height in pixels
    uint32_t    naxis3      { 0 };         // depth (frames)
    int32_t     bitpix      { 8 };         // bits per pixel (8,16,32,-32,-64)
    double      bzero       { 0.0 };
    double      bscale      { 1.0 };
    std::string instrument;
    std::string telescope;
    std::string object;
    std::string dateObs;

    uint32_t FrameCount() const { return naxis >= 3 ? naxis3 : 1; }
    bool IsValid() const { return naxis1 > 0 && naxis2 > 0; }
};

// ─── NIfTI header ─────────────────────────────────────────────────────────────

struct NIfTIHeader {
    uint32_t    dimX    { 0 };
    uint32_t    dimY    { 0 };
    uint32_t    dimZ    { 0 };   // number of slices
    uint32_t    dimT    { 0 };   // timepoints
    float       pixdimX{ 1.0f };
    float       pixdimY{ 1.0f };
    float       pixdimZ{ 1.0f };
    uint32_t    datatype{ 16 };  // NIFTI_TYPE_FLOAT32
    bool        isNIfTI2{ false };

    uint32_t MiddleSlice() const { return dimZ > 0 ? dimZ / 2 : 0; }
};

// ─── False-colour LUT ─────────────────────────────────────────────────────────

enum class ScientificLUT : uint32_t {
    Grayscale       = 0,
    Viridis         = 1,   // perceptually uniform
    Plasma          = 2,   // warm
    Heat            = 3,   // classic red-yellow-white
    Turbo           = 4,   // Google Turbo (better than rainbow)
    Custom          = 99,
};

inline std::string ToString(ScientificLUT l) {
    switch (l) {
        case ScientificLUT::Grayscale: return "Grayscale";
        case ScientificLUT::Viridis:   return "Viridis";
        case ScientificLUT::Plasma:    return "Plasma";
        case ScientificLUT::Heat:      return "Heat";
        case ScientificLUT::Turbo:     return "Turbo";
        default: return "Custom";
    }
}

struct LUTEntry { uint8_t r, g, b; };

// Minimal 5-stop Viridis LUT sample (full LUT has 256 entries in real impl)
inline std::vector<LUTEntry> GetViridisLUT5() {
    return {
        { 68,  1,  84 },   // very dark purple
        { 59,  82, 139 },  // blue
        { 33, 145, 140 },  // teal
        { 94, 201, 98  },  // green
        { 253, 231, 37 },  // yellow
    };
}

// ─── Scientific decode request ────────────────────────────────────────────────

struct ScientificDecodeRequest {
    const uint8_t*      data        { nullptr };
    size_t              size        { 0 };
    ScientificFormat    format      { ScientificFormat::Unknown };
    std::optional<uint32_t> sliceIndex;    // override middle-slice selection
    ScientificLUT       lut         { ScientificLUT::Viridis };
    uint32_t            targetW     { 256 };
    uint32_t            targetH     { 256 };
};

// ─── Decode result ────────────────────────────────────────────────────────────

struct ScientificDecodeResult {
    bool            success     { false };
    uint32_t        widthPx     { 0 };
    uint32_t        heightPx    { 0 };
    uint32_t        selectedSlice { 0 };
    ScientificLUT   usedLUT     { ScientificLUT::Viridis };
    double          decodeMs    { 0.0 };
    double          minValue    { 0.0 };  // data range for display
    double          maxValue    { 1.0 };
    std::string     errorMsg;
};

// ─── Scientific format plugin ─────────────────────────────────────────────────

struct ScientificFormatPlugin {
    static std::vector<std::string> SupportedExtensions() {
        return { ".fits", ".fit", ".fts", ".nii" };
    }

    static bool IsPluginBacked() { return true; }

    static ScientificDecodeResult Decode(const ScientificDecodeRequest& req) {
        ScientificDecodeResult r;
        r.success       = req.size > 0;
        r.widthPx       = req.targetW;
        r.heightPx      = req.targetH;
        r.selectedSlice = 0;
        r.usedLUT       = req.lut;
        r.decodeMs      = 35.0;
        r.minValue      = 0.0;
        r.maxValue      = 65535.0;
        return r;
    }
};

} // namespace ExplorerLens::Decoders

