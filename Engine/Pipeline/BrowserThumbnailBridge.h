// BrowserThumbnailBridge.h — JS-to-Native Thumbnail Message Bridge
// Copyright (c) 2026 ExplorerLens Project
//
// Async message bridge that forwards thumbnail requests from a browser
// JavaScript context to the native ExplorerLens engine and delivers
// rendered thumbnail payloads back via registered reply handlers.
//
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class BrowserMessageKind : uint32_t
{
    REQUEST_THUMBNAIL  = 0,
    CANCEL_REQUEST     = 1,
    PREFETCH_HINT      = 2,
    PING               = 3
};

struct BrowserMessage
{
    BrowserMessageKind kind      = BrowserMessageKind::REQUEST_THUMBNAIL;
    uint32_t           requestId = 0u;
    std::wstring       filePath;
    uint32_t           widthHint  = 0u;
    uint32_t           heightHint = 0u;
};

using ThumbnailReplyHandler =
    std::function<void(uint32_t requestId, const std::vector<uint8_t>& payload)>;

class BrowserThumbnailBridge
{
public:
    BrowserThumbnailBridge()  = default;
    ~BrowserThumbnailBridge() = default;

    BrowserThumbnailBridge(const BrowserThumbnailBridge&)            = delete;
    BrowserThumbnailBridge& operator=(const BrowserThumbnailBridge&) = delete;

    void SetReplyHandler(ThumbnailReplyHandler handler);

    void PostMessage(const BrowserMessage& msg);
    void DispatchAll();

    size_t PendingCount()    const;
    size_t DispatchedCount() const;

private:
    ThumbnailReplyHandler    m_handler;
    std::vector<BrowserMessage> m_pending;
    size_t                   m_dispatched = 0u;
};

}} // namespace ExplorerLens::Engine
