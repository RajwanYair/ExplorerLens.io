/******************************************************************************
 * ExplorerLens Plugin Isolation Mode
 * Copyright (c) 2026 - ExplorerLens Project
 * 
 * Determines whether plugins should run in-process or in separate PluginHost.
 *****************************************************************************/

#pragma once

#include <Windows.h>
#include <filesystem>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace ExplorerLens {

//============================================================================
// Isolation Mode
//============================================================================

enum class IsolationMode {
    InWorker,   // Load DLL directly (trusted plugins only)
    PluginHost  // Separate PluginHost.exe process (default, secure)
};

//============================================================================
// Isolation Mode Selector
//============================================================================

class IsolationModeSelector
{
  public:
    static IsolationModeSelector& Instance()
    {
        static IsolationModeSelector instance;
        return instance;
    }

    // Determine isolation mode for a plugin
    IsolationMode DetermineMode(const std::wstring& plugin_id, const std::filesystem::path& plugin_path);

    // Explicit trust management
    bool IsTrustedPlugin(const std::wstring& plugin_id) const;
    void AddTrustedPlugin(const std::wstring& plugin_id);
    void RemoveTrustedPlugin(const std::wstring& plugin_id);

    // Signature verification
    bool IsSignatureValid(const std::filesystem::path& plugin_path) const;
    bool IsSignatureVerified(const std::filesystem::path& plugin_path) const;

    // User preference
    bool UserAllowsInWorker(const std::wstring& plugin_id) const;
    void SetUserPreference(const std::wstring& plugin_id, bool allow_in_worker);

    // Enterprise policy
    IsolationMode GetMinimumIsolationMode() const;
    bool IsPluginAllowedByPolicy(const std::wstring& plugin_id) const;
    bool IsPluginDeniedByPolicy(const std::wstring& plugin_id) const;

    // Load/Save configuration
    void LoadConfiguration();
    void SaveConfiguration();

  private:
    IsolationModeSelector();
    ~IsolationModeSelector();

    // Trusted plugin list
    std::unordered_set<std::wstring> trusted_plugins_;

    // User preferences (plugin_id -> allow in-worker)
    std::unordered_map<std::wstring, bool> user_preferences_;

    // Policy settings
    IsolationMode minimum_mode_ = IsolationMode::PluginHost;
    std::unordered_set<std::wstring> policy_allowed_;
    std::unordered_set<std::wstring> policy_denied_;

    // Thread safety
    mutable std::mutex mutex_;
};

//============================================================================
// Helper Functions
//============================================================================

// Get isolation mode name
inline const wchar_t* GetIsolationModeName(IsolationMode mode)
{
    switch (mode) {
        case IsolationMode::InWorker:
            return L"In-Worker (Direct)";
        case IsolationMode::PluginHost:
            return L"PluginHost (Sandboxed)";
        default:
            return L"Unknown";
    }
}

// Check if plugin is from trusted vendor (Microsoft, Adobe, etc.)
bool IsTrustedVendor(const std::wstring& vendor);

// Get plugin signature publisher name
std::wstring GetPluginPublisher(const std::filesystem::path& plugin_path);

}  // namespace ExplorerLens
