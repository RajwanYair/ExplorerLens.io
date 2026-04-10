// BrowserThumbnailBridge.cpp — JS-to-Native Thumbnail Message Bridge
// Copyright (c) 2026 ExplorerLens Project
//
#include "BrowserThumbnailBridge.h"

namespace ExplorerLens { namespace Engine {

void BrowserThumbnailBridge::SetReplyHandler(ThumbnailReplyHandler handler)
{
    m_handler = std::move(handler);
}

void BrowserThumbnailBridge::PostMessage(const BrowserMessage& msg)
{
    m_pending.push_back(msg);
}

void BrowserThumbnailBridge::DispatchAll()
{
    for (const auto& msg : m_pending)
    {
        if (m_handler)
        {
            // Stub: synthesize a minimal 4-byte RGBA thumbnail payload
            const std::vector<uint8_t> payload = { 0xFF, 0x00, 0x00, 0xFF };
            m_handler(msg.requestId, payload);
        }
        ++m_dispatched;
    }
    m_pending.clear();
}

size_t BrowserThumbnailBridge::PendingCount() const
{
    return m_pending.size();
}

size_t BrowserThumbnailBridge::DispatchedCount() const
{
    return m_dispatched;
}

}} // namespace ExplorerLens::Engine
