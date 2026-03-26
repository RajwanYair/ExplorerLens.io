// FirstRunExperience.h — Out-of-Box Experience Wizard and Onboarding Flow
// Copyright (c) 2026 ExplorerLens Project
//
// Detects first-run installation state and presents a guided OOBE wizard for
// shell registration, license entry, and initial configuration.
//
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class OOBEStep {
    Welcome,
    LicenseAgreement,
    RegistrationChoice, // Trial / Enter Key / Community
    ShellRegistration,
    ThumbnailCacheSetup,
    PrivacyConsent,
    Complete
};

struct OOBEState {
    OOBEStep    currentStep  = OOBEStep::Welcome;
    bool        licenseAccepted  = false;
    bool        privacyAccepted  = false;
    bool        shellRegistered  = false;
    bool        trialStarted     = false;
    std::wstring licenseKey;
    int         stepIndex    = 0;
    int         totalSteps   = 6;
};

// Step descriptor used to drive the wizard UI
struct OOBEStepInfo {
    OOBEStep      step;
    const wchar_t* title;
    const wchar_t* description;
    bool           canGoBack;
    bool           canSkip;
};

static const OOBEStepInfo k_OOBESteps[] = {
    { OOBEStep::Welcome,            L"Welcome to ExplorerLens",  L"GPU-accelerated thumbnails for 200+ formats.", false, false },
    { OOBEStep::LicenseAgreement,   L"License Agreement",         L"Review and accept the End User License Agreement.", false, false },
    { OOBEStep::RegistrationChoice, L"Choose How to Start",       L"Start a 30-day Pro trial, enter a license key, or use the Community edition.", true, false },
    { OOBEStep::ShellRegistration,  L"Register Shell Extension",  L"Register ExplorerLens with Windows Explorer.", true, true },
    { OOBEStep::ThumbnailCacheSetup, L"Configure Thumbnail Cache", L"Set cache location and maximum size.", true, true },
    { OOBEStep::PrivacyConsent,     L"Privacy Settings",          L"Choose what anonymous usage data to share.", true, false },
    { OOBEStep::Complete,           L"Setup Complete",            L"ExplorerLens is ready.  Restart Explorer to see thumbnails.", false, false }
};

class FirstRunExperience {
public:
    // Returns true if this is the first run (flag not set in registry)
    static bool IsFirstRun() {
        HKEY hk = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\ExplorerLens",
                0, KEY_READ, &hk) != ERROR_SUCCESS)
            return true;
        DWORD val = 0, sz = sizeof(val);
        bool hasFlag = (RegQueryValueExW(hk, L"SetupComplete", nullptr, nullptr,
                reinterpret_cast<BYTE*>(&val), &sz) == ERROR_SUCCESS) && val;
        RegCloseKey(hk);
        return !hasFlag;
    }

    // Mark setup as complete so wizard doesn't reappear
    static void MarkComplete() {
        HKEY hk = nullptr;
        RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\ExplorerLens",
                0, nullptr, 0, KEY_WRITE, nullptr, &hk, nullptr);
        if (!hk) return;
        DWORD one = 1;
        RegSetValueExW(hk, L"SetupComplete", 0, REG_DWORD,
                reinterpret_cast<const BYTE*>(&one), sizeof(DWORD));
        RegCloseKey(hk);
    }

    // Reset OOBE (for testing / reinstall)
    static void ResetOOBE() {
        HKEY hk = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\ExplorerLens",
                0, KEY_WRITE, &hk) != ERROR_SUCCESS) return;
        RegDeleteValueW(hk, L"SetupComplete");
        RegCloseKey(hk);
    }

    // --- Wizard state machine ---
    void Begin() {
        m_state = OOBEState{};
        m_state.currentStep = OOBEStep::Welcome;
        NotifyStep();
    }

    bool Advance(bool accepted = true) {
        switch (m_state.currentStep) {
        case OOBEStep::Welcome:
            m_state.currentStep = OOBEStep::LicenseAgreement;
            break;
        case OOBEStep::LicenseAgreement:
            if (!accepted) return false; // Must accept
            m_state.licenseAccepted = true;
            m_state.currentStep = OOBEStep::RegistrationChoice;
            break;
        case OOBEStep::RegistrationChoice:
            m_state.currentStep = OOBEStep::ShellRegistration;
            break;
        case OOBEStep::ShellRegistration:
            m_state.shellRegistered = accepted;
            m_state.currentStep = OOBEStep::ThumbnailCacheSetup;
            break;
        case OOBEStep::ThumbnailCacheSetup:
            m_state.currentStep = OOBEStep::PrivacyConsent;
            break;
        case OOBEStep::PrivacyConsent:
            m_state.privacyAccepted = accepted;
            m_state.currentStep = OOBEStep::Complete;
            break;
        case OOBEStep::Complete:
            MarkComplete();
            if (m_onComplete) m_onComplete(m_state);
            return true;
        }
        m_state.stepIndex++;
        NotifyStep();
        return true;
    }

    bool GoBack() {
        if (m_state.stepIndex == 0) return false;
        m_state.stepIndex--;
        // Reverse enum
        int e = static_cast<int>(m_state.currentStep);
        m_state.currentStep = static_cast<OOBEStep>(e - 1);
        NotifyStep();
        return true;
    }

    const OOBEState& State() const { return m_state; }

    const OOBEStepInfo& CurrentStepInfo() const {
        return k_OOBESteps[m_state.stepIndex];
    }

    // Set trial / license key from RegistrationChoice step
    void SetLicenseKey(const std::wstring& key) { m_state.licenseKey = key; }
    void SetTrialChosen(bool t) { m_state.trialStarted = t; }

    void OnStep(std::function<void(const OOBEState&, const OOBEStepInfo&)> cb) {
        m_onStep = std::move(cb);
    }
    void OnComplete(std::function<void(const OOBEState&)> cb) {
        m_onComplete = std::move(cb);
    }

private:
    void NotifyStep() {
        if (m_onStep) m_onStep(m_state, CurrentStepInfo());
    }

    OOBEState m_state;
    std::function<void(const OOBEState&, const OOBEStepInfo&)> m_onStep;
    std::function<void(const OOBEState&)>                      m_onComplete;
};

}} // namespace ExplorerLens::Engine
