// LiveSyncTokenManager.cpp — Live-Sync Session Token Lifecycle Manager
// Copyright (c) 2026 ExplorerLens Project
//
#include "LiveSyncTokenManager.h"
#include <chrono>

namespace ExplorerLens { namespace Engine {

uint64_t LiveSyncTokenManager::NowMs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

LiveSyncTokenManager::LiveSyncTokenManager(const Config& cfg)
    : m_config(cfg)
{}

LiveSyncToken LiveSyncTokenManager::Issue(const std::wstring& filePath, uint32_t userId)
{
    auto& fileMap = m_tokens[filePath];
    auto& token   = fileMap[userId];

    token.filePath         = filePath;
    token.version.userId   = userId;
    token.version.counter  = token.version.counter + 1;
    token.version.issuedAt = NowMs();
    token.expiresAt        = (m_config.defaultTtlMs > 0)
                             ? NowMs() + m_config.defaultTtlMs : 0;
    token.valid            = true;
    return token;
}

bool LiveSyncTokenManager::Validate(const LiveSyncToken& token) const
{
    auto fileIt = m_tokens.find(token.filePath);
    if (fileIt == m_tokens.end()) return false;

    auto userIt = fileIt->second.find(token.version.userId);
    if (userIt == fileIt->second.end()) return false;

    const auto& stored = userIt->second;
    if (!stored.valid) return false;
    if (stored.expiresAt > 0 && NowMs() > stored.expiresAt) return false;
    return stored.version.counter == token.version.counter;
}

void LiveSyncTokenManager::Expire(const std::wstring& filePath, uint32_t userId)
{
    auto fileIt = m_tokens.find(filePath);
    if (fileIt == m_tokens.end()) return;
    auto userIt = fileIt->second.find(userId);
    if (userIt != fileIt->second.end())
        userIt->second.valid = false;
}

bool LiveSyncTokenManager::Refresh(const std::wstring& filePath, uint32_t userId)
{
    auto fileIt = m_tokens.find(filePath);
    if (fileIt == m_tokens.end()) return false;
    auto userIt = fileIt->second.find(userId);
    if (userIt == fileIt->second.end()) return false;

    auto& t = userIt->second;
    t.version.counter++;
    t.version.issuedAt = NowMs();
    if (m_config.defaultTtlMs > 0)
        t.expiresAt = NowMs() + m_config.defaultTtlMs;
    t.valid = true;
    return true;
}

uint32_t LiveSyncTokenManager::PurgeExpired(uint64_t currentMs)
{
    uint32_t purged = 0;
    for (auto& [path, userMap] : m_tokens) {
        for (auto it = userMap.begin(); it != userMap.end(); ) {
            if (it->second.expiresAt > 0 && currentMs > it->second.expiresAt) {
                it = userMap.erase(it);
                ++purged;
            } else {
                ++it;
            }
        }
    }
    return purged;
}

uint32_t LiveSyncTokenManager::ActiveTokenCount() const
{
    uint32_t n = 0;
    for (const auto& [path, userMap] : m_tokens)
        for (const auto& [uid, tok] : userMap)
            if (tok.valid) ++n;
    return n;
}

const LiveSyncTokenManager::Config& LiveSyncTokenManager::GetConfig() const
{
    return m_config;
}

}} // namespace ExplorerLens::Engine
