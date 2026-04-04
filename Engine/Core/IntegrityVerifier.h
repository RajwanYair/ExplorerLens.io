// IntegrityVerifier.h — HMAC-SHA256 Plugin Signature Verifier
// Copyright (c) 2026 ExplorerLens Project
//
// Verifies plugin DLL authenticity using a chain of trust:
//   1. Authenticode signature via WinVerifyTrust
//   2. Publisher certificate against the ExplorerLens trusted publisher list
//   3. HMAC-SHA256 content hash against the marketplace manifest
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class VerifyStatus : uint8_t {
    OK = 0,
    NotSigned = 1,
    SignatureInvalid = 2,
    CertUntrusted = 3,
    HashMismatch = 4,
    ManifestMissing = 5,
    TimestampExpired = 6,
    RevocationCheckFail = 7,
    Error = 8,
};

struct VerifyReport
{
    VerifyStatus status{VerifyStatus::Error};
    std::wstring subjectName;
    std::wstring issuerName;
    std::wstring thumbprint;   // SHA-1 cert thumbprint (hex)
    std::wstring signingTime;  // RFC3339
    bool isTimestamped{false};
    bool isTrustedPublisher{false};
    std::string errorDetail;
};

struct ExpectedHash
{
    std::string hexSHA256;  // lower-case hex-encoded SHA-256 of the DLL bytes
    std::wstring pluginPath;
};

class IntegrityVerifier
{
  public:
    struct Config
    {
        // Hex SHA-256 thumbprints of trusted signing certs (pinned set).
        std::vector<std::string> trustedThumbprints;
        bool checkRevocation{true};
        bool requireTimestamp{true};
    };

    explicit IntegrityVerifier(Config cfg = {});

    // Full verification: Authenticode + publisher trust + optional hash check.
    [[nodiscard]] VerifyReport Verify(const std::wstring& dllPath, const ExpectedHash* expectedHash = nullptr) const;

    // Fast Authenticode-only check (no hash, no publisher lookup).
    [[nodiscard]] VerifyStatus VerifyAuthenticode(const std::wstring& dllPath) const noexcept;

    // Compute SHA-256 of a file's raw bytes.
    static std::string ComputeSHA256Hex(const std::wstring& path);

    // HMAC-SHA256 of data with key.
    static std::string HMACSha256Hex(const void* data, size_t len, const void* key, size_t keyLen);

  private:
    Config m_cfg;
};

}  // namespace Engine
}  // namespace ExplorerLens
