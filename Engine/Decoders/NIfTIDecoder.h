//==============================================================================
// DarkThumbs Engine — Sprint 287: NIfTI Neuroimaging Decoder
// Neuroimaging Informatics Technology Initiative (.nii/.nii.gz) decoder.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// NIfTI data type codes
enum class NIfTIDataType : uint8_t {
    UInt8,          // DT_UINT8 (2)
    Int16,          // DT_INT16 (4)
    Int32,          // DT_INT32 (8)
    Float32,        // DT_FLOAT32 (16)
    Float64,        // DT_FLOAT64 (64)
    Complex64,      // DT_COMPLEX64 (32)
    RGB24,          // DT_RGB24 (128)
    COUNT
};

/// NIfTI slice orientation
enum class NIfTISlice : uint8_t {
    Axial,          // Transverse / horizontal
    Coronal,        // Front view
    Sagittal,       // Side view
    COUNT
};

/// NIfTI file variant
enum class NIfTIVariant : uint8_t {
    NIfTI1,         // .nii (NIfTI-1)
    NIfTI2,         // .nii (NIfTI-2, 64-bit)
    Analyze75,      // Legacy Analyze 7.5 (.hdr/.img)
    GZipped,        // .nii.gz compressed
    COUNT
};

/// NIfTI header info
struct NIfTIHeaderInfo {
    NIfTIVariant variant    = NIfTIVariant::NIfTI1;
    NIfTIDataType dataType  = NIfTIDataType::Float32;
    uint16_t dim[8]         = {};       // dimension array
    float    pixdim[8]      = {};       // voxel size
    float    sclSlope       = 1.0f;
    float    sclInter       = 0.0f;
    uint32_t voxOffset      = 352;      // NIfTI-1 default
    std::wstring description;
};

/// NIfTI preview config
struct NIfTIPreviewConfig {
    NIfTISlice slice        = NIfTISlice::Axial;
    float      sliceRatio   = 0.5f;     // 0-1: which slice (middle default)
    bool       showColorBar = true;
    bool       autoContrast = true;
    uint32_t   windowWidth  = 256;
    uint32_t   windowLevel  = 128;
};

/// NIfTI neuroimaging decoder
class NIfTIDecoder {
public:
    static const wchar_t* DataTypeName(NIfTIDataType t) {
        switch (t) {
            case NIfTIDataType::UInt8:     return L"UInt8";
            case NIfTIDataType::Int16:     return L"Int16";
            case NIfTIDataType::Int32:     return L"Int32";
            case NIfTIDataType::Float32:   return L"Float32";
            case NIfTIDataType::Float64:   return L"Float64";
            case NIfTIDataType::Complex64: return L"Complex64";
            case NIfTIDataType::RGB24:     return L"RGB24";
            default: return L"Unknown";
        }
    }

    static const wchar_t* SliceName(NIfTISlice s) {
        switch (s) {
            case NIfTISlice::Axial:    return L"Axial";
            case NIfTISlice::Coronal:  return L"Coronal";
            case NIfTISlice::Sagittal: return L"Sagittal";
            default: return L"Unknown";
        }
    }

    static const wchar_t* VariantName(NIfTIVariant v) {
        switch (v) {
            case NIfTIVariant::NIfTI1:    return L"NIfTI-1";
            case NIfTIVariant::NIfTI2:    return L"NIfTI-2";
            case NIfTIVariant::Analyze75: return L"Analyze 7.5";
            case NIfTIVariant::GZipped:   return L"NIfTI GZipped";
            default: return L"Unknown";
        }
    }

    /// NIfTI-1 magic: "n+1\0" at offset 344 (or "ni1\0" for header-only)
    static bool CheckNIfTIMagic(const uint8_t* data, size_t size) {
        if (size < 348) return false;
        return (data[344] == 'n' && data[345] == '+' && data[346] == '1' && data[347] == '\0') ||
               (data[344] == 'n' && data[345] == 'i' && data[346] == '1' && data[347] == '\0');
    }

    static constexpr size_t DataTypeCount() { return static_cast<size_t>(NIfTIDataType::COUNT); }
    static constexpr size_t SliceCount() { return static_cast<size_t>(NIfTISlice::COUNT); }
    static constexpr size_t VariantCount() { return static_cast<size_t>(NIfTIVariant::COUNT); }
};

}} // namespace DarkThumbs::Engine
