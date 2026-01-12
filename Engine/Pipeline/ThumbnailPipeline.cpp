#include "ThumbnailPipeline.h"
#include "FormatDetector.h"
#include "../GPU/D3D11Renderer.h"
#include "../GPU/GDIRenderer.h"
#include "../Cache/ThumbnailCache.h"
#include <chrono>
#include <algorithm>

namespace DarkThumbs {
namespace Engine {

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
        initialized = false;
    }

    ThumbnailResult GenerateThumbnail(const ThumbnailRequest& request) {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        ThumbnailResult result;
        result.status = E_FAIL;

        if (!initialized) {
            return result;
        }

        totalRequests++;

        // Step 1: Check cache for existing thumbnail
        if (config.enableCache && cacheProvider) {
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

        // Step 2: Find appropriate decoder
        IThumbnailDecoder* decoder = decoderRegistry.FindDecoder(request.filePath);
        if (!decoder) {
            result.status = E_NOINTERFACE;  // No decoder found

            auto endTime = std::chrono::high_resolution_clock::now();
            result.generationTimeMs = static_cast<uint32_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    endTime - startTime).count());

            return result;
        }

        // Step 3: Generate thumbnail
        result.status = decoder->Decode(request, result);

        // Step 4: Cache the result if successful
        if (SUCCEEDED(result.status) && config.enableCache && cacheProvider && result.hBitmap) {
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

} // namespace Engine
} // namespace DarkThumbs
