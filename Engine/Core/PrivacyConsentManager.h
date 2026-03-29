// PrivacyConsentManager.h — Privacy Consent Manager
// Copyright (c) 2026 ExplorerLens Project
//
// Manages granular telemetry opt-in/opt-out consent with audit trail.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>

namespace ExplorerLens { namespace Engine {

enum class ConsentCategory { Usage, Performance, Crash, PersonalizedAI, All };
enum class ConsentState    { Granted, Denied, Pending };

struct ConsentRecord {
    ConsentCategory category    = ConsentCategory::Usage;
    ConsentState    state       = ConsentState::Pending;
    uint64_t        timestampMs = 0;
};

class PrivacyConsentManager {
public:
    void SetConsent(ConsentCategory cat, ConsentState state) {
        auto now = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        ConsentRecord rec{ cat, state, now };
        m_consents[static_cast<int>(cat)] = state;
        m_trail.push_back(rec);
    }
    ConsentState GetConsent(ConsentCategory cat) const {
        auto it = m_consents.find(static_cast<int>(cat));
        return it != m_consents.end() ? it->second : ConsentState::Pending;
    }
    bool IsAllowed(ConsentCategory cat) const {
        return GetConsent(cat) == ConsentState::Granted;
    }
    std::vector<ConsentRecord> AuditTrail() const { return m_trail; }

private:
    std::unordered_map<int, ConsentState> m_consents;
    std::vector<ConsentRecord>            m_trail;
};

}} // namespace ExplorerLens::Engine
