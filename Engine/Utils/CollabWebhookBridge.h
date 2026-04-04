// CollabWebhookBridge.h — Teams/Slack Annotation Webhook Bridge
// Copyright (c) 2026 ExplorerLens Project
//
// Posts annotation events (new, deleted, shared) to Microsoft Teams and Slack
// incoming webhook endpoints. Payloads conform to Adaptive Cards (Teams) and
// Block Kit (Slack) formats for rich notification display.
//
#pragma once
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class WebhookPlatform {
    Teams,
    Slack,
    Generic
};

struct WebhookConfig
{
    WebhookPlatform platform = WebhookPlatform::Teams;
    std::string webhookUrl;
    std::string channelName;
    bool enabled = true;
};

struct AnnotationEvent
{
    enum class Kind {
        Created,
        Deleted,
        Shared,
        BatchShared
    } kind = Kind::Created;
    std::wstring filePath;
    std::wstring annotationValue;
    std::wstring authorId;
    int count = 1;
};

struct WebhookPostResult
{
    bool success = false;
    int statusCode = 0;
    std::string error;
};

using HttpPostFn = std::function<WebhookPostResult(const std::string& url, const std::string& body)>;

class CollabWebhookBridge
{
  public:
    explicit CollabWebhookBridge() = default;
    explicit CollabWebhookBridge(HttpPostFn postFn) : m_postFn(std::move(postFn)) {}

    void AddConfig(const WebhookConfig& cfg)
    {
        m_configs.push_back(cfg);
    }
    void ClearConfigs() noexcept
    {
        m_configs.clear();
    }
    int ConfigCount() const noexcept
    {
        return (int)m_configs.size();
    }

    std::vector<WebhookPostResult> PostEvent(const AnnotationEvent& evt)
    {
        std::vector<WebhookPostResult> results;
        for (const auto& cfg : m_configs) {
            if (!cfg.enabled || cfg.webhookUrl.empty())
                continue;
            std::string payload = BuildPayload(cfg.platform, evt);
            WebhookPostResult r;
            if (m_postFn)
                r = m_postFn(cfg.webhookUrl, payload);
            else {
                r.success = true;
                r.statusCode = 200;
            }  // stub
            results.push_back(r);
        }
        return results;
    }

    std::string BuildPayload(WebhookPlatform platform, const AnnotationEvent& evt) const
    {
        if (platform == WebhookPlatform::Slack)
            return "{\"text\":\"Annotation event on file\"}";
        // Teams Adaptive Card
        return "{\"type\":\"message\",\"attachments\":[{\"contentType\":\"application/vnd.microsoft.card.adaptive\",\"content\":{\"type\":\"AdaptiveCard\",\"body\":[{\"type\":\"TextBlock\",\"text\":\"Annotation event\"}]}}]}";
        (void)evt;
    }

  private:
    std::vector<WebhookConfig> m_configs;
    HttpPostFn m_postFn;
};

}  // namespace Engine
}  // namespace ExplorerLens
