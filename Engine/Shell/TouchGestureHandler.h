// TouchGestureHandler.h — Touch and Pen Gesture Processing
// Copyright (c) 2026 ExplorerLens Project
//
// Processes touch and pen input gestures (tap, pinch, pan, swipe, hold)
// for the thumbnail preview UI with configurable thresholds.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <functional>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

enum class GestureType : uint32_t {
    Tap = 0,
    Pinch = 1,
    Pan = 2,
    Swipe = 3,
    Hold = 4
};

struct GestureEvent {
    GestureType type = GestureType::Tap;
    uint64_t    timestampMs = 0;
    float       x = 0.0f;
    float       y = 0.0f;
    float       deltaX = 0.0f;
    float       deltaY = 0.0f;
    float       scale = 1.0f;   // For pinch
    float       velocity = 0.0f;   // pixels/sec for swipe
    uint32_t    touchCount = 1;
    bool        isPen = false;
    float       pressure = 0.0f;   // Pen pressure 0.0 - 1.0

    float Distance() const {
        return std::sqrt(deltaX * deltaX + deltaY * deltaY);
    }
};

struct GestureConfig {
    float    tapMaxDistance = 10.0f;  // pixels
    uint32_t tapMaxDurationMs = 300;
    uint32_t holdMinDurationMs = 500;
    float    swipeMinVelocity = 200.0f; // pixels/sec
    float    pinchMinScale = 0.1f;
    float    panMinDistance = 5.0f;
};

class TouchGestureHandler {
public:
    using GestureCallback = std::function<void(const GestureEvent&)>;

    static TouchGestureHandler& Instance() {
        static TouchGestureHandler s;
        return s;
    }

    void RegisterGestures(HWND hwnd) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_hwnd = hwnd;
        m_registered = true;
        // In production, would call RegisterTouchWindow here
    }

    void OnTouch(float x, float y, uint32_t touchCount, bool isPen, float pressure) {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t now = GetTickCount64();

        if (!m_touchActive) {
            m_touchActive = true;
            m_touchStartX = x;
            m_touchStartY = y;
            m_touchStartTime = now;
            m_lastX = x;
            m_lastY = y;
            m_touchCount = touchCount;
            return;
        }

        float dx = x - m_touchStartX;
        float dy = y - m_touchStartY;
        float dist = std::sqrt(dx * dx + dy * dy);
        uint64_t duration = now - m_touchStartTime;

        GestureEvent evt;
        evt.timestampMs = now;
        evt.x = x;
        evt.y = y;
        evt.deltaX = x - m_lastX;
        evt.deltaY = y - m_lastY;
        evt.touchCount = touchCount;
        evt.isPen = isPen;
        evt.pressure = pressure;

        if (touchCount >= 2) {
            evt.type = GestureType::Pinch;
            evt.scale = dist > 0 ? dist / 100.0f : 1.0f;
        }
        else if (dist > m_config.panMinDistance && duration < m_config.tapMaxDurationMs) {
            float velocity = duration > 0 ? (dist * 1000.0f / duration) : 0.0f;
            if (velocity > m_config.swipeMinVelocity) {
                evt.type = GestureType::Swipe;
                evt.velocity = velocity;
            }
            else {
                evt.type = GestureType::Pan;
            }
        }
        else if (duration >= m_config.holdMinDurationMs && dist < m_config.tapMaxDistance) {
            evt.type = GestureType::Hold;
        }
        else if (dist < m_config.tapMaxDistance) {
            evt.type = GestureType::Tap;
        }

        m_activeGestures.push_back(evt);
        if (m_activeGestures.size() > 100)
            m_activeGestures.erase(m_activeGestures.begin());

        // Fire callback
        auto cbIt = m_callbacks.find(static_cast<uint32_t>(evt.type));
        if (cbIt != m_callbacks.end() && cbIt->second) {
            cbIt->second(evt);
        }

        m_lastX = x;
        m_lastY = y;
        m_totalGestures++;
    }

    void OnTouchEnd() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_touchActive = false;
    }

    void SetCallback(GestureType type, GestureCallback callback) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_callbacks[static_cast<uint32_t>(type)] = callback;
    }

    void SetConfig(const GestureConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config = config;
    }

    std::vector<GestureEvent> GetActiveGestures() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_activeGestures;
    }

    size_t GetGestureCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_activeGestures.size();
    }

    uint64_t GetTotalGestures() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_totalGestures;
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_activeGestures.clear();
        m_callbacks.clear();
        m_touchActive = false;
        m_totalGestures = 0;
        m_config = GestureConfig{};
    }

    bool Validate() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_config.tapMaxDistance < 0.0f) return false;
        if (m_config.swipeMinVelocity < 0.0f) return false;
        if (m_config.holdMinDurationMs == 0) return false;
        for (const auto& evt : m_activeGestures) {
            if (static_cast<uint32_t>(evt.type) > 4) return false;
            if (evt.pressure < 0.0f || evt.pressure > 1.0f) {
                if (evt.isPen) return false;
            }
        }
        return true;
    }

private:
    TouchGestureHandler() = default;
    ~TouchGestureHandler() = default;
    TouchGestureHandler(const TouchGestureHandler&) = delete;
    TouchGestureHandler& operator=(const TouchGestureHandler&) = delete;

    mutable std::mutex m_mutex;
    HWND m_hwnd = nullptr;
    bool m_registered = false;
    bool m_touchActive = false;
    float m_touchStartX = 0.0f;
    float m_touchStartY = 0.0f;
    float m_lastX = 0.0f;
    float m_lastY = 0.0f;
    uint64_t m_touchStartTime = 0;
    uint32_t m_touchCount = 0;
    uint64_t m_totalGestures = 0;
    GestureConfig m_config;
    std::vector<GestureEvent> m_activeGestures;
    std::unordered_map<uint32_t, GestureCallback> m_callbacks;
};

}
} // namespace ExplorerLens::Engine
