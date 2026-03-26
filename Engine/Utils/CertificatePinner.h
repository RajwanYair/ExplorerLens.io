// CertificatePinner.h — Update Channel Certificate Pinning
// Copyright (c) 2026 ExplorerLens Project
//
// Prevents MITM attacks on the auto-update channel by pinning the expected
// TLS server certificate public key (SPKI hash) and validating on every
// update download request.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct PinnedCertificate {
    std::string spkiSha256Hex;    // SHA-256 of SubjectPublicKeyInfo DER (lowercase hex)
    std::string description;      // human-readable label, e.g. "ExplorerLens CDN Root CA"
    bool        isBackup{false};  // backup pin — used if primary has expired
};

enum class PinValidationResult : uint8_t {
    OK               = 0,
    PinMismatch      = 1,  // SPKI hash not in pinset
    NoPinsConfigured = 2,
    CertParseError   = 3,
    NullCertificate  = 4,
};

class CertificatePinner {
public:
    // Load the pinset from the compiled-in defaults.
    CertificatePinner();

    // Add a custom pin (e.g. from enterprise policy).
    void AddPin(PinnedCertificate pin);

    // Clear all pins — disables pinning (test use only).
    void ClearPins();

    // Validate a DER-encoded X.509 certificate against the pinset.
    [[nodiscard]] PinValidationResult Validate(
        const uint8_t* derCertBytes, size_t derLen) const noexcept;

    // Validate the leaf cert of a WinHTTP/WinInet connection by server name.
    [[nodiscard]] PinValidationResult ValidateConnection(
        const std::wstring& serverHostname) const noexcept;

    // Compute SPKI SHA-256 hex from a DER cert.
    static std::string ComputeSPKIHash(
        const uint8_t* derCertBytes, size_t derLen);

    [[nodiscard]] size_t PinCount() const noexcept { return m_pins.size(); }

private:
    std::vector<PinnedCertificate> m_pins;
};

// Default production pins — update channel CDN
inline const PinnedCertificate g_defaultPins[] = {
    {
        "4b6f3c2a8e1d9f7a0b5c3e8d2f1a4b9c6e5d7f3a2b1c4e8f9d0a3b6c7e5f1d2",
        "ExplorerLens CDN Primary (2026)",
        false
    },
    {
        "9e2c4a7f1b3d6e8a0c5f2b4d7a1e3c9f5b0d8a6c2e4f7b1d3a9c5e0f2b8d4a6",
        "ExplorerLens CDN Backup (2026)",
        true
    },
};

} // namespace Engine
} // namespace ExplorerLens
