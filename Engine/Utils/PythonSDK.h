#pragma once
//==============================================================================
// PythonSDK
// Python bindings for ExplorerLens engine. Exposes thumbnail generation,
// format detection, and decoder capabilities through a C-compatible ABI
// that can be loaded via ctypes or wrapped with pybind11.
//
// Features:
// - C ABI export functions for Python ctypes interop
// - Format detection from file path or buffer
// - Thumbnail generation to BGRA buffer
// - Decoder enumeration and capability querying
// - Batch processing with progress callback
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Python SDK configuration
struct PythonSDKConfig
{
    uint32_t maxThumbnailWidth = 512;
    uint32_t maxThumbnailHeight = 512;
    bool enableGPU = true;
    bool enableCache = true;
    bool enablePlugins = false;
    uint32_t maxConcurrency = 4;
    std::wstring cachePath;
    std::wstring pluginPath;
};

/// Thumbnail result for Python
struct PythonThumbnailResult
{
    std::vector<uint8_t> pixelData;  ///< BGRA pixel data
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t channels = 4;
    std::wstring format;  ///< Detected format name
    double decodeTimeMs = 0.0;
    bool success = false;
    std::wstring error;
};

/// Decoder info for Python
struct PythonDecoderInfo
{
    std::wstring name;
    std::wstring description;
    std::vector<std::wstring> extensions;
    bool isAvailable = true;
    bool requiresExternalLib = false;
    std::wstring libraryVersion;
};

/// Batch processing entry
struct BatchEntry
{
    std::wstring inputPath;
    std::wstring outputPath;
    uint32_t width = 256;
    uint32_t height = 256;
    bool completed = false;
    bool success = false;
    double timeMs = 0.0;
    std::wstring error;
};

/// Batch result
struct PythonBatchResult
{
    uint32_t total = 0;
    uint32_t succeeded = 0;
    uint32_t failed = 0;
    double totalTimeMs = 0.0;
    double avgTimeMs = 0.0;
    std::vector<BatchEntry> entries;
};

/// Progress callback type
using ProgressCallback = void (*)(uint32_t current, uint32_t total, const wchar_t* currentFile);

//==============================================================================
// PythonSDK
//==============================================================================
class PythonSDK
{
  public:
    PythonSDK();
    explicit PythonSDK(const PythonSDKConfig& config);

    /// Initialize SDK
    bool Initialize();

    /// Generate thumbnail from file path
    PythonThumbnailResult GenerateThumbnail(const std::wstring& filePath, uint32_t width = 256, uint32_t height = 256);

    /// Generate thumbnail from memory buffer
    PythonThumbnailResult GenerateFromBuffer(const uint8_t* data, size_t size, const std::wstring& formatHint,
                                             uint32_t width = 256, uint32_t height = 256);

    /// Detect format from file path
    std::wstring DetectFormat(const std::wstring& filePath) const;

    /// Detect format from buffer
    std::wstring DetectFormatFromBuffer(const uint8_t* data, size_t size) const;

    /// Get available decoders
    std::vector<PythonDecoderInfo> GetDecoders() const;

    /// Get decoder count
    uint32_t GetDecoderCount() const;

    /// Check if format is supported
    bool IsFormatSupported(const std::wstring& extension) const;

    /// Batch process files
    PythonBatchResult ProcessBatch(std::vector<BatchEntry>& entries, ProgressCallback callback = nullptr);

    /// Get SDK version
    static std::wstring GetVersion();

    /// Get SDK config
    const PythonSDKConfig& GetConfig() const
    {
        return m_config;
    }

    /// Generate Python bindings stub code
    static std::wstring GenerateCtypesStub();

    /// Generate pybind11 wrapper code
    static std::wstring GeneratePybindWrapper();

  private:
    PythonSDKConfig m_config;
    bool m_initialized = false;
    std::vector<PythonDecoderInfo> m_decoders;

    void PopulateDecoders();
};

}  // namespace Engine
}  // namespace ExplorerLens

//==============================================================================
// C ABI Exports (for ctypes)
//==============================================================================
#ifdef __cplusplus
extern "C"
{
#endif

/// Initialize the SDK (returns handle)
void* ExplorerLens_Init(void);

/// Generate thumbnail from file (returns pixel data via out params)
int ExplorerLens_GenerateThumbnail(void* handle, const wchar_t* path, uint32_t width, uint32_t height,
                                   uint8_t** outPixels, uint32_t* outSize);

/// Free pixel data
void ExplorerLens_FreePixels(uint8_t* pixels);

/// Get version string
const wchar_t* ExplorerLens_GetVersion(void);

/// Get decoder count
uint32_t ExplorerLens_GetDecoderCount(void* handle);

/// Cleanup
void ExplorerLens_Destroy(void* handle);

#ifdef __cplusplus
}
#endif
