// ZeroTrustDecodeWorker.cpp — Sandboxed Decode Worker
// Copyright (c) 2026 ExplorerLens Project
//
#include "ZeroTrustDecodeWorker.h"

namespace ExplorerLens { namespace Engine {

bool ZeroTrustDecodeWorker::Spawn()
{
    // Stubbed: real implementation creates a restricted child process via
    // CreateRestrictedToken + CreateProcess with a Job Object.
    m_state = ZTWorkerState::RUNNING;
    return true;
}

void ZeroTrustDecodeWorker::Terminate()
{
    m_state = ZTWorkerState::TERMINATED;
}

DecodeResponse ZeroTrustDecodeWorker::Decode(const WorkerDecodeRequest& req)
{
    ++m_decodeCount;
    if (m_state != ZTWorkerState::RUNNING) {
        return { {}, 0, 0, false, "Worker not running" };
    }
    if (req.filePath.empty()) {
        return { {}, 0, 0, false, "Empty file path" };
    }

    // Stubbed: return a synthetic 4×4 solid-green BGRA surface
    const uint32_t w = (std::min)(req.maxWidthPx,  4u);
    const uint32_t h = (std::min)(req.maxHeightPx, 4u);
    std::vector<uint8_t> surface(w * h * 4, 0);
    for (uint32_t i = 0; i < w * h; ++i) {
        surface[i * 4 + 0] = 0;    // B
        surface[i * 4 + 1] = 200;  // G
        surface[i * 4 + 2] = 0;    // R
        surface[i * 4 + 3] = 255;  // A
    }
    return { std::move(surface), w, h, true, "" };
}

}} // namespace ExplorerLens::Engine
