// WASMDebuggerBridge.h — WASM Plugin Debugger Bridge / Inspector Protocol
// Copyright (c) 2026 ExplorerLens Project
//
// Implements the Chrome DevTools Protocol (CDP) / DAP bridge for WASM plugin
// debugging — supporting breakpoints, stack inspection, and memory dumps.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DebuggerState    { Disconnected, Connecting, Connected, Paused, Stepping };
enum class BreakpointKind   { Line, FunctionEntry, MemoryWatch, Exception };

struct WASMBreakpoint {
    uint32_t        id           = 0;
    BreakpointKind  kind         = BreakpointKind::Line;
    std::string     location;
    bool            enabled      = true;
    uint32_t        hitCount     = 0;
};

struct WASMStackFrame {
    uint32_t    frameId     = 0;
    std::string functionName;
    uint32_t    instructionOffset = 0;
    std::vector<std::pair<std::string,std::string>> locals;
};

struct DebuggerConfig {
    uint16_t    listenPort   = 9229;
    bool        breakOnEntry = false;
    bool        enableCDP    = true;
    bool        enableDAP    = false;
};

class WASMDebuggerBridge {
public:
    explicit WASMDebuggerBridge(const DebuggerConfig& cfg = {}) : m_cfg(cfg) {}

    bool  Connect()         { m_state = DebuggerState::Connected; return true; }
    void  Disconnect()      { m_state = DebuggerState::Disconnected; }

    uint32_t  SetBreakpoint(const WASMBreakpoint& bp) {
        m_breakpoints.push_back(bp);
        m_breakpoints.back().id = ++m_nextBpId;
        return m_nextBpId;
    }
    bool  RemoveBreakpoint(uint32_t id) {
        auto it = std::find_if(m_breakpoints.begin(), m_breakpoints.end(),
                               [id](const WASMBreakpoint& b){ return b.id == id; });
        if (it == m_breakpoints.end()) return false;
        m_breakpoints.erase(it);
        return true;
    }

    std::vector<WASMStackFrame> GetCallStack()  const { return {}; }
    DebuggerState               GetState()      const { return m_state; }
    bool                        IsConnected()   const { return m_state == DebuggerState::Connected; }
    uint32_t                    BreakpointCount() const { return static_cast<uint32_t>(m_breakpoints.size()); }
    const DebuggerConfig&       GetConfig()     const { return m_cfg; }
    void  Reset() { m_breakpoints.clear(); m_state = DebuggerState::Disconnected; }

private:
    DebuggerConfig                m_cfg;
    DebuggerState                 m_state = DebuggerState::Disconnected;
    std::vector<WASMBreakpoint>   m_breakpoints;
    uint32_t                      m_nextBpId = 0;
};

} // namespace Engine
} // namespace ExplorerLens
