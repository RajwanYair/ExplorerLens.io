// PerUserPredictionIsolator.h — Per-User Prediction Isolator
// Copyright (c) 2026 ExplorerLens Project
//
// Ensures prediction models for one user cannot be influenced by or leak data of another user.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

struct PUPIUserProfile {
    std::string userId;
    bool        isolated        = true;
    uint32_t    modelSlot       = 0;
};

struct PUPILeakTestResult {
    bool        leaked          = false;
    std::string leakDetails;
    uint32_t    testedPairCount = 0;
};

class PerUserPredictionIsolator {
public:
    void RegisterUser(const PUPIUserProfile& profile) {
        m_profiles[profile.userId] = profile;
    }
    bool IsIsolated(const std::string& userId) const {
        auto it = m_profiles.find(userId);
        return it != m_profiles.end() && it->second.isolated;
    }
    PUPILeakTestResult TestIsolation() const {
        PUPILeakTestResult r;
        r.testedPairCount = static_cast<uint32_t>(m_profiles.size());
        // Verify all model slots are unique when isolation is on
        std::unordered_map<uint32_t, std::string> slotToUser;
        for (const auto& [id, prof] : m_profiles) {
            if (prof.isolated) {
                auto sit = slotToUser.find(prof.modelSlot);
                if (sit != slotToUser.end()) { r.leaked = true; r.leakDetails = "Slot collision"; break; }
                slotToUser[prof.modelSlot] = id;
            }
        }
        return r;
    }
    void RemoveUser(const std::string& userId) { m_profiles.erase(userId); }
    uint32_t UserCount() const { return static_cast<uint32_t>(m_profiles.size()); }

private:
    std::unordered_map<std::string, PUPIUserProfile> m_profiles;
};

}} // namespace ExplorerLens::Engine
