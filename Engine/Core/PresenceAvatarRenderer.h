// PresenceAvatarRenderer.h — Presence Avatar Renderer (Cursor + Badge Overlays)
// Copyright (c) 2026 ExplorerLens Project
//
// Renders colourful avatar badges and cursor overlays for all connected collaboration
// participants, compositing them onto the thumbnail grid in real time.
//
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct AvatarRenderConfig
{
    int badgeSize = 24;
    float opacity = 0.85f;
    bool showName = true;
    bool showCursor = true;
};

struct AvatarParticipant
{
    std::string userId;
    std::string initials;  // max 2 chars
    std::array<uint8_t, 3> color;
    std::array<float, 2> position;  // normalised [0,1] on canvas
    bool isMicOn = false;
};

struct AvatarFrame
{
    std::vector<uint8_t> rgba;  // overlay alpha-composited onto parent
    int widthPx = 0;
    int heightPx = 0;
    int participantCount = 0;
};

class PresenceAvatarRenderer
{
  public:
    explicit PresenceAvatarRenderer(AvatarRenderConfig config = {}) : m_config(std::move(config)) {}

    AvatarFrame Render(const std::vector<AvatarParticipant>& participants, int canvasWidth, int canvasHeight) const
    {
        AvatarFrame frame;
        frame.widthPx = canvasWidth;
        frame.heightPx = canvasHeight;
        frame.participantCount = static_cast<int>(participants.size());
        // RGBA overlay; fully transparent background
        frame.rgba.assign(static_cast<size_t>(canvasWidth) * canvasHeight * 4, 0x00);
        // Paint a badge pixel cluster per participant (stub)
        for (const auto& p : participants) {
            int bx = static_cast<int>(p.position[0] * canvasWidth);
            int by = static_cast<int>(p.position[1] * canvasHeight);
            PaintBadge(frame.rgba, bx, by, canvasWidth, canvasHeight, p.color);
        }
        return frame;
    }

    const AvatarRenderConfig& Config() const noexcept
    {
        return m_config;
    }
    void SetConfig(AvatarRenderConfig cfg) noexcept
    {
        m_config = std::move(cfg);
    }

  private:
    void PaintBadge(std::vector<uint8_t>& rgba, int cx, int cy, int w, int h, const std::array<uint8_t, 3>& color) const
    {
        int r = m_config.badgeSize / 2;
        for (int dy = -r; dy <= r; ++dy) {
            for (int dx = -r; dx <= r; ++dx) {
                int px = cx + dx, py = cy + dy;
                if (px < 0 || py < 0 || px >= w || py >= h)
                    continue;
                if (dx * dx + dy * dy > r * r)
                    continue;
                size_t idx = static_cast<size_t>(py * w + px) * 4;
                rgba[idx] = color[0];
                rgba[idx + 1] = color[1];
                rgba[idx + 2] = color[2];
                rgba[idx + 3] = static_cast<uint8_t>(m_config.opacity * 255);
            }
        }
    }

    AvatarRenderConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
