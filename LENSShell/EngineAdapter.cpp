//==============================================================================
// EngineAdapter.cpp - Implementation of Engine adapter
//==============================================================================

#include "EngineAdapter.h"

#include "../Engine/Decoders/AVIFDecoder.h"
#include "../Engine/Decoders/ArchiveDecoder.h"
#include "../Engine/Decoders/ImageDecoder.h"
#include "../Engine/Decoders/WebPDecoder.h"
#include "StdAfx.h"  // Must be first — provides ATL/COM/Windows headers (PCH anchor)
// Note: JXL/HEIF decoders are registered by ThumbnailPipeline automatically.
// No direct includes needed here - the pipeline handles all decoder registration.
#include "error_logger.h"

namespace ExplorerLens {

EngineAdapter::EngineAdapter() : m_initialized(false) {}

EngineAdapter::~EngineAdapter()
{
    Shutdown();
}

bool EngineAdapter::Initialize()
{
    if (m_initialized) {
        return true;
    }

    try {
        // Create the thumbnail pipeline
        m_pipeline = std::make_unique<Engine::ThumbnailPipeline>();

        // Configure pipeline
        Engine::PipelineConfig config;
        config.enableCache = true;
        config.enableGPU = true;
        config.preserveAspectRatio = true;
        config.defaultWidth = 256;
        config.defaultHeight = 256;
        config.maxFileSize = 100 * 1024 * 1024;  // 100MB
        config.timeoutMs = 5000;                 // 5 seconds

        if (!m_pipeline->Initialize(config)) {
            DT_LOG_ERROR(LogCategory::ENGINE, "Failed to initialize ThumbnailPipeline");
            return false;
        }

        // Note: ThumbnailPipeline automatically registers all built-in decoders
        // No need to manually register decoders here

        m_initialized = true;

        DT_LOG_INFO(LogCategory::ENGINE, "Engine adapter initialized successfully");
        return true;

    } catch (const std::exception& ex) {
        DT_LOG_ERROR(LogCategory::ENGINE, std::string("Engine initialization exception: ") + ex.what());
        return false;
    }
}

void EngineAdapter::Shutdown()
{
    if (!m_initialized) {
        return;
    }

    if (m_pipeline) {
        m_pipeline->Shutdown();
        m_pipeline.reset();
    }

    m_initialized = false;
    DT_LOG_INFO(LogCategory::ENGINE, "Engine adapter shutdown complete");
}

HRESULT EngineAdapter::GenerateThumbnail(const wchar_t* filePath, uint32_t width, uint32_t height, bool useGPU,
                                         HBITMAP* phBitmap)
{
    if (!m_initialized || !m_pipeline) {
        DT_LOG_ERROR(LogCategory::ENGINE, "Engine not initialized");
        return E_FAIL;
    }

    if (!filePath || !phBitmap) {
        DT_LOG_ERROR(LogCategory::ENGINE, "Invalid parameters");
        return E_POINTER;
    }

    *phBitmap = nullptr;

    // Create request
    Engine::ThumbnailRequest request;
    request.filePath = filePath;
    request.width = width;
    request.height = height;
    request.flags = Engine::ThumbnailFlags::PreserveAspect | Engine::ThumbnailFlags::UseCache;

    if (useGPU) {
        request.flags = request.flags | Engine::ThumbnailFlags::UseGPU;
    }

    // Generate thumbnail through pipeline
    auto startTime = std::chrono::high_resolution_clock::now();

    Engine::ThumbnailResult result = m_pipeline->GenerateThumbnail(request);

    auto endTime = std::chrono::high_resolution_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    // Check result
    if (SUCCEEDED(result.status)) {
        *phBitmap = result.hBitmap;

        std::string logMsg = "Thumbnail generated: " + std::to_string(result.width) + "x"
                             + std::to_string(result.height) + " in " + std::to_string(durationMs) + "ms";

        if (result.fromCache) {
            logMsg += " (cached)";
        }
        if (result.usedGPU) {
            logMsg += " (GPU)";
        }

        DT_LOG_DEBUG(LogCategory::ENGINE, logMsg);
        return S_OK;
    } else {
        DT_LOG_HRESULT(LogLevel::LVL_ERROR, LogCategory::ENGINE, "Thumbnail generation failed", result.status);
        return result.status;
    }
}

bool EngineAdapter::IsFormatSupported(const wchar_t* filePath) const
{
    if (!m_initialized || !m_pipeline || !filePath) {
        return false;
    }

    return m_pipeline->IsFormatSupported(filePath);
}

void EngineAdapter::GetStatistics(uint64_t& totalRequests, uint64_t& cacheHits, double& averageTimeMs) const
{
    if (!m_initialized || !m_pipeline) {
        totalRequests = 0;
        cacheHits = 0;
        averageTimeMs = 0.0;
        return;
    }

    uint64_t cacheMisses = 0;
    m_pipeline->GetStatistics(totalRequests, cacheHits, cacheMisses, averageTimeMs);
}

}  // namespace ExplorerLens
