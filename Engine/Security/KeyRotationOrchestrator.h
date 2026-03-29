// KeyRotationOrchestrator.h — Key Rotation Orchestrator
// Copyright (c) 2026 ExplorerLens Project
//
// Manages automated cryptographic key rotation schedules, tracks key
// lifecycles, and coordinates with HSM/TPM backends for seamless rotation.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class KeyRotationState { Active, PendingRotation, Rotated, Revoked };

struct KeyRecord {
    std::string       keyId;
    std::string       algorithm;
    int64_t           createdAt     = 0;
    int64_t           expiresAt     = 0;
    KeyRotationState  state         = KeyRotationState::Active;
    uint32_t          rotationCount = 0;
};

struct RotationResult {
    bool        success    = false;
    std::string oldKeyId;
    std::string newKeyId;
    std::string errorMsg;
};

using RotationCallback = std::function<void(const RotationResult&)>;

class KeyRotationOrchestrator {
public:
    KeyRotationOrchestrator() = default;

    bool Initialize(uint32_t rotationIntervalDays = 90) {
        m_intervalDays = rotationIntervalDays;
        m_ready        = true;
        return true;
    }
    bool IsReady() const { return m_ready; }

    std::string RegisterKey(const std::string& algorithm, int64_t nowMs = 0) {
        KeyRecord rec;
        rec.keyId     = "key-" + std::to_string(m_nextId++);
        rec.algorithm = algorithm;
        rec.createdAt = nowMs;
        rec.expiresAt = nowMs + static_cast<int64_t>(m_intervalDays) * 86400000LL;
        rec.state     = KeyRotationState::Active;
        m_keys.push_back(rec);
        return rec.keyId;
    }

    bool IsExpired(const std::string& keyId, int64_t nowMs) const {
        for (const auto& k : m_keys) {
            if (k.keyId == keyId) return k.expiresAt > 0 && nowMs > k.expiresAt;
        }
        return false;
    }

    RotationResult RotateKey(const std::string& keyId, int64_t nowMs = 0) {
        RotationResult res;
        for (auto& k : m_keys) {
            if (k.keyId != keyId) continue;
            std::string newId = RegisterKey(k.algorithm, nowMs);
            k.state = KeyRotationState::Rotated;
            ++k.rotationCount;
            res.success  = true;
            res.oldKeyId = keyId;
            res.newKeyId = newId;
            if (m_callback) m_callback(res);
            return res;
        }
        res.errorMsg = "key_not_found";
        return res;
    }

    void SetRotationCallback(RotationCallback cb) { m_callback = std::move(cb); }

    std::vector<KeyRecord> GetKeys() const { return m_keys; }

    void Shutdown() { m_ready = false; }

private:
    bool                   m_ready       = false;
    uint32_t               m_intervalDays = 90;
    uint32_t               m_nextId       = 1;
    std::vector<KeyRecord> m_keys;
    RotationCallback       m_callback;
};

}} // namespace ExplorerLens::Engine
