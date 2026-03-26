// PluginSignatureVerifier.h — PKI Signature Verification for Plugin Packages
// Copyright (c) 2026 ExplorerLens Project
//
// Verifies the RSA-PSS / Authenticode signature of a .lenspkg package against
// the ExplorerLens Plugin CA certificate chain, using Windows CryptVerifyMessage
// or the OpenSSL EVP API for cross-platform builds.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace ExplorerLens {
namespace Engine {

// ---- Trust Policy -----------------------------------------------------------

enum class SignatureTrustPolicy : uint8_t {
    RequireValid     = 0,   // Reject any package with invalid / missing signature
    WarnOnInvalid    = 1,   // Allow but log a WARNING (dev/testing mode)
    AllowUnsigned    = 2,   // Accept unsigned packages (insecure — enterprise only)
};

// ---- Verification Result ----------------------------------------------------

enum class SignatureVerifyStatus {
    Valid              = 0,
    SignatureInvalid   = 1,   // Signature bytes don't match package hash
    CertExpired        = 2,
    CertRevoked        = 3,   // OCSP check failed
    CertUntrusted      = 4,   // Not chained to ExplorerLens Plugin CA
    Unsigned           = 5,   // No signature present in manifest
    HashMismatch       = 6,   // Manifest sha256 doesn't match actual package
    InternalError      = 99,
};

struct SignatureVerifyResult {
    SignatureVerifyStatus status    = SignatureVerifyStatus::InternalError;
    std::string           signerCN;   // CN from the signer's certificate
    std::string           issuerCN;
    std::string           notBefore;  // ISO 8601
    std::string           notAfter;
    bool                  ocspChecked = false;
    std::string           error;
};

// ---- PluginSignatureVerifier ------------------------------------------------

class PluginSignatureVerifier {
public:
    explicit PluginSignatureVerifier(
        SignatureTrustPolicy policy = SignatureTrustPolicy::RequireValid);
    ~PluginSignatureVerifier();

    // Verify the .lenspkg file at the given path.
    SignatureVerifyResult Verify(const std::string& pkgPath) const;

    // Verify raw package bytes (for in-memory verification before disk write).
    SignatureVerifyResult VerifyBytes(
        const std::vector<uint8_t>& pkgBytes,
        const std::string&          signatureB64,
        const std::string&          expectedSHA256) const;

    // Load the bundled ExplorerLens Plugin CA certificate from resources.
    static bool LoadBuiltInCA(std::vector<uint8_t>& outDERCert);

    // Check if OCSP validation is available (requires network).
    static bool OCSPAvailable();

    void SetTrustPolicy(SignatureTrustPolicy policy);
    SignatureTrustPolicy GetTrustPolicy() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
