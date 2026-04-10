// ZeroTrustDecodeWorker.h — Sandboxed Decode Worker
// Copyright (c) 2026 ExplorerLens Project
//
// Represents a sandboxed decode worker process that runs with reduced privileges
// (no network, no filesystem writes) to isolate untrusted format parsers.
// On Windows the worker runs inside a Job Object with restricted token.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class WorkerState : uint8_t {
    IDLE       = 0,
    RUNNING    = 1,
    FAILED     = 2,
    TERMINATED = 3,
};

struct DecodeRequest {
    std::wstring           filePath;
    std::string            formatHint;   // e.g. "pdf", "psd"
    uint32_t               maxWidthPx   = 256;
    uint32_t               maxHeightPx  = 256;
};

struct DecodeResponse {
    std::vector<uint8_t>   bgraSurface;
    uint32_t               width        = 0;
    uint32_t               height       = 0;
    bool                   success      = false;
    std::string            error;
};

class ZeroTrustDecodeWorker {
public:
    struct Config {
        uint32_t    timeoutMs           = 5000;
        bool        allowNetworkAccess  = false;
        bool        allowFileWrite      = false;
    };

    explicit ZeroTrustDecodeWorker(const Config& cfg = {}) : m_cfg(cfg) {}

    bool         Spawn();
    void         Terminate();
    DecodeResponse Decode(const DecodeRequest& req);

    WorkerState  GetState()           const { return m_state; }
    uint32_t     DecodeCount()        const { return m_decodeCount; }
    uint32_t     TimeoutCount()       const { return m_timeoutCount; }
    bool         IsAlive()            const { return m_state == WorkerState::RUNNING; }

    const Config& GetConfig()         const { return m_cfg; }

private:
    Config      m_cfg;
    WorkerState m_state        = WorkerState::IDLE;
    uint32_t    m_decodeCount  = 0;
    uint32_t    m_timeoutCount = 0;
};

}} // namespace ExplorerLens::Engine
