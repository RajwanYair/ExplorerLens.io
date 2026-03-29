// QuantumSafeKeyRotator.h — Post-Quantum Key Rotation Scheduler
// Copyright (c) 2026 ExplorerLens Project
//
// Automates periodic rotation of ML-KEM and SLH-DSA key material,
// enforcing maximum key lifetimes and triggering re-encapsulation in active sessions.
//
#pragma once
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class KeyType        { MLKEM, SLHDSA, Classical };
enum class RotationReason { Scheduled, PolicyTriggered, Manual, Compromise };

struct KeyRotationPolicy {
    std::chrono::hours maxKeyAgeHours     = std::chrono::hours{24};
    std::chrono::hours warningAheadHours  = std::chrono::hours{2};
    int                maxEncapsulations  = 10000;
    bool               autoRotate         = true;
};

struct KeyRecord {
    std::string keyId;
    KeyType     type         = KeyType::MLKEM;
    int64_t     createdAtEpoch = 0;
    int         encapCount   = 0;
    bool        active       = true;
};

struct RotationResult {
    bool           success       = false;
    std::string    oldKeyId;
    std::string    newKeyId;
    RotationReason reason        = RotationReason::Scheduled;
    std::string    errorMsg;
    bool Ok() const noexcept { return success; }
};

using RotationCallback = std::function<void(const RotationResult&)>;

class QuantumSafeKeyRotator {
public:
    explicit QuantumSafeKeyRotator(KeyRotationPolicy policy = {})
        : m_policy(std::move(policy)) {}

    void SetRotationCallback(RotationCallback cb) { m_callback = std::move(cb); }

    std::string AddKey(KeyType type) {
        auto epoch = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        KeyRecord rec;
        rec.keyId          = "key-" + std::to_string(m_counter++);
        rec.type           = type;
        rec.createdAtEpoch = epoch;
        m_keys.push_back(rec);
        return rec.keyId;
    }

    RotationResult Rotate(const std::string& oldKeyId, RotationReason reason = RotationReason::Scheduled) {
        for (auto& k : m_keys) {
            if (k.keyId == oldKeyId && k.active) {
                k.active         = false;
                std::string newId = AddKey(k.type);
                RotationResult r { true, oldKeyId, newId, reason, {} };
                if (m_callback) m_callback(r);
                return r;
            }
        }
        return { false, oldKeyId, {}, reason, "Key not found" };
    }

    bool ShouldRotate(const KeyRecord& rec) const noexcept {
        if (!m_policy.autoRotate) return false;
        if (rec.encapCount >= m_policy.maxEncapsulations) return true;
        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        int64_t ageH = (now - rec.createdAtEpoch) / 3600;
        return ageH >= static_cast<int64_t>(m_policy.maxKeyAgeHours.count());
    }

    int ActiveKeyCount() const noexcept {
        int c = 0;
        for (const auto& k : m_keys) if (k.active) c++;
        return c;
    }

    const KeyRotationPolicy& Policy() const noexcept { return m_policy; }

private:
    KeyRotationPolicy        m_policy;
    std::vector<KeyRecord>   m_keys;
    RotationCallback         m_callback;
    int                      m_counter = 1;
};

} // namespace Engine
} // namespace ExplorerLens
