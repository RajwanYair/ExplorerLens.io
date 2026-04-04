// StreamingThumbnailEmitter.h — Progressive thumbnail refinement
// Copyright (c) 2026 ExplorerLens Project
//
// Emits progressive thumbnail refinements: first a quick low-res pass from
// embedded EXIF, then full-quality decode, enabling fast first-paint.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct StreamingThumbnailEmitterConfig
{
    bool enabled = true;
    uint32_t lowResSize = 64;
    uint32_t fullResSize = 256;
    std::string label = "StreamingThumbnailEmitter";
};

class StreamingThumbnailEmitter
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    StreamingThumbnailEmitterConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    enum class Phase : uint8_t {
        None,
        LowRes,
        FullRes,
        Complete
    };

    Phase GetCurrentPhase() const
    {
        return m_phase;
    }

    bool AdvancePhase()
    {
        switch (m_phase) {
            case Phase::None:
                m_phase = Phase::LowRes;
                return true;
            case Phase::LowRes:
                m_phase = Phase::FullRes;
                return true;
            case Phase::FullRes:
                m_phase = Phase::Complete;
                return true;
            case Phase::Complete:
                return false;
        }
        return false;
    }

    void Reset()
    {
        m_phase = Phase::None;
    }
    bool IsComplete() const
    {
        return m_phase == Phase::Complete;
    }

  private:
    bool m_initialized = false;
    StreamingThumbnailEmitterConfig m_config;
    Phase m_phase = Phase::None;
};

}  // namespace Engine
}  // namespace ExplorerLens
