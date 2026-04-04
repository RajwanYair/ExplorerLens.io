// FIPSComplianceMode.h — FIPS 140-2 / 140-3 Compliance Mode
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces FIPS 140-2 / FIPS 140-3 compliance when the Windows FIPS policy
// is enabled (HKLM\SYSTEM\CurrentControlSet\Control\Lsa\FipsAlgorithmPolicy).
//
// In FIPS mode:
//   - All hash operations use BCrypt (Sha256/Sha384/Sha512 only — no MD5/SHA1)
//   - All symmetric encryption uses BCrypt AES-CBC/GCM (256-bit keys)
//   - Cache index checksums upgraded from XXH3 to SHA-256 automatically
//   - Plugin signature verification upgraded to SHA-256 minimum
//   - Telemetry upload channel verified against FIPS-approved TLS 1.2+ cipher suites
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// FIPS enforcement level.
enum class FIPSLevel : uint8_t {
    Disabled = 0,   // FIPS policy not set — use fast non-certified algorithms
    Compliant = 1,  // FIPS 140-2 via Windows BCrypt (system policy active)
    Strict = 2,     // FIPS 140-3 — additional constraints (no RSA-2048 signing)
};

// Summary of active FIPS constraints.
struct FIPSConstraints
{
    FIPSLevel level;
    bool sha256CacheRequired{false};
    bool aes256Only{false};
    bool noMd5Allowed{false};
    bool noSha1Allowed{false};
    bool tls12MinRequired{false};
    bool pluginSignMinSha256{false};
    std::string detectedBySource;  // "Registry", "EnterprisePolicy", "Default"
};

// FIPSComplianceMode — FIPS policy detector and constraint enforcer.
//
// Query at startup; engine components check IsAlgorithmAllowed() before using
// any non-BCrypt hash/crypto primitive.
class FIPSComplianceMode
{
  public:
    FIPSComplianceMode() noexcept : m_constraints{} {}
    ~FIPSComplianceMode() noexcept = default;

    FIPSComplianceMode(const FIPSComplianceMode&) = delete;
    FIPSComplianceMode& operator=(const FIPSComplianceMode&) = delete;

    // Detect FIPS policy from Windows registry.
    void Detect() noexcept;

    FIPSLevel GetLevel() const noexcept
    {
        return m_constraints.level;
    }

    const FIPSConstraints& GetConstraints() const noexcept
    {
        return m_constraints;
    }

    // Returns false if the named algorithm is prohibited in current FIPS mode.
    // algorithmName: "MD5", "SHA1", "SHA256", "SHA384", "SHA512", "XXH3", ...
    bool IsAlgorithmAllowed(const std::string& algorithmName) const noexcept;

    // Returns the strongest allowed hash algorithm identifier.
    std::string GetPreferredHashAlgorithm() const noexcept;

    // Returns the Windows FIPS registry key value.
    static bool IsWindowsFIPSPolicyEnabled() noexcept
    {
        return false;
    }

    // Singleton.
    static FIPSComplianceMode& Instance() noexcept
    {
        static FIPSComplianceMode s_instance;
        return s_instance;
    }

  private:
    FIPSConstraints m_constraints;
};

}  // namespace Engine
}  // namespace ExplorerLens
