/******************************************************************************
 * DarkThumbs Plugin Isolation Mode Implementation
 * Copyright (c) 2026 - DarkThumbs Project
 *****************************************************************************/

#include "IsolationModeSelector.h"
#include <shlwapi.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <iostream>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "wintrust.lib")

namespace DarkThumbs {

//============================================================================
// IsolationModeSelector Implementation
//============================================================================

IsolationModeSelector::IsolationModeSelector() {
    LoadConfiguration();
}

IsolationModeSelector::~IsolationModeSelector() {
    SaveConfiguration();
}

IsolationMode IsolationModeSelector::DetermineMode(const std::wstring& plugin_id,
                                                   const std::filesystem::path& plugin_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check enterprise policy - denied plugins cannot run
    if (IsPluginDeniedByPolicy(plugin_id)) {
        // Plugin is explicitly denied
        return IsolationMode::PluginHost;  // Still return a mode (will fail loading elsewhere)
    }
    
    // Check enterprise policy - allowed list
    if (!policy_allowed_.empty() && !IsPluginAllowedByPolicy(plugin_id)) {
        // Allowlist is active and plugin is not on it
        return IsolationMode::PluginHost;
    }
    
    // Check explicit trust list
    if (IsTrustedPlugin(plugin_id)) {
        return IsolationMode::InWorker;
    }
    
    // Check signature and verification
    if (IsSignatureVerified(plugin_path)) {
        // Plugin is signed and verified
        
        // Check user preference
        if (UserAllowsInWorker(plugin_id)) {
            // User has explicitly allowed in-worker mode
            
            // Check against minimum policy
            if (minimum_mode_ == IsolationMode::InWorker) {
                return IsolationMode::InWorker;
            }
        }
    }
    
    // Default: separate process for security
    return IsolationMode::PluginHost;
}

bool IsolationModeSelector::IsTrustedPlugin(const std::wstring& plugin_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return trusted_plugins_.find(plugin_id) != trusted_plugins_.end();
}

void IsolationModeSelector::AddTrustedPlugin(const std::wstring& plugin_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    trusted_plugins_.insert(plugin_id);
    SaveConfiguration();
}

void IsolationModeSelector::RemoveTrustedPlugin(const std::wstring& plugin_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    trusted_plugins_.erase(plugin_id);
    SaveConfiguration();
}

bool IsolationModeSelector::IsSignatureValid(const std::filesystem::path& plugin_path) const {
    // Verify Authenticode signature
    WINTRUST_FILE_INFO file_info = {};
    file_info.cbStruct = sizeof(WINTRUST_FILE_INFO);
    file_info.pcwszFilePath = plugin_path.c_str();
    file_info.hFile = nullptr;
    file_info.pgKnownSubject = nullptr;
    
    // Use WINTRUST_ACTION_GENERIC_VERIFY_V2 GUID directly
    GUID policy_guid = { 0xaac56b, 0xcd44, 0x11d0, { 0x8c, 0xc2, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee } };
    
    WINTRUST_DATA trust_data = {};
    trust_data.cbStruct = sizeof(WINTRUST_DATA);
    trust_data.dwUIChoice = WTD_UI_NONE;
    trust_data.fdwRevocationChecks = WTD_REVOKE_NONE;  // Skip revocation check for performance
    trust_data.dwUnionChoice = WTD_CHOICE_FILE;
    trust_data.pFile = &file_info;
    trust_data.dwStateAction = WTD_STATEACTION_VERIFY;
    trust_data.dwProvFlags = WTD_SAFER_FLAG | WTD_CACHE_ONLY_URL_RETRIEVAL;
    
    LONG result = WinVerifyTrust(nullptr, &policy_guid, &trust_data);
    
    // Close trust handle
    trust_data.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust(nullptr, &policy_guid, &trust_data);
    
    return result == ERROR_SUCCESS;
}

bool IsolationModeSelector::IsSignatureVerified(const std::filesystem::path& plugin_path) const {
    // Check signature and also verify publisher is trusted
    if (!IsSignatureValid(plugin_path)) {
        return false;
    }
    
    // Get publisher name
    std::wstring publisher = GetPluginPublisher(plugin_path);
    if (publisher.empty()) {
        return false;
    }
    
    // Check if vendor is trusted
    return IsTrustedVendor(publisher);
}

bool IsolationModeSelector::UserAllowsInWorker(const std::wstring& plugin_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = user_preferences_.find(plugin_id);
    return (it != user_preferences_.end()) && it->second;
}

void IsolationModeSelector::SetUserPreference(const std::wstring& plugin_id, bool allow_in_worker) {
    std::lock_guard<std::mutex> lock(mutex_);
    user_preferences_[plugin_id] = allow_in_worker;
    SaveConfiguration();
}

IsolationMode IsolationModeSelector::GetMinimumIsolationMode() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return minimum_mode_;
}

bool IsolationModeSelector::IsPluginAllowedByPolicy(const std::wstring& plugin_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (policy_allowed_.empty()) {
        return true;  // No allowlist = all allowed
    }
    return policy_allowed_.find(plugin_id) != policy_allowed_.end();
}

bool IsolationModeSelector::IsPluginDeniedByPolicy(const std::wstring& plugin_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return policy_denied_.find(plugin_id) != policy_denied_.end();
}

void IsolationModeSelector::LoadConfiguration() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Load from registry: HKCU\Software\DarkThumbs\Plugins
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\DarkThumbs\\Plugins",
                     0, KEY_READ, &key) != ERROR_SUCCESS) {
        return;
    }
    
    // Load trusted plugins (REG_MULTI_SZ)
    wchar_t buffer[4096];
    DWORD buffer_size = sizeof(buffer);
    DWORD type;
    
    if (RegQueryValueExW(key, L"TrustedPlugins", nullptr, &type,
                        reinterpret_cast<BYTE*>(buffer), &buffer_size) == ERROR_SUCCESS &&
        type == REG_MULTI_SZ) {
        wchar_t* ptr = buffer;
        while (*ptr) {
            trusted_plugins_.insert(ptr);
            ptr += wcslen(ptr) + 1;
        }
    }
    
    // Load user preferences (each plugin has its own DWORD value)
    wchar_t value_name[256];
    DWORD value_name_size = sizeof(value_name) / sizeof(wchar_t);
    DWORD index = 0;
    
    while (RegEnumValueW(key, index++, value_name, &value_name_size,
                        nullptr, &type, nullptr, nullptr) == ERROR_SUCCESS) {
        if (type == REG_DWORD && wcsstr(value_name, L"AllowInWorker_") == value_name) {
            DWORD allow = 0;
            DWORD size = sizeof(allow);
            if (RegQueryValueExW(key, value_name, nullptr, nullptr,
                               reinterpret_cast<BYTE*>(&allow), &size) == ERROR_SUCCESS) {
                std::wstring plugin_id = value_name + 14;  // Skip "AllowInWorker_"
                user_preferences_[plugin_id] = (allow != 0);
            }
        }
        value_name_size = sizeof(value_name) / sizeof(wchar_t);
    }
    
    RegCloseKey(key);
    
    // Load enterprise policy from HKLM
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies\\DarkThumbs\\Plugins",
                     0, KEY_READ, &key) == ERROR_SUCCESS) {
        // Minimum isolation mode
        DWORD min_mode = 1;  // Default: PluginHost
        DWORD size = sizeof(min_mode);
        if (RegQueryValueExW(key, L"MinIsolationMode", nullptr, nullptr,
                           reinterpret_cast<BYTE*>(&min_mode), &size) == ERROR_SUCCESS) {
            minimum_mode_ = (min_mode == 0) ? IsolationMode::InWorker : IsolationMode::PluginHost;
        }
        
        // Policy allowed/denied lists
        buffer_size = sizeof(buffer);
        if (RegQueryValueExW(key, L"AllowedPlugins", nullptr, &type,
                            reinterpret_cast<BYTE*>(buffer), &buffer_size) == ERROR_SUCCESS &&
            type == REG_MULTI_SZ) {
            wchar_t* ptr = buffer;
            while (*ptr) {
                policy_allowed_.insert(ptr);
                ptr += wcslen(ptr) + 1;
            }
        }
        
        buffer_size = sizeof(buffer);
        if (RegQueryValueExW(key, L"DeniedPlugins", nullptr, &type,
                            reinterpret_cast<BYTE*>(buffer), &buffer_size) == ERROR_SUCCESS &&
            type == REG_MULTI_SZ) {
            wchar_t* ptr = buffer;
            while (*ptr) {
                policy_denied_.insert(ptr);
                ptr += wcslen(ptr) + 1;
            }
        }
        
        RegCloseKey(key);
    }
}

void IsolationModeSelector::SaveConfiguration() {
    // Save to registry: HKCU\Software\DarkThumbs\Plugins
    HKEY key;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\DarkThumbs\\Plugins",
                       0, nullptr, 0, KEY_WRITE, nullptr, &key, nullptr) != ERROR_SUCCESS) {
        return;
    }
    
    // Save trusted plugins (REG_MULTI_SZ)
    std::wstring multi_sz;
    for (const auto& plugin_id : trusted_plugins_) {
        multi_sz += plugin_id;
        multi_sz += L'\0';
    }
    multi_sz += L'\0';
    
    RegSetValueExW(key, L"TrustedPlugins", 0, REG_MULTI_SZ,
                  reinterpret_cast<const BYTE*>(multi_sz.c_str()),
                  static_cast<DWORD>(multi_sz.size() * sizeof(wchar_t)));
    
    // Save user preferences
    for (const auto& pair : user_preferences_) {
        std::wstring value_name = L"AllowInWorker_" + pair.first;
        DWORD allow = pair.second ? 1 : 0;
        RegSetValueExW(key, value_name.c_str(), 0, REG_DWORD,
                      reinterpret_cast<const BYTE*>(&allow), sizeof(allow));
    }
    
    RegCloseKey(key);
}

//============================================================================
// Helper Functions
//============================================================================

bool IsTrustedVendor(const std::wstring& vendor) {
    // List of trusted vendors (simplified)
    static const std::unordered_set<std::wstring> trusted_vendors = {
        L"Microsoft Corporation",
        L"Adobe Systems Incorporated",
        L"Google LLC",
        L"Apple Inc.",
        L"DarkThumbs Project",  // Our own plugins
    };
    
    return trusted_vendors.find(vendor) != trusted_vendors.end();
}

std::wstring GetPluginPublisher(const std::filesystem::path& plugin_path) {
    // Get certificate information from signed executable
    HCERTSTORE cert_store = nullptr;
    HCRYPTMSG crypt_msg = nullptr;
    DWORD encoding;
    DWORD content_type;
    DWORD format_type;
    
    if (!CryptQueryObject(
            CERT_QUERY_OBJECT_FILE,
            plugin_path.c_str(),
            CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
            CERT_QUERY_FORMAT_FLAG_BINARY,
            0,
            &encoding,
            &content_type,
            &format_type,
            &cert_store,
            &crypt_msg,
            nullptr)) {
        return L"";
    }
    
    // Get signer certificate
    DWORD signer_info_size = 0;
    CryptMsgGetParam(crypt_msg, CMSG_SIGNER_INFO_PARAM, 0, nullptr, &signer_info_size);
    
    if (signer_info_size == 0) {
        if (cert_store) CertCloseStore(cert_store, 0);
        if (crypt_msg) CryptMsgClose(crypt_msg);
        return L"";
    }
    
    std::vector<BYTE> signer_info_buffer(signer_info_size);
    CMSG_SIGNER_INFO* signer_info = reinterpret_cast<CMSG_SIGNER_INFO*>(signer_info_buffer.data());
    
    if (!CryptMsgGetParam(crypt_msg, CMSG_SIGNER_INFO_PARAM, 0, signer_info, &signer_info_size)) {
        if (cert_store) CertCloseStore(cert_store, 0);
        if (crypt_msg) CryptMsgClose(crypt_msg);
        return L"";
    }
    
    // Find certificate in store
    CERT_INFO cert_info = {};
    cert_info.Issuer = signer_info->Issuer;
    cert_info.SerialNumber = signer_info->SerialNumber;
    
    PCCERT_CONTEXT cert_context = CertFindCertificateInStore(
        cert_store,
        encoding,
        0,
        CERT_FIND_SUBJECT_CERT,
        &cert_info,
        nullptr);
    
    std::wstring publisher;
    
    if (cert_context) {
        // Get subject name
        DWORD name_size = CertGetNameStringW(
            cert_context,
            CERT_NAME_SIMPLE_DISPLAY_TYPE,
            0,
            nullptr,
            nullptr,
            0);
        
        if (name_size > 0) {
            std::vector<wchar_t> name_buffer(name_size);
            CertGetNameStringW(
                cert_context,
                CERT_NAME_SIMPLE_DISPLAY_TYPE,
                0,
                nullptr,
                name_buffer.data(),
                name_size);
            publisher = name_buffer.data();
        }
        
        CertFreeCertificateContext(cert_context);
    }
    
    if (cert_store) CertCloseStore(cert_store, 0);
    if (crypt_msg) CryptMsgClose(crypt_msg);
    
    return publisher;
}

} // namespace DarkThumbs
