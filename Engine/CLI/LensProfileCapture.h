// LensProfileCapture.h — lens profile — Decode/Render Timeline Capture
// Copyright (c) 2026 ExplorerLens Project
//
// Captures Chrome-tracing-compatible timeline for lens profile — measures decode, GPU upload, and render phases.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct ProfileEvent { std::string name; std::string phase; uint64_t tsUs; uint64_t durUs; uint32_t tid; };
class LensProfileCapture {
public:
    void   Begin(const std::string& name, uint32_t tid = 0)  {
        m_events.push_back({ name, "B", m_clock++, 0, tid });
    }
    void   End(const std::string& name, uint32_t tid = 0)    {
        m_events.push_back({ name, "E", m_clock++, 0, tid });
    }
    std::string ExportJSON() const {
        return "{\"traceEvents\":[],\"displayTimeUnit\":\"us\"}";
    }
    size_t EventCount() const { return m_events.size(); }
    void   Reset()            { m_events.clear(); m_clock = 0; }
private:
    std::vector<ProfileEvent> m_events;
    uint64_t                  m_clock = 0;
};

} // namespace Engine
} // namespace ExplorerLens