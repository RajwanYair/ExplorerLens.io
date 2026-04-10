// MultiStageThumbnailEmitter.cpp — Progressive Multi-Stage Thumbnail Emitter
// Copyright (c) 2026 ExplorerLens Project
//
#include "MultiStageThumbnailEmitter.h"
#include <sstream>

namespace ExplorerLens { namespace Engine {

MultiStageThumbnailEmitter::MultiStageThumbnailEmitter(StageCallback cb)
    : m_callback(std::move(cb))
{}

void MultiStageThumbnailEmitter::SetConfig(const Config& cfg)
{
    m_config = cfg;
}

StageResult MultiStageThumbnailEmitter::EmitStage(
    EmitterFidelity fidelity, uint32_t w, uint32_t h, uint32_t simulatedMs)
{
    StageResult r;
    r.fidelity   = fidelity;
    r.width      = w;
    r.height     = h;
    r.durationMs = simulatedMs;
    r.success    = true;
    if (m_callback) m_callback(r);
    return r;
}

bool MultiStageThumbnailEmitter::Emit(const std::wstring& /*filePath*/)
{
    m_cancel.store(false);
    m_wasCancelled = false;
    m_stagesCompleted = 0;

    if (m_config.emitPlaceholder)
    {
        if (m_cancel.load()) { m_wasCancelled = true; return false; }
        EmitStage(EmitterFidelity::PLACEHOLDER, 64, 64, m_config.placeholderMs);
        ++m_stagesCompleted;
    }
    if (m_config.emitLowRes)
    {
        if (m_cancel.load()) { m_wasCancelled = true; return false; }
        EmitStage(EmitterFidelity::LOW_RES, 256, 256, m_config.lowResMs);
        ++m_stagesCompleted;
    }
    if (m_config.emitFullRes)
    {
        if (m_cancel.load()) { m_wasCancelled = true; return false; }
        EmitStage(EmitterFidelity::FULL_RES, 512, 512, m_config.fullResMs);
        ++m_stagesCompleted;
    }
    return true;
}

void MultiStageThumbnailEmitter::Cancel()
{
    m_cancel.store(true);
}

bool MultiStageThumbnailEmitter::WasCancelled() const
{
    return m_wasCancelled;
}

uint32_t MultiStageThumbnailEmitter::StagesCompleted() const
{
    return m_stagesCompleted;
}

std::wstring MultiStageThumbnailEmitter::Summary() const
{
    std::wostringstream oss;
    oss << L"MultiStageThumbnailEmitter: stages=" << m_stagesCompleted
        << L" cancelled=" << (m_wasCancelled ? L"yes" : L"no");
    return oss.str();
}

}} // namespace ExplorerLens::Engine
