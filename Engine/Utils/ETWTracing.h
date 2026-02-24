//==============================================================================
// ETWTracing.h - Event Tracing for Windows Integration
// Copyright (c) 2026 - ExplorerLens Project
// 
// Provides performance tracing for Windows Performance Analyzer (WPA)
//==============================================================================

#pragma once

#include <windows.h>
#include <evntprov.h>
#include <string>

#pragma comment(lib, "Advapi32.lib")

namespace ExplorerLens {
namespace Tracing {

//==============================================================================
// ETW Provider GUID: {A1B2C3D4-E5F6-4748-9A8B-9C8D7E6F5A4B}
// Register with: wevtutil im ExplorerLensManifest.xml
//==============================================================================

const GUID EXPLORERLENS_PROVIDER_GUID = {
    0xA1B2C3D4, 0xE5F6, 0x4748,
    {0x9A, 0x8B, 0x9C, 0x8D, 0x7E, 0x6F, 0x5A, 0x4B}
};

//==============================================================================
// ETW Event IDs
//==============================================================================

enum class EventID : uint16_t {
    ThumbnailGeneration_Start = 1,
    ThumbnailGeneration_Stop = 2,
    Decode_Start = 10,
    Decode_Stop = 11,
    GPU_Render_Start = 20,
    GPU_Render_Stop = 21,
    Cache_Hit = 30,
    Cache_Miss = 31,
    Error = 100,
    Warning = 101,
    Info = 102
};

//==============================================================================
// ETW Event Levels (matches Windows ETW levels)
//==============================================================================

enum class EventLevel : uint8_t {
    Critical = 1,
    Error = 2,
    Warning = 3,
    Info = 4,
    Verbose = 5
};

//==============================================================================
// ETW Tracer
//==============================================================================

class ETWTracer {
public:
    static ETWTracer& Get() {
        static ETWTracer instance;
        return instance;
    }

    // Initialize ETW provider
    bool Initialize();
    
    // Shutdown ETW provider
    void Shutdown();
    
    // Check if ETW is enabled
    bool IsEnabled() const { return m_enabled && m_registered; }
    
    // Log thumbnail generation event
    void LogThumbnailStart(const wchar_t* filePath, uint32_t width, uint32_t height);
    void LogThumbnailStop(const wchar_t* filePath, HRESULT hr, uint32_t durationMs);
    
    // Log decoder events
    void LogDecodeStart(const wchar_t* decoderName, const wchar_t* filePath);
    void LogDecodeStop(const wchar_t* decoderName, HRESULT hr, uint32_t durationMs);
    
    // Log GPU events
    void LogGPURenderStart(uint32_t width, uint32_t height);
    void LogGPURenderStop(HRESULT hr, uint32_t durationMs);
    
    // Log cache events
    void LogCacheHit(const wchar_t* filePath);
    void LogCacheMiss(const wchar_t* filePath);
    
    // Generic logging
    void LogError(const wchar_t* message);
    void LogWarning(const wchar_t* message);
    void LogInfo(const wchar_t* message);

private:
    ETWTracer() = default;
    ~ETWTracer();
    
    ETWTracer(const ETWTracer&) = delete;
    ETWTracer& operator=(const ETWTracer&) = delete;
    
    // Write ETW event
    void WriteEvent(EventID eventId, EventLevel level, const wchar_t* message);
    
    REGHANDLE m_providerHandle = 0;
    bool m_registered = false;
    bool m_enabled = false;
};

//==============================================================================
// RAII Scope Tracer
//==============================================================================

class ETWScope {
public:
    ETWScope(EventID startEvent, EventID stopEvent, const wchar_t* name)
        : m_stopEvent(stopEvent), m_name(name), m_startTime(GetTickCount64()) {
        // Log start event
        if (ETWTracer::Get().IsEnabled()) {
            // Start event logged
        }
    }
    
    ~ETWScope() {
        if (ETWTracer::Get().IsEnabled()) {
            uint32_t duration = static_cast<uint32_t>(GetTickCount64() - m_startTime);
            // Log stop event with duration
        }
    }

private:
    EventID m_stopEvent;
    const wchar_t* m_name;
    uint64_t m_startTime;
};

// Convenience macros
#define ETW_TRACE_THUMBNAIL(filePath) \
    ExplorerLens::Tracing::ETWScope _etwScope_##__LINE__( \
        ExplorerLens::Tracing::EventID::ThumbnailGeneration_Start, \
        ExplorerLens::Tracing::EventID::ThumbnailGeneration_Stop, \
        filePath)

#define ETW_TRACE_DECODE(decoderName) \
    ExplorerLens::Tracing::ETWScope _etwScope_##__LINE__( \
        ExplorerLens::Tracing::EventID::Decode_Start, \
        ExplorerLens::Tracing::EventID::Decode_Stop, \
        decoderName)

} // namespace Tracing
} // namespace ExplorerLens


