#include "ThumbnailPipeline.h"
#include "FormatDetector.h"
#include "../Core/Config.h"
#include "../GPU/D3D11Renderer.h"
#include "../GPU/GDIRenderer.h"
#include "../Cache/ThumbnailCache.h"
#include "../Utils/PerformanceProfiler.h"
#include "../Decoders/ImageDecoder.h"
#include "../Decoders/WebPDecoder.h"
#include "../Decoders/AVIFDecoder.h"
#include "../Decoders/ArchiveDecoder.h"
#include "../Decoders/RAWDecoder.h"
#include "../Decoders/JXLDecoder.h"
#include "../Decoders/HEIFDecoder.h"
#include "../Decoders/ICODecoder.h"
#include "../Decoders/TGADecoder.h"
#include "../Decoders/QOIDecoder.h"
#include "../Decoders/PSDDecoder.h"
#include "../Decoders/DDSDecoder.h"
#include "../Decoders/HDRDecoder.h"
#include "../Decoders/PPMDecoder.h"
#include "../Decoders/EXRDecoder.h"
#include "../Decoders/SVGDecoder.h"
#include "../Decoders/VideoDecoder.h"
#include "../Decoders/AudioDecoder.h"
#include "../Decoders/PDFDecoder.h"
#include "../Decoders/DocumentDecoder.h"
#include "../Decoders/FontDecoder.h"
#include "../Decoders/ModelDecoder.h"
#include "../Plugin/PluginDecoder.h"
#include "../Plugin/PluginManager.h"
#include <chrono>
#include <algorithm>
#include <thread>
#include <future>
#include <mutex>
#include <unordered_map>

namespace DarkThumbs {
namespace Engine {

namespace {

std::wstring GetLowercaseExtension(const wchar_t* filePath) {
    if (!filePath) {
        return std::wstring();
    }

    const wchar_t* dot = wcsrchr(filePath, L'.');
    if (!dot || *(dot + 1) == L'\0') {
        return std::wstring();
    }

    std::wstring extension(dot);
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](wchar_t ch) { return static_cast<wchar_t>(towlower(ch)); });
    return extension;
}

ProfileComponent GetDecoderProfileComponent(const IThumbnailDecoder* decoder) {
    if (!decoder || !decoder->GetName()) {
        return ProfileComponent::DECODE_IMAGE;
    }

    const wchar_t* name = decoder->GetName();
    if (_wcsicmp(name, L"WebPDecoder") == 0) return ProfileComponent::DECODE_WEBP;
    if (_wcsicmp(name, L"AVIFDecoder") == 0) return ProfileComponent::DECODE_AVIF;
    if (_wcsicmp(name, L"ArchiveDecoder") == 0) return ProfileComponent::DECODE_ARCHIVE;
    if (_wcsicmp(name, L"JXLDecoder") == 0) return ProfileComponent::DECODE_JXL;
    if (_wcsicmp(name, L"HEIFDecoder") == 0) return ProfileComponent::DECODE_HEIF;
    if (_wcsicmp(name, L"RAWDecoder") == 0) return ProfileComponent::DECODE_RAW;
    if (_wcsicmp(name, L"ICODecoder") == 0) return ProfileComponent::DECODE_ICO;
    if (_wcsicmp(name, L"TGADecoder") == 0) return ProfileComponent::DECODE_TGA;
    if (_wcsicmp(name, L"QOIDecoder") == 0) return ProfileComponent::DECODE_QOI;
    if (_wcsicmp(name, L"PSDDecoder") == 0) return ProfileComponent::DECODE_PSD;
    if (_wcsicmp(name, L"DDSDecoder") == 0) return ProfileComponent::DECODE_DDS;
    if (_wcsicmp(name, L"HDRDecoder") == 0) return ProfileComponent::DECODE_HDR;
    if (_wcsicmp(name, L"PPMDecoder") == 0) return ProfileComponent::DECODE_PPM;
    if (_wcsicmp(name, L"EXRDecoder") == 0) return ProfileComponent::DECODE_EXR;
    if (_wcsicmp(name, L"SVGDecoder") == 0) return ProfileComponent::DECODE_SVG;
    if (_wcsicmp(name, L"VideoDecoder") == 0) return ProfileComponent::DECODE_VIDEO;
    if (_wcsicmp(name, L"AudioDecoder") == 0) return ProfileComponent::DECODE_AUDIO;
    if (_wcsicmp(name, L"PDFDecoder") == 0) return ProfileComponent::DECODE_PDF;
    if (_wcsicmp(name, L"DocumentDecoder") == 0) return ProfileComponent::DECODE_DOCUMENT;
    if (_wcsicmp(name, L"FontDecoder") == 0) return ProfileComponent::DECODE_FONT;

    return ProfileComponent::DECODE_IMAGE;
}

}

//==============================================================================
// Pipeline Implementation (PIMPL Pattern)
//==============================================================================

class ThumbnailPipeline::Impl {
public:
    PipelineConfig config;
    DecoderRegistry decoderRegistry;
    std::unique_ptr<IFormatDetector> formatDetector;
    std::unique_ptr<IGPURenderer> gpuRenderer;
    std::unique_ptr<ICacheProvider> cacheProvider;

    // Decoder storage (we own these) - lazy initialization
    std::vector<std::unique_ptr<IThumbnailDecoder>> decoders;
    bool decodersInitialized = false;  // NEW: Lazy init flag
    std::mutex decoderInitMutex;  // NEW: Thread-safe lazy init
    std::unordered_map<std::wstring, IThumbnailDecoder*> decoderLookupCache;
    std::mutex decoderLookupCacheMutex;

    // Statistics
    uint64_t totalRequests = 0;
    uint64_t cacheHits = 0;
    uint64_t cacheMisses = 0;
    uint64_t totalProcessingTimeMs = 0;

    bool initialized = false;

    Impl() = default;

    ~Impl() {
        Shutdown();
    }

    bool Initialize(const PipelineConfig& cfg) {
        if (initialized) {
            return true;
        }

        config = cfg;

        // Create default format detector if not provided
        if (!formatDetector) {
            formatDetector = std::make_unique<FormatDetector>();
        }

        // NEW: Decoders are NOT created here anymore - lazy init on first use
        // This saves ~50-100ms startup time per COM object creation
        
        // Create GPU renderer if enabled
        if (config.enableGPU && !gpuRenderer) {
            // Try hardware GPU first (D3D11)
            auto d3dRenderer = std::make_unique<D3D11Renderer>();
            if (SUCCEEDED(d3dRenderer->Initialize())) {
                gpuRenderer = std::move(d3dRenderer);
            } else {
                // Fall back to CPU renderer (GDI+)
                auto cpuRenderer = std::make_unique<GDIRenderer>();
                if (SUCCEEDED(cpuRenderer->Initialize())) {
                    gpuRenderer = std::move(cpuRenderer);
                }
            }
        }

        // Create cache if enabled
        if (config.enableCache && !cacheProvider) {
            auto cache = std::make_unique<ThumbnailCache>();
            if (SUCCEEDED(cache->Initialize())) {
                cacheProvider = std::move(cache);
            }
        }

        initialized = true;
        return true;
    }

    void EnsureDecodersInitialized() {
        if (decodersInitialized) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(decoderInitMutex);
        if (decodersInitialized) {  // Double-check after lock
            return;
        }
        
        OutputDebugStringW(L"[Pipeline] Lazy initializing decoders...\n");
        
        // Register all available decoders (order matters - more specific first)
        // 1. Archive formats (ZIP, RAR, 7Z, etc.)
        auto archiveDecoder = std::make_unique<ArchiveDecoder>();
        decoderRegistry.RegisterDecoder(archiveDecoder.get());
        decoders.push_back(std::move(archiveDecoder));
        
        // 2. Modern image formats with specific decoders
        auto webpDecoder = std::make_unique<WebPDecoder>();
        decoderRegistry.RegisterDecoder(webpDecoder.get());
        decoders.push_back(std::move(webpDecoder));
        
        auto avifDecoder = std::make_unique<AVIFDecoder>();
        decoderRegistry.RegisterDecoder(avifDecoder.get());
        decoders.push_back(std::move(avifDecoder));
        
        // Camera RAW formats (Canon, Nikon, Sony, DNG, etc.)
        auto rawDecoder = std::make_unique<RAWDecoder>();
        decoderRegistry.RegisterDecoder(rawDecoder.get());
        decoders.push_back(std::move(rawDecoder));
        
        // HEIF/HEIC formats (iPhone photos, AVIF variants)
        auto heifDecoder = std::make_unique<HEIFDecoder>();
        decoderRegistry.RegisterDecoder(heifDecoder.get());
        decoders.push_back(std::move(heifDecoder));
        
        // JPEG XL (modern high-efficiency format)
        auto jxlDecoder = std::make_unique<JXLDecoder>();
        decoderRegistry.RegisterDecoder(jxlDecoder.get());
        decoders.push_back(std::move(jxlDecoder));
        
        // Professional/specialty formats
        auto icoDecoder = std::make_unique<ICODecoder>();
        decoderRegistry.RegisterDecoder(icoDecoder.get());
        decoders.push_back(std::move(icoDecoder));
        
        auto tgaDecoder = std::make_unique<TGADecoder>();
        decoderRegistry.RegisterDecoder(tgaDecoder.get());
        decoders.push_back(std::move(tgaDecoder));
        
        auto qoiDecoder = std::make_unique<QOIDecoder>();
        decoderRegistry.RegisterDecoder(qoiDecoder.get());
        decoders.push_back(std::move(qoiDecoder));
        
        // Adobe Photoshop PSD/PSB
        auto psdDecoder = std::make_unique<PSDDecoder>();
        decoderRegistry.RegisterDecoder(psdDecoder.get());
        decoders.push_back(std::move(psdDecoder));
        
        // DirectDraw Surface (game textures)
        auto ddsDecoder = std::make_unique<DDSDecoder>();
        decoderRegistry.RegisterDecoder(ddsDecoder.get());
        decoders.push_back(std::move(ddsDecoder));
        
        // Radiance HDR (RGBE)
        auto hdrDecoder = std::make_unique<HDRDecoder>();
        decoderRegistry.RegisterDecoder(hdrDecoder.get());
        decoders.push_back(std::move(hdrDecoder));
        
        // Netpbm formats (PPM/PGM/PBM/PNM/PAM/PFM)
        auto ppmDecoder = std::make_unique<PPMDecoder>();
        decoderRegistry.RegisterDecoder(ppmDecoder.get());
        decoders.push_back(std::move(ppmDecoder));
        
        // OpenEXR (via WIC codec)
        auto exrDecoder = std::make_unique<EXRDecoder>();
        decoderRegistry.RegisterDecoder(exrDecoder.get());
        decoders.push_back(std::move(exrDecoder));
        
        // SVG/SVGZ vector images
        auto svgDecoder = std::make_unique<SVGDecoder>();
        decoderRegistry.RegisterDecoder(svgDecoder.get());
        decoders.push_back(std::move(svgDecoder));
        
        // Video formats (Media Foundation)
        auto videoDecoder = std::make_unique<VideoDecoder>();
        decoderRegistry.RegisterDecoder(videoDecoder.get());
        decoders.push_back(std::move(videoDecoder));
        
        // Audio formats (album art / waveform)
        auto audioDecoder = std::make_unique<AudioDecoder>();
        decoderRegistry.RegisterDecoder(audioDecoder.get());
        decoders.push_back(std::move(audioDecoder));
        
        // PDF documents
        auto pdfDecoder = std::make_unique<PDFDecoder>();
        decoderRegistry.RegisterDecoder(pdfDecoder.get());
        decoders.push_back(std::move(pdfDecoder));
        
        // Office/eBook/document formats
        auto documentDecoder = std::make_unique<DocumentDecoder>();
        decoderRegistry.RegisterDecoder(documentDecoder.get());
        decoders.push_back(std::move(documentDecoder));
        
        // Font preview (TTF, OTF, etc.)
        auto fontDecoder = std::make_unique<FontDecoder>();
        decoderRegistry.RegisterDecoder(fontDecoder.get());
        decoders.push_back(std::move(fontDecoder));
        
        // 3D models (.obj, .stl, .gltf, .glb)
        auto modelDecoder = std::make_unique<ModelDecoder>();
        decoderRegistry.RegisterDecoder(modelDecoder.get());
        decoders.push_back(std::move(modelDecoder));
        
        // 3. Standard image formats via WIC (JPEG, PNG, BMP, GIF, TIFF)
        auto imageDecoder = std::make_unique<ImageDecoder>();
        decoderRegistry.RegisterDecoder(imageDecoder.get());
        decoders.push_back(std::move(imageDecoder));
        
        // 4. Scan for and load third-party plugins
        LoadPlugins();
        
        decodersInitialized = true;
        OutputDebugStringW(L"[Pipeline] Lazy decoder init complete\n");
    }

    void Shutdown() {
        if (!initialized) {
            return;
        }

        if (gpuRenderer) {
            gpuRenderer->Shutdown();
        }

        if (cacheProvider) {
            // Properly shutdown cache (ThumbnailCache has Shutdown method)
            auto* cache = dynamic_cast<ThumbnailCache*>(cacheProvider.get());
            if (cache) {
                cache->Shutdown();
            }
        }

        decoderRegistry.Clear();
        {
            std::lock_guard<std::mutex> lock(decoderLookupCacheMutex);
            decoderLookupCache.clear();
        }
        decoders.clear();
        decodersInitialized = false;
        initialized = false;
    }

    ThumbnailResult GenerateThumbnail(const ThumbnailRequest& request) {
        ScopedTimer pipelineTotalTimer(ProfileComponent::PIPELINE_TOTAL);
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        ThumbnailResult result;
        result.status = E_FAIL;

        // Input validation (Task A21: Add comprehensive bounds checking)
        if (!request.filePath || request.filePath[0] == L'\0') {
            OutputDebugStringW(L"[Pipeline] ERROR: Invalid file path\n");
            result.status = E_INVALIDARG;
            return result;
        }
        
        if (request.width == 0 || request.height == 0) {
            OutputDebugStringW(L"[Pipeline] ERROR: Invalid dimensions (0x0)\n");
            result.status = E_INVALIDARG;
            return result;
        }
        
        // Reasonable max resolution check (8K = 7680x4320)
        if (request.width > 8192 || request.height > 8192) {
            OutputDebugStringW(L"[Pipeline] ERROR: Dimensions exceed maximum (8192x8192)\n");
            result.status = E_INVALIDARG;
            return result;
        }

        if (!initialized) {
            OutputDebugStringW(L"[Pipeline] ERROR: Pipeline not initialized\n");
            return result;
        }

        totalRequests++;
        
        // Log request
        wchar_t logBuf[512];
        swprintf_s(logBuf, L"[Pipeline] Request #%llu: %s (%ux%u)\n", 
                   totalRequests, request.filePath, request.width, request.height);
        OutputDebugStringW(logBuf);

        // Step 1: Check cache for existing thumbnail
        if (config.enableCache && cacheProvider) {
            ScopedTimer cacheLookupTimer(ProfileComponent::CACHE_LOOKUP);
            HBITMAP cachedBitmap = nullptr;
            if (SUCCEEDED(cacheProvider->Get(request.filePath, 
                                              request.width, 
                                              request.height, 
                                              &cachedBitmap))) {
                // Cache hit!
                result.hBitmap = cachedBitmap;
                result.width = request.width;
                result.height = request.height;
                result.fromCache = true;
                result.status = S_OK;
                
                cacheHits++;
                
                auto endTime = std::chrono::high_resolution_clock::now();
                result.generationTimeMs = static_cast<uint32_t>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        endTime - startTime).count());
                
                return result;
            }
        }
        cacheMisses++;

        // Step 2: Ensure decoders are initialized (lazy init)
        {
            ScopedTimer decoderInitTimer(ProfileComponent::PIPELINE_DECODER_INIT);
            EnsureDecodersInitialized();
        }

        // Step 3: Find appropriate decoder
        IThumbnailDecoder* decoder = nullptr;
        {
            ScopedTimer decoderLookupTimer(ProfileComponent::PIPELINE_DECODER_LOOKUP);
            const std::wstring extensionKey = GetLowercaseExtension(request.filePath);

            if (!extensionKey.empty()) {
                std::lock_guard<std::mutex> lock(decoderLookupCacheMutex);
                auto cached = decoderLookupCache.find(extensionKey);
                if (cached != decoderLookupCache.end()) {
                    decoder = cached->second;
                }
            }

            if (!decoder) {
                decoder = decoderRegistry.FindDecoder(request.filePath);
                if (decoder && !extensionKey.empty()) {
                    std::lock_guard<std::mutex> lock(decoderLookupCacheMutex);
                    decoderLookupCache[extensionKey] = decoder;
                }
            }
        }
        if (!decoder) {
            OutputDebugStringW(L"[Pipeline] ERROR: No decoder found for file\n");
            result.status = E_NOINTERFACE;  // No decoder found

            auto endTime = std::chrono::high_resolution_clock::now();
            result.generationTimeMs = static_cast<uint32_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    endTime - startTime).count());

            return result;
        }
        
        // Log decoder found
        wchar_t decoderLog[256];
        swprintf_s(decoderLog, L"[Pipeline] Using decoder: %s\n", decoder->GetName());
        OutputDebugStringW(decoderLog);

        // Step 4: Generate thumbnail
        {
            ScopedTimer decoderTimer(GetDecoderProfileComponent(decoder));
            result.status = decoder->Decode(request, result);
        }
        
        // Log decode result
        wchar_t resultLog[256];
        swprintf_s(resultLog, L"[Pipeline] Decode result: HRESULT=0x%08X, HBITMAP=%p\n", 
                   result.status, result.hBitmap);
        OutputDebugStringW(resultLog);

        // Step 5: Cache the result if successful
        if (SUCCEEDED(result.status) && config.enableCache && cacheProvider && result.hBitmap) {
            ScopedTimer cacheStoreTimer(ProfileComponent::CACHE_STORE);
            // Store in cache (fire and forget - don't fail thumbnail on cache error)
            cacheProvider->Put(request.filePath, 
                              result.width, 
                              result.height, 
                              result.hBitmap);
        }

        // Update timing
        auto endTime = std::chrono::high_resolution_clock::now();
        result.generationTimeMs = static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                endTime - startTime).count());

        totalProcessingTimeMs += result.generationTimeMs;

        return result;
    }

    bool IsFormatSupported(const std::wstring& filePath) const {
        if (!initialized) {
            return false;
        }

        // Lazy init decoders if needed
        const_cast<Impl*>(this)->EnsureDecodersInitialized();

        // Check if we have a decoder for this file
        return decoderRegistry.FindDecoder(filePath.c_str()) != nullptr;
    }

    void GetStatistics(
        uint64_t& outTotalRequests,
        uint64_t& outCacheHits,
        uint64_t& outCacheMisses,
        double& outAverageTimeMs) const 
    {
        outTotalRequests = totalRequests;
        outCacheHits = cacheHits;
        outCacheMisses = cacheMisses;
        
        if (totalRequests > 0) {
            outAverageTimeMs = static_cast<double>(totalProcessingTimeMs) / totalRequests;
        } else {
            outAverageTimeMs = 0.0;
        }
    }

    void ResetStatistics() {
        totalRequests = 0;
        cacheHits = 0;
        cacheMisses = 0;
        totalProcessingTimeMs = 0;
    }

private:
    /// Load and register third-party plugins
    void LoadPlugins() {
        OutputDebugStringW(L"[Pipeline] Scanning for plugins...\n");
        
        // Get plugin manager singleton
        auto& pluginManager = PluginManager::Instance();
        
        // Get plugin search paths
        auto searchPaths = PluginDiscovery::GetPluginSearchPaths();
        
        size_t totalLoaded = 0;
        
        // Scan each search path
        for (const auto& path : searchPaths) {
            if (!std::filesystem::exists(path)) {
                continue;
            }
            
            wchar_t logBuf[512];
            swprintf_s(logBuf, L"[Pipeline] Scanning plugin directory: %s\n", path.c_str());
            OutputDebugStringW(logBuf);
            
            size_t loaded = pluginManager.ScanPluginDirectory(path);
            totalLoaded += loaded;
            
            if (loaded > 0) {
                swprintf_s(logBuf, L"[Pipeline] Loaded %zu plugin(s) from %s\n", 
                          loaded, path.c_str());
                OutputDebugStringW(logBuf);
            }
        }
        
        // Create decoder wrappers for all loaded plugins
        auto pluginNames = pluginManager.GetPluginNames();
        for (const auto& name : pluginNames) {
            // Get plugin handle
            PluginHandle* pluginHandle = pluginManager.GetPluginHandle(name);
            if (!pluginHandle || !pluginHandle->IsLoaded()) {
                continue;
            }
            
            // Get plugin info
            const PluginInfo* info = pluginHandle->GetInfo();
            if (!info) {
                continue;
            }
            
            // Log plugin discovery
            wchar_t logBuf[512];
            if (info->plugin_name) {
                swprintf_s(logBuf, L"[Pipeline] Registering plugin decoder: %S (v%S)\n", 
                          info->plugin_name, 
                          info->plugin_version ? info->plugin_version : "1.0");
                OutputDebugStringW(logBuf);
                
                // List supported extensions
                if (info->supported_extensions) {
                    for (size_t i = 0; info->supported_extensions[i] != nullptr; ++i) {
                        swprintf_s(logBuf, L"[Pipeline]   - Format: %S\n", 
                                  info->supported_extensions[i]);
                        OutputDebugStringW(logBuf);
                    }
                }
            }
            
            // Create PluginDecoder wrapper
            std::wstring name_wide(name.begin(), name.end());
            auto pluginDecoder = std::make_unique<PluginDecoder>(pluginHandle, name_wide);
            IThumbnailDecoder* decoderPtr = pluginDecoder.get();
            decoderRegistry.RegisterDecoder(decoderPtr);
            decoders.push_back(std::move(pluginDecoder));
        }
        
        wchar_t summary[256];
        swprintf_s(summary, L"[Pipeline] Plugin scan complete. Total plugins: %zu\n", totalLoaded);
        OutputDebugStringW(summary);
    }
};

//==============================================================================
// ThumbnailPipeline Public Interface
//==============================================================================

ThumbnailPipeline::ThumbnailPipeline()
    : m_impl(std::make_unique<Impl>())
{
}

ThumbnailPipeline::~ThumbnailPipeline() = default;

bool ThumbnailPipeline::Initialize(const PipelineConfig& config) {
    return m_impl->Initialize(config);
}

void ThumbnailPipeline::Shutdown() {
    m_impl->Shutdown();
}

ThumbnailResult ThumbnailPipeline::GenerateThumbnail(const ThumbnailRequest& request) {
    return m_impl->GenerateThumbnail(request);
}

std::vector<ThumbnailResult> ThumbnailPipeline::GenerateThumbnailsBatch(
    const std::vector<ThumbnailRequest>& requests) {
    std::vector<ThumbnailResult> results(requests.size());
    
    if (!m_impl->initialized || requests.empty()) return results;
    
    if (!m_impl->config.enableParallelDecode || requests.size() <= 1) {
        // Sequential fallback
        for (size_t i = 0; i < requests.size(); ++i) {
            results[i] = m_impl->GenerateThumbnail(requests[i]);
        }
        return results;
    }
    
    // Parallel execution with bounded concurrency
    uint32_t maxConcurrent = m_impl->config.maxConcurrentDecodes;
    if (maxConcurrent == 0) maxConcurrent = 4;
    
    size_t remaining = requests.size();
    size_t offset = 0;
    
    while (remaining > 0) {
        size_t batchSize = (std::min)(static_cast<size_t>(maxConcurrent), remaining);
        std::vector<std::future<ThumbnailResult>> futures;
        futures.reserve(batchSize);
        
        for (size_t i = 0; i < batchSize; ++i) {
            size_t idx = offset + i;
            futures.push_back(std::async(std::launch::async,
                [this, &requests, idx]() {
                    return m_impl->GenerateThumbnail(requests[idx]);
                }));
        }
        
        for (size_t i = 0; i < batchSize; ++i) {
            results[offset + i] = futures[i].get();
        }
        
        offset += batchSize;
        remaining -= batchSize;
    }
    
    return results;
}

bool ThumbnailPipeline::IsFormatSupported(const std::wstring& filePath) const {
    return m_impl->IsFormatSupported(filePath);
}

DecoderRegistry& ThumbnailPipeline::GetDecoderRegistry() {
    return m_impl->decoderRegistry;
}

void ThumbnailPipeline::SetFormatDetector(std::unique_ptr<IFormatDetector> detector) {
    m_impl->formatDetector = std::move(detector);
}

void ThumbnailPipeline::SetGPURenderer(std::unique_ptr<IGPURenderer> renderer) {
    m_impl->gpuRenderer = std::move(renderer);
}

void ThumbnailPipeline::SetCacheProvider(std::unique_ptr<ICacheProvider> cache) {
    m_impl->cacheProvider = std::move(cache);
}

void ThumbnailPipeline::GetStatistics(
    uint64_t& totalRequests,
    uint64_t& cacheHits,
    uint64_t& cacheMisses,
    double& averageTimeMs) const 
{
    m_impl->GetStatistics(totalRequests, cacheHits, cacheMisses, averageTimeMs);
}

void ThumbnailPipeline::ResetStatistics() {
    m_impl->ResetStatistics();
}

//==============================================================================
// Decoder Status Reporting
//==============================================================================

std::vector<ThumbnailPipeline::DecoderStatus> ThumbnailPipeline::GetDecoderStatus() const {
    std::vector<DecoderStatus> status;

    if (!m_impl || !m_impl->initialized)
        return status;

    for (const auto& decoder : m_impl->decoders) {
        if (!decoder) continue;

        DecoderInfo info = decoder->GetInfo();
        DecoderStatus entry;
        entry.name = info.name;
        entry.version = info.version;
        entry.extensionCount = info.extensionCount;
        entry.supportsGPU = info.supportsGPU;
        entry.isArchiveDecoder = info.isArchiveDecoder;
        status.push_back(entry);
    }

    return status;
}

size_t ThumbnailPipeline::GetDecoderCount() const {
    if (!m_impl || !m_impl->initialized)
        return 0;
    return m_impl->decoders.size();
}

size_t ThumbnailPipeline::GetTotalExtensionCount() const {
    if (!m_impl || !m_impl->initialized)
        return 0;

    size_t total = 0;
    for (const auto& decoder : m_impl->decoders) {
        if (!decoder) continue;
        DecoderInfo info = decoder->GetInfo();
        total += info.extensionCount;
    }
    return total;
}

} // namespace Engine
} // namespace DarkThumbs
