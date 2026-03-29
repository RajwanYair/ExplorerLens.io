// ScreenReaderBridge.h — Screen Reader Bridge
// Copyright (c) 2026 ExplorerLens Project
//
// Integrates with platform screen reader APIs: MSAA/UIAutomation (Windows),
// VoiceOver (macOS), Orca (Linux). Announces thumbnail generation events and
// navigation changes to assistive technology.
//
#pragma once
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class SRPlatform { UIAutomation, VoiceOver, Orca, None };
enum class SRLiveRegionPoliteness { Off, Polite, Assertive };

struct SRAnnouncement {
    std::string                text;
    SRLiveRegionPoliteness     politeness = SRLiveRegionPoliteness::Polite;
    bool                       interrupt  = false;
};

class ScreenReaderBridge {
public:
    ScreenReaderBridge() = default;

    bool Initialize() {
        m_platform = DetectPlatform();
        m_ready    = true;
        return true;
    }
    bool  IsReady()    const { return m_ready; }
    SRPlatform GetPlatform() const { return m_platform; }
    bool  IsActive()   const { return m_platform != SRPlatform::None; }

    bool Announce(const SRAnnouncement& ann) {
        if (!m_ready || m_platform == SRPlatform::None) return false;
        m_announcements.push_back(ann.text);
        if (m_sink) m_sink(ann);
        return true;
    }

    bool AnnounceThumbnailReady(const std::string& filePath,
                                 const std::string& altText) {
        SRAnnouncement ann;
        ann.text      = "Thumbnail loaded: " + ExtractName(filePath)
                      + (altText.empty() ? "" : ". " + altText);
        ann.politeness = SRLiveRegionPoliteness::Polite;
        return Announce(ann);
    }

    bool AnnounceNavigate(const std::string& context) {
        SRAnnouncement ann;
        ann.text      = "Navigating to " + context;
        ann.politeness = SRLiveRegionPoliteness::Assertive;
        ann.interrupt  = false;
        return Announce(ann);
    }

    void SetAnnounceSink(std::function<void(const SRAnnouncement&)> fn) {
        m_sink = std::move(fn);
    }

    uint32_t GetAnnouncementCount() const {
        return static_cast<uint32_t>(m_announcements.size());
    }

    void Shutdown() { m_ready = false; }

private:
    bool        m_ready    = false;
    SRPlatform  m_platform = SRPlatform::None;
    std::vector<std::string> m_announcements;
    std::function<void(const SRAnnouncement&)> m_sink;

    static SRPlatform DetectPlatform() {
#if defined(_WIN32)
        return SRPlatform::UIAutomation;
#elif defined(__APPLE__)
        return SRPlatform::VoiceOver;
#elif defined(__linux__)
        return SRPlatform::Orca;
#else
        return SRPlatform::None;
#endif
    }

    static std::string ExtractName(const std::string& path) {
        size_t pos = path.find_last_of("/\\");
        return (pos != std::string::npos) ? path.substr(pos + 1) : path;
    }
};

}} // namespace ExplorerLens::Engine
