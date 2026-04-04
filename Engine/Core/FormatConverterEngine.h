#pragma once
//==============================================================================
// FormatConverterEngine
// Batch format conversion between supported thumbnail types.
// Supports quality presets, metadata preservation, and batch scheduling.
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include "EncoderExportEngine.h"

namespace ExplorerLens {
namespace Engine {

enum class ConvertFormat : uint8_t {
    PNG = 0,
    JPEG = 1,
    WebP = 2,
    BMP = 3,
    TIFF = 4,
    AVIF = 5,
    JXL = 6,
    FormatCount = 7
};

struct ConversionJob
{
    std::wstring inputPath;
    std::wstring outputPath;
    ConvertFormat sourceFormat = ConvertFormat::PNG;
    ConvertFormat targetFormat = ConvertFormat::JPEG;
    ExportQualityPreset quality = ExportQualityPreset::High;
    uint32_t maxWidth = 0;
    uint32_t maxHeight = 0;
    bool preserveMetadata = true;
};

struct EngineConvResult
{
    bool success = false;
    std::wstring inputPath;
    std::wstring outputPath;
    uint64_t inputSize = 0;
    uint64_t outputSize = 0;
    double conversionTimeMs = 0.0;
    double compressionRatio = 0.0;
};

struct EngineConversionResult
{
    uint32_t totalJobs = 0;
    uint32_t succeeded = 0;
    uint32_t failed = 0;
    double totalTimeMs = 0.0;
    uint64_t totalInputBytes = 0;
    uint64_t totalOutputBytes = 0;
    std::vector<EngineConvResult> results;
};

class FormatConverterEngine
{
  public:
    FormatConverterEngine();

    EngineConvResult Convert(const ConversionJob& job);
    EngineConversionResult ConvertBatch(const std::vector<ConversionJob>& jobs);

    void SetThreadCount(uint32_t threads)
    {
        m_threadCount = threads;
    }
    uint32_t GetThreadCount() const
    {
        return m_threadCount;
    }

    static ConvertFormat DetectFormat(const std::wstring& filePath);
    static const wchar_t* GetFormatName(ConvertFormat format);
    static const wchar_t* GetFormatExtension(ConvertFormat format);
    static const wchar_t* GetPresetName(ExportQualityPreset preset);
    static uint32_t GetQualityValue(ExportQualityPreset preset);
    static uint32_t GetFormatCount()
    {
        return static_cast<uint32_t>(ConvertFormat::FormatCount);
    }

  private:
    uint32_t m_threadCount = 4;
};

}  // namespace Engine
}  // namespace ExplorerLens
