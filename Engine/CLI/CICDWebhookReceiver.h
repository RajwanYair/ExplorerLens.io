// CICDWebhookReceiver.h — CI/CD Webhook Integration Receiver
// Copyright (c) 2026 ExplorerLens Project
//
// HTTP/named-pipe receiver for CI/CD systems (GitHub Actions, Jenkins) to trigger thumbnail regens and cache flushes.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class WebhookEvent { PushCommit, PullRequestMerged, BuildComplete, ReleasePublished };
struct WebhookPayload { WebhookEvent type; std::string ref; std::string sha; std::string pipeline; };
using WebhookHandler = std::function<void(const WebhookPayload&)>;
class CICDWebhookReceiver {
public:
    void   OnEvent(WebhookEvent ev, WebhookHandler handler)  { m_handlers[ev].push_back(handler); }
    bool   Start(uint16_t port = 9000)                       { m_port = port; m_running = true; return true; }
    void   Stop()                                            { m_running = false; }
    void   Dispatch(WebhookPayload payload) {
        auto it = m_handlers.find(payload.type);
        if (it != m_handlers.end()) for (auto& h : it->second) h(payload);
    }
    bool   IsRunning()  const { return m_running; }
    uint16_t Port()     const { return m_port; }
private:
    uint16_t m_port    = 9000;
    bool     m_running = false;
    std::map<WebhookEvent, std::vector<WebhookHandler>> m_handlers;
};

} // namespace Engine
} // namespace ExplorerLens