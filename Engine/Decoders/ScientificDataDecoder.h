//==============================================================================
// ExplorerLens Engine — HDF5/NetCDF Scientific Decoder
// Hierarchical Data Format 5 and NetCDF scientific data visualization.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Scientific data format
enum class ScientificDataFormat : uint8_t {
    HDF5,     // Hierarchical Data Format 5
    NetCDF3,  // Network Common Data Form v3
    NetCDF4,  // Network Common Data Form v4 (HDF5-based)
    GRIB,     // Gridded Binary (meteorology)
    COUNT
};

/// HDF5 data type
enum class HDF5DataType : uint8_t {
    Int8,
    Int16,
    Int32,
    Int64,
    Float32,
    Float64,
    String,
    Compound,
    COUNT
};

/// Scientific visualization mode
enum class SciVisMode : uint8_t {
    Heatmap,     // 2D data as color heatmap
    Contour,     // Contour lines
    SliceView,   // 3D data slice
    Histogram,   // Data distribution
    TimeSeries,  // Time series plot
    COUNT
};

/// Scientific data info
struct ScientificDataInfo
{
    ScientificDataFormat format = ScientificDataFormat::HDF5;
    uint32_t groupCount = 0;
    uint32_t datasetCount = 0;
    uint32_t attributeCount = 0;
    uint32_t dimensions = 0;
    std::vector<uint64_t> shape;
    uint64_t totalElements = 0;
};

/// Scientific visualization config
struct SciVisConfig
{
    SciVisMode mode = SciVisMode::Heatmap;
    uint32_t colorMapSteps = 256;
    double minValue = 0.0;
    double maxValue = 1.0;
    bool autoRange = true;
    bool showColorBar = true;
    bool showAxes = true;
};

/// HDF5/NetCDF decoder
class ScientificDataDecoder
{
  public:
    static const wchar_t* FormatName(ScientificDataFormat f)
    {
        switch (f) {
            case ScientificDataFormat::HDF5:
                return L"HDF5";
            case ScientificDataFormat::NetCDF3:
                return L"NetCDF3";
            case ScientificDataFormat::NetCDF4:
                return L"NetCDF4";
            case ScientificDataFormat::GRIB:
                return L"GRIB";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* DataTypeName(HDF5DataType t)
    {
        switch (t) {
            case HDF5DataType::Int8:
                return L"Int8";
            case HDF5DataType::Int16:
                return L"Int16";
            case HDF5DataType::Int32:
                return L"Int32";
            case HDF5DataType::Int64:
                return L"Int64";
            case HDF5DataType::Float32:
                return L"Float32";
            case HDF5DataType::Float64:
                return L"Float64";
            case HDF5DataType::String:
                return L"String";
            case HDF5DataType::Compound:
                return L"Compound";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* VisModeName(SciVisMode m)
    {
        switch (m) {
            case SciVisMode::Heatmap:
                return L"Heatmap";
            case SciVisMode::Contour:
                return L"Contour";
            case SciVisMode::SliceView:
                return L"Slice View";
            case SciVisMode::Histogram:
                return L"Histogram";
            case SciVisMode::TimeSeries:
                return L"Time Series";
            default:
                return L"Unknown";
        }
    }

    /// HDF5 magic: \x89HDF\r\n\x1a\n
    static bool CheckHDF5Magic(const uint8_t* data, size_t size)
    {
        if (size < 8)
            return false;
        return data[0] == 0x89 && data[1] == 0x48 && data[2] == 0x44 && data[3] == 0x46 && data[4] == 0x0D
               && data[5] == 0x0A && data[6] == 0x1A && data[7] == 0x0A;
    }

    static constexpr size_t FormatCount()
    {
        return static_cast<size_t>(ScientificDataFormat::COUNT);
    }
    static constexpr size_t DataTypeCount()
    {
        return static_cast<size_t>(HDF5DataType::COUNT);
    }
    static constexpr size_t VisModeCount()
    {
        return static_cast<size_t>(SciVisMode::COUNT);
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
