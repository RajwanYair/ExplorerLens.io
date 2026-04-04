//==============================================================================
// FormatConverterEngine
//==============================================================================

#include "FormatConverterEngine.h"
#include <algorithm>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

FormatConverterEngine::FormatConverterEngine() {}

EngineConvResult FormatConverterEngine::Convert(const ConversionJob& job)
{
    EngineConvResult result;
    auto start = std::chrono::high_resolution_clock::now();

    result.inputPath = job.inputPath;
    result.outputPath = job.outputPath;

    // In production: use WIC/codec to transcode
    result.success = !job.inputPath.empty() && !job.outputPath.empty();

    auto end = std::chrono::high_resolution_clock::now();
    result.conversionTimeMs = std::chrono::duration<double, std::milli>(end - start).count();

    if (result.inputSize > 0 && result.outputSize > 0) {
        result.compressionRatio = static_cast<double>(result.inputSize) / result.outputSize;
    }

    return result;
}

EngineConversionResult FormatConverterEngine::ConvertBatch(const std::vector<ConversionJob>& jobs)
{
    EngineConversionResult batch;
    auto start = std::chrono::high_resolution_clock::now();

    batch.totalJobs = static_cast<uint32_t>(jobs.size());

    for (const auto& job : jobs) {
        auto result = Convert(job);
        if (result.success) {
            batch.succeeded++;
        } else {
            batch.failed++;
        }
        batch.totalInputBytes += result.inputSize;
        batch.totalOutputBytes += result.outputSize;
        batch.results.push_back(result);
    }

    auto end = std::chrono::high_resolution_clock::now();
    batch.totalTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
    return batch;
}

ConvertFormat FormatConverterEngine::DetectFormat(const std::wstring& filePath)
{
    auto dot = filePath.find_last_of(L'.');
    if (dot == std::wstring::npos)
        return ConvertFormat::PNG;

    auto ext = filePath.substr(dot + 1);
    for (auto& c : ext)
        c = towlower(c);

    if (ext == L"png")
        return ConvertFormat::PNG;
    if (ext == L"jpg" || ext == L"jpeg")
        return ConvertFormat::JPEG;
    if (ext == L"webp")
        return ConvertFormat::WebP;
    if (ext == L"bmp")
        return ConvertFormat::BMP;
    if (ext == L"tiff" || ext == L"tif")
        return ConvertFormat::TIFF;
    if (ext == L"avif")
        return ConvertFormat::AVIF;
    if (ext == L"jxl")
        return ConvertFormat::JXL;
    return ConvertFormat::PNG;
}

const wchar_t* FormatConverterEngine::GetFormatName(ConvertFormat format)
{
    switch (format) {
        case ConvertFormat::PNG:
            return L"PNG";
        case ConvertFormat::JPEG:
            return L"JPEG";
        case ConvertFormat::WebP:
            return L"WebP";
        case ConvertFormat::BMP:
            return L"BMP";
        case ConvertFormat::TIFF:
            return L"TIFF";
        case ConvertFormat::AVIF:
            return L"AVIF";
        case ConvertFormat::JXL:
            return L"JPEG XL";
        default:
            return L"Unknown";
    }
}

const wchar_t* FormatConverterEngine::GetFormatExtension(ConvertFormat format)
{
    switch (format) {
        case ConvertFormat::PNG:
            return L".png";
        case ConvertFormat::JPEG:
            return L".jpg";
        case ConvertFormat::WebP:
            return L".webp";
        case ConvertFormat::BMP:
            return L".bmp";
        case ConvertFormat::TIFF:
            return L".tif";
        case ConvertFormat::AVIF:
            return L".avif";
        case ConvertFormat::JXL:
            return L".jxl";
        default:
            return L".bin";
    }
}

const wchar_t* FormatConverterEngine::GetPresetName(ExportQualityPreset preset)
{
    switch (preset) {
        case ExportQualityPreset::Draft:
            return L"Draft";
        case ExportQualityPreset::Normal:
            return L"Normal";
        case ExportQualityPreset::High:
            return L"High";
        case ExportQualityPreset::Lossless:
            return L"Lossless";
        default:
            return L"Unknown";
    }
}

uint32_t FormatConverterEngine::GetQualityValue(ExportQualityPreset preset)
{
    switch (preset) {
        case ExportQualityPreset::Draft:
            return 50;
        case ExportQualityPreset::Normal:
            return 75;
        case ExportQualityPreset::High:
            return 90;
        case ExportQualityPreset::Lossless:
            return 100;
        default:
            return 75;
    }
}

}  // namespace Engine
}  // namespace ExplorerLens
