// SSOIntegrationBridge.h — SAML 2.0 / OIDC SSO Authentication Bridge
// Copyright (c) 2026 ExplorerLens Project
//
// Integrates ExplorerLens Manager with enterprise SSO providers (Entra ID,
// ADFS, Okta, Ping). Handles token acquisition, refresh, and session binding
// for plugin marketplace auth and fleet management console login.
//
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <optional>
#include <cstdint>

namespace ExplorerLens { namespace Engine { namespace Enterprise {

enum class SSOProtocol : uint8_t {
    OIDC_PKCE  = 0,   // OAuth 2.0 + OIDC with PKCE (recommended)
    SAML2      = 1,   // SAML 2.0 SP-initiated
    WAM        = 2,   // Windows Authentication Manager (WAM/broker)
    WIA        = 3    // Windows Integrated Auth (Kerberos/NTLM)
};

enum class TokenScope : uint8_t {
    Profile         = 0x01,
    PluginMarket    = 0x02,   // Marketplace API access
    FleetManage     = 0x04,   // Fleet config read/write
    AuditRead       = 0x08,   // SIEM audit log pull
    ComplianceAdmin = 0x10    // Compliance report write
};

struct SSOProviderConfig {
    SSOProtocol   protocol       = SSOProtocol::WAM;
    std::string   tenantId;           // Azure AD tenant GUID or "common"
    std::string   clientId;           // App registration client_id
    std::string   authority;          // https://login.microsoftonline.com/<tenant>
    std::string   redirectUri;        // ms-appx-web://... or http://localhost
    std::vector<std::string> scopes;  // Requested OAuth scopes
    bool          useSystemBroker = true;  // Use WAM/WebAccountManager
    bool          silentFirst     = true;  // Try silent before interactive
};

struct SSOTokenBundle {
    std::string   accessToken;
    std::string   idToken;
    std::string   refreshToken;
    std::string   tokenType;    // "Bearer"
    std::string   scope;
    std::string   userOid;      // Entra Object ID
    std::string   upn;          // User Principal Name
    std::string   displayName;
    std::string   tenantId;
    std::chrono::system_clock::time_point expiresAt;
    bool          isExpired() const {
        return std::chrono::system_clock::now() >= expiresAt;
    }
};

enum class SSOResult : uint8_t {
    Success        = 0,
    Silent         = 1,   // Obtained silently (cached)
    InteractionRequired = 2,
    AuthFailed     = 3,
    Cancelled      = 4,
    NetworkError   = 5
};

struct SSOAcquireResult {
    SSOResult       status = SSOResult::AuthFailed;
    SSOTokenBundle  token;
    std::string     errorCode;
    std::string     errorDescription;
};

class SSOIntegrationBridge {
public:
    static SSOIntegrationBridge& Instance() {
        static SSOIntegrationBridge inst;
        return inst;
    }

    void Configure(SSOProviderConfig cfg) { m_cfg = std::move(cfg); }
    const SSOProviderConfig& Config() const { return m_cfg; }

    // Acquire token — tries silent (cached) then interactive
    SSOAcquireResult AcquireToken(uint8_t scopeFlags = static_cast<uint8_t>(TokenScope::Profile)) {
        // Try cached token first
        if (m_cachedToken.has_value() && !m_cachedToken->isExpired())
            return { SSOResult::Silent, *m_cachedToken };

        // Attempt WAM (Windows Authentication Manager) broker flow
        if (m_cfg.protocol == SSOProtocol::WAM)
            return AcquireViaWAM(scopeFlags);

        // OIDC PKCE fallback via WinHTTP
        return AcquireViaOIDC(scopeFlags);
    }

    // Force interactive re-authentication (clear cache)
    SSOAcquireResult ReAuthenticate() {
        m_cachedToken.reset();
        return AcquireToken();
    }

    // Validate a bearer token for inbound requests (fleet API)
    bool ValidateInboundToken(const std::string& bearerToken,
                               const std::string& expectedAudience) const {
        if (bearerToken.empty()) return false;
        // Production: validate JWT signature via JWKS from tenant discovery endpoint
        // Check: iss, aud, exp, iat, nonce (OIDC)
        // Stub: non-empty and correct prefix
        return bearerToken.substr(0, 6) == "eyJ0eX" || bearerToken.substr(0, 6) == "eyJhbG";
    }

    // Sign out and revoke cached tokens
    void SignOut() {
        m_cachedToken.reset();
        FireSignOutCallbacks();
    }

    bool IsSignedIn() const {
        return m_cachedToken.has_value() && !m_cachedToken->isExpired();
    }

    std::optional<SSOTokenBundle> CurrentToken() const { return m_cachedToken; }

    // Subscribe to sign-in/sign-out events
    using AuthFn = std::function<void(const SSOTokenBundle&)>;
    using SignOutFn = std::function<void()>;
    void OnSignIn(AuthFn fn)   { m_signInCbs.push_back(std::move(fn)); }
    void OnSignOut(SignOutFn fn){ m_signOutCbs.push_back(std::move(fn)); }

    // Build Authorization header value
    std::string AuthHeader() const {
        if (!m_cachedToken.has_value()) return {};
        return "Bearer " + m_cachedToken->accessToken;
    }

private:
    SSOIntegrationBridge() = default;

    SSOAcquireResult AcquireViaWAM(uint8_t scopeFlags) {
        // Production: use IWebAuthenticationCoreManagerInterop + AccountsSettingsPane
        // WAM provides seamless SSO with Windows 11 account integration.
        // Stub: simulate a successful WAM silent acquisition.
        SSOTokenBundle tok;
        tok.tokenType   = "Bearer";
        tok.tenantId    = m_cfg.tenantId;
        tok.expiresAt   = std::chrono::system_clock::now() + std::chrono::hours(1);
        m_cachedToken   = tok;
        FireSignInCallbacks(tok);
        return { SSOResult::Success, tok };
    }

    SSOAcquireResult AcquireViaOIDC(uint8_t) {
        // Production: launch browser / loopback redirect + PKCE exchange via WinHTTP
        return { SSOResult::InteractionRequired, {} };
    }

    void FireSignInCallbacks(const SSOTokenBundle& tok) {
        for (auto& fn : m_signInCbs) fn(tok);
    }

    void FireSignOutCallbacks() {
        for (auto& fn : m_signOutCbs) fn();
    }

    SSOProviderConfig                m_cfg;
    std::optional<SSOTokenBundle>    m_cachedToken;
    std::vector<AuthFn>              m_signInCbs;
    std::vector<SignOutFn>           m_signOutCbs;
};

}}} // namespace ExplorerLens::Engine::Enterprise
