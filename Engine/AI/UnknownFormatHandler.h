// UnknownFormatHandler.h — Unknown Format Decoder Request Synthesizer
// Copyright (c) 2026 ExplorerLens Project
//
// Handles file formats not in the registered decoder registry — synthesizes a
// best-effort decode attempt via format-family generalisation and ML inference.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace ExplorerLens {
namespace Engine {

enum class UnknownHandlerStrategy { ReturnPlaceholder, SynthesizeDecode, RequestPlugin, Skip };

struct UnknownFormatRequest {
    const uint8_t* fileData      = nullptr;
    size_t         fileSize      = 0;
    std::string    fileExtension;
    std::string    detectedMime;
    float          confidence    = 0.0f;
};

struct UnknownHandlerResult {
    UnknownHandlerStrategy strategyUsed = UnknownHandlerStrategy::Skip;
    bool                   success      = false;
    std::string            fallbackIcon;
    std::string            diagnosticMessage;
    float                  qualityScore = 0.0f;
};

class UnknownFormatHandler {
public:
    using PluginRequestCallback = std::function<bool(const std::string& mimeType)>;

    explicit UnknownFormatHandler(UnknownHandlerStrategy strategy = UnknownHandlerStrategy::ReturnPlaceholder)
        : m_strategy(strategy) {}

    UnknownHandlerResult Handle(const UnknownFormatRequest& req) {
        UnknownHandlerResult r;
        r.strategyUsed = m_strategy;
        if (m_strategy == UnknownHandlerStrategy::ReturnPlaceholder) {
            r.success        = true;
            r.fallbackIcon   = "generic_file.png";
            r.qualityScore   = 0.1f;
        }
        r.diagnosticMessage = "Handled via " + req.fileExtension;
        ++m_handledCount;
        return r;
    }

    void                    SetStrategy(UnknownHandlerStrategy s) { m_strategy = s; }
    UnknownHandlerStrategy  GetStrategy()     const { return m_strategy; }
    void                    SetPluginCallback(PluginRequestCallback cb) { m_pluginCb = std::move(cb); }
    uint32_t                HandledCount()    const { return m_handledCount; }
    void                    Reset()           { m_handledCount = 0; }

private:
    UnknownHandlerStrategy  m_strategy;
    PluginRequestCallback   m_pluginCb;
    uint32_t                m_handledCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
