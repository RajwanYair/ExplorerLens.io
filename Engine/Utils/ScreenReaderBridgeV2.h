// ScreenReaderBridgeV2.h — Screen Reader Bridge v2 (NVDA / JAWS / Narrator)
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a unified screen reader notification bridge for NVDA, JAWS, and Windows
// Narrator, routing live region changes and thumbnail-ready announcements through
// UI Automation or direct IAccessible2 notifications.
//
#pragma once
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ScreenReaderType {
    NVDA,
    JAWS,
    Narrator,
    VoiceOver,
    Orca,
    Unknown
};
enum class LiveRegionPoliteness {
    Off,
    Polite,
    Assertive,
    Rude
};

struct ScreenReaderAnnouncement
{
    std::string text;
    LiveRegionPoliteness politeness = LiveRegionPoliteness::Polite;
    bool atomic = false;
    bool relevant = true;
};

struct ScreenReaderCapabilities
{
    bool supportsMathML = false;
    bool supportsLiveReg = true;
    bool supportsImages = true;
    bool supportsTables = true;
};

using AnnounceFn = std::function<bool(const ScreenReaderAnnouncement&)>;

class ScreenReaderBridgeV2
{
  public:
    explicit ScreenReaderBridgeV2() = default;
    void SetAnnounceFn(AnnounceFn fn)
    {
        m_announceFn = std::move(fn);
    }
    void SetActiveReader(ScreenReaderType type) noexcept
    {
        m_activeReader = type;
    }
    void SetAvailable(bool available) noexcept
    {
        m_available = available;
    }

    bool Announce(const std::string& text, LiveRegionPoliteness politeness = LiveRegionPoliteness::Polite)
    {
        if (!m_available || m_activeReader == ScreenReaderType::Unknown)
            return false;
        ScreenReaderAnnouncement ann{text, politeness, false, true};
        if (m_announceFn)
            return m_announceFn(ann);
        return false;
    }

    bool AnnounceThumbnailReady(const std::wstring& fileName)
    {
        std::string msg = "Thumbnail ready: ";
        for (wchar_t c : fileName)
            msg += (c < 128) ? static_cast<char>(c) : '?';
        return Announce(msg, LiveRegionPoliteness::Polite);
    }

    bool IsAvailable() const noexcept
    {
        return m_available;
    }
    ScreenReaderType ActiveReader() const noexcept
    {
        return m_activeReader;
    }

    static std::string ReaderName(ScreenReaderType t) noexcept
    {
        switch (t) {
            case ScreenReaderType::NVDA:
                return "NVDA";
            case ScreenReaderType::JAWS:
                return "JAWS";
            case ScreenReaderType::Narrator:
                return "Narrator";
            case ScreenReaderType::VoiceOver:
                return "VoiceOver";
            case ScreenReaderType::Orca:
                return "Orca";
            case ScreenReaderType::Unknown:
                return "Unknown";
        }
        return "Unknown";
    }

  private:
    bool m_available = false;
    ScreenReaderType m_activeReader = ScreenReaderType::Unknown;
    AnnounceFn m_announceFn;
};

}  // namespace Engine
}  // namespace ExplorerLens
