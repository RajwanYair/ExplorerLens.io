// LiveSyncTokenManager.h — Live-Sync Session Token Lifecycle Manager
// Copyright (c) 2026 ExplorerLens Project
//
// Issues, refreshes, validates, and expires per-user per-file edit-session tokens
// used by the collaborative thumbnailing pipeline.  Each token carries an opaque
// version vector so cache coordinators can detect conflicts without exposing file
// content.
//
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <functional>

namespace ExplorerLens { namespace Engine {

/// Opaque version vector for a live-sync session (monotonic counter per file).
struct SyncVersionVector {
    uint64_t counter  = 0;    ///< Monotonically increasing edit count
    uint64_t issuedAt = 0;    ///< Wall clock ms when the token was issued
    uint32_t userId   = 0;    ///< Opaque user identifier (CRC of user principal)
};

/// A live-sync session token for one user editing one file.
struct LiveSyncToken {
    std::wstring         filePath;
    SyncVersionVector    version;
    uint64_t             expiresAt = 0;   ///< Wall clock ms; 0 = no expiry (test only)
    bool                 valid     = true;
};

/// Token callback fired when a token is issued or expired.
using TokenCallback = std::function<void(const LiveSyncToken&)>;

/// Manages live-sync session tokens.
class LiveSyncTokenManager {
public:
    struct Config {
        uint64_t defaultTtlMs   = 30'000;  ///< 30-second default TTL
        uint32_t maxTokensPerFile = 64;    ///< Cap for a single hotly-contested file
    };

    explicit LiveSyncTokenManager(const Config& cfg = {});

    /// Issue a new token for (filePath, userId).  Bumps version counter.
    LiveSyncToken Issue(const std::wstring& filePath, uint32_t userId);

    /// Validate a previously issued token; returns false if expired or not found.
    bool Validate(const LiveSyncToken& token) const;

    /// Explicitly expire a token (user closed the file).
    void Expire(const std::wstring& filePath, uint32_t userId);

    /// Refresh TTL of an existing token; bumps version counter.
    bool Refresh(const std::wstring& filePath, uint32_t userId);

    /// Purge all tokens whose expiresAt < currentMs.
    uint32_t PurgeExpired(uint64_t currentMs);

    uint32_t ActiveTokenCount() const;
    const Config& GetConfig() const;

private:
    Config m_config;
    std::unordered_map<std::wstring, std::unordered_map<uint32_t, LiveSyncToken>> m_tokens;

    static uint64_t NowMs();
};

}} // namespace ExplorerLens::Engine
