// TelemetryConsentManager.h — GDPR Telemetry Consent and Gating Layer
// Copyright (c) 2026 ExplorerLens Project
//
// Manages user telemetry consent state per EU GDPR / CCPA requirements.
// All telemetry emission in ExplorerLens passes through ConsentGate()
// before being sent. State is persisted in HKCU and honoured by GP.
//
#pragma once

#include <windows.h>
#include <string>
#include <functional>
#include <vector>
#include <cstdint>
#include <optional>
#include <chrono>

namespace ExplorerLens { namespace Engine { namespace Core {

enum class ConsentTelemetryLevel : uint8_t {
    None        = 0,   // No telemetry — crash + security reporting only
    Basic       = 1,   // Anonymized usage counts (no file data)
    Standard    = 2,   // Aggregated perf metrics + error codes
    Full        = 3    // Detailed diagnostics (never includes file content)
};

enum class ConsentStatus : uint8_t {
    Unknown     = 0,   // Not yet shown consent dialog
    Accepted    = 1,
    Declined    = 2,
    PolicyForced= 3    // MDM/GP overrides user choice
};

struct TelemetryConsentRecord {
    ConsentStatus                         status         = ConsentStatus::Unknown;
    ConsentTelemetryLevel                        level          = ConsentTelemetryLevel::Basic;
    std::chrono::system_clock::time_point decidedAt;
    std::string                           clientVersion;
    bool                                  policyManaged  = false;
};

struct ConsentTelemetryEvent {
    std::string  name;           // e.g. "Decode.Complete"
    ConsentTelemetryLevel minLevel;     // Minimum consent level to emit
    std::string  payload;        // JSON key-value pairs (no PII)
};

class TelemetryConsentManager {
public:
    static TelemetryConsentManager& Instance() {
        static TelemetryConsentManager inst;
        return inst;
    }

    // Returns true if the given event is allowed to be emitted
    bool ConsentGate(const ConsentTelemetryEvent& ev) const {
        if (m_record.policyManaged) {
            // MDM/GP forced-off overrides everything
            return m_record.status != ConsentStatus::Declined
                && static_cast<uint8_t>(m_record.level) >= static_cast<uint8_t>(ev.minLevel);
        }
        if (m_record.status != ConsentStatus::Accepted) return false;
        return static_cast<uint8_t>(m_record.level) >= static_cast<uint8_t>(ev.minLevel);
    }

    // Convenience: gate + emit in one call
    void EmitIfAllowed(const ConsentTelemetryEvent& ev) {
        if (!ConsentGate(ev)) return;
        for (auto& fn : m_emitters) fn(ev);
    }

    // Record user consent decision (call from onboarding dialog)
    void RecordConsent(ConsentTelemetryLevel level) {
        m_record.status    = ConsentStatus::Accepted;
        m_record.level     = level;
        m_record.decidedAt = std::chrono::system_clock::now();
        Persist();
        FireChangeCallbacks();
    }

    void RecordDecline() {
        m_record.status    = ConsentStatus::Declined;
        m_record.level     = ConsentTelemetryLevel::None;
        m_record.decidedAt = std::chrono::system_clock::now();
        Persist();
        FireChangeCallbacks();
    }

    // Withdraw previously given consent (GDPR right to withdraw)
    void WithdrawConsent() { RecordDecline(); }

    ConsentStatus        Status()  const { return m_record.status; }
    ConsentTelemetryLevel       Level()   const { return m_record.level; }
    bool                 NeedsConsentDialog() const {
        return m_record.status == ConsentStatus::Unknown && !m_record.policyManaged;
    }

    const TelemetryConsentRecord& Record() const { return m_record; }

    // Register a telemetry emitter (e.g. ETW provider, app insights SDK)
    using EmitFn = std::function<void(const ConsentTelemetryEvent&)>;
    void AddEmitter(EmitFn fn) { m_emitters.push_back(std::move(fn)); }

    // Subscribe to consent change events
    using ChangeFn = std::function<void(ConsentStatus, ConsentTelemetryLevel)>;
    void OnConsentChange(ChangeFn fn) { m_changeCbs.push_back(std::move(fn)); }

    // Export consent record as JSON for compliance evidence
    std::string ToJson() const {
        auto t = std::chrono::system_clock::to_time_t(m_record.decidedAt);
        char ts[32] = {}; struct tm tm_buf;
        gmtime_s(&tm_buf, &t);
        strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", &tm_buf);
        char buf[256];
        snprintf(buf, sizeof(buf),
            "{\"status\":%d,\"level\":%d,\"decidedAt\":\"%s\","
            "\"policyManaged\":%s,\"version\":\"%s\"}",
            static_cast<int>(m_record.status),
            static_cast<int>(m_record.level),
            ts,
            m_record.policyManaged ? "true" : "false",
            m_record.clientVersion.c_str());
        return buf;
    }

private:
    TelemetryConsentManager() {
        m_record.clientVersion = "19.1.0";
        Load();
    }

    static constexpr wchar_t CONSENT_KEY[] = L"SOFTWARE\\ExplorerLens\\Telemetry";

    void Load() {
        // First check GP (policy managed)
        HKEY hkGP = nullptr;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Policies\\ExplorerLens", 0, KEY_READ, &hkGP) == ERROR_SUCCESS) {
            DWORD val = 0, sz = sizeof(DWORD);
            if (RegQueryValueExW(hkGP, L"AllowTelemetry", nullptr, nullptr,
                reinterpret_cast<BYTE*>(&val), &sz) == ERROR_SUCCESS) {
                m_record.policyManaged = true;
                m_record.status = (val != 0) ? ConsentStatus::Accepted : ConsentStatus::Declined;
                m_record.level  = static_cast<ConsentTelemetryLevel>(val <= 3 ? val : 2);
            }
            RegCloseKey(hkGP);
        }
        if (m_record.policyManaged) return;

        // Load user choice from HKCU
        HKEY hk = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, CONSENT_KEY, 0, KEY_READ, &hk) != ERROR_SUCCESS) return;
        DWORD dw = 0, sz = sizeof(DWORD);
        if (RegQueryValueExW(hk, L"ConsentStatus", nullptr, nullptr,
            reinterpret_cast<BYTE*>(&dw), &sz) == ERROR_SUCCESS)
            m_record.status = static_cast<ConsentStatus>(dw);
        if (RegQueryValueExW(hk, L"ConsentTelemetryLevel", nullptr, nullptr,
            reinterpret_cast<BYTE*>(&dw), &sz) == ERROR_SUCCESS)
            m_record.level = static_cast<ConsentTelemetryLevel>(dw);
        RegCloseKey(hk);
    }

    void Persist() {
        if (m_record.policyManaged) return;
        HKEY hk = nullptr;
        RegCreateKeyExW(HKEY_CURRENT_USER, CONSENT_KEY, 0, nullptr,
            REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &hk, nullptr);
        if (!hk) return;
        DWORD s = static_cast<DWORD>(m_record.status);
        DWORD l = static_cast<DWORD>(m_record.level);
        RegSetValueExW(hk, L"ConsentStatus",  0, REG_DWORD, reinterpret_cast<BYTE*>(&s), 4);
        RegSetValueExW(hk, L"ConsentTelemetryLevel", 0, REG_DWORD, reinterpret_cast<BYTE*>(&l), 4);
        RegCloseKey(hk);
    }

    void FireChangeCallbacks() {
        for (auto& fn : m_changeCbs) fn(m_record.status, m_record.level);
    }

    TelemetryConsentRecord      m_record;
    std::vector<EmitFn>         m_emitters;
    std::vector<ChangeFn>       m_changeCbs;
};

}}} // namespace ExplorerLens::Engine::Core
