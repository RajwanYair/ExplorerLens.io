// NetworkTrustManager.h — Certificate Pinning and HTTPS Trust Enforcement
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces certificate pinning for all ExplorerLens backend connections
// (activation, telemetry, feedback) and validates TLS chains against a
// set of pinned Subject Public Key Info (SPKI) SHA-256 hashes.
//
#pragma once
#include <wincrypt.h>
#include <windows.h>
#include <winhttp.h>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "crypt32.lib")

namespace ExplorerLens {
namespace Engine {

struct PinnedEndpoint
{
    std::wstring hostname;                    // e.g. L"activate.explorerlens.io"
    std::vector<std::string> spkiSHA256Pins;  // Base64-encoded SPKI SHA-256
    int maxAgeDays = 30;                      // Max time between pin refresh
    bool requireHSTS = true;
};

enum class TrustCheckResult {
    Trusted,           // Pin matched
    PinMismatch,       // Certificate SPKI doesn't match any pin
    NoPinConfigured,   // Endpoint has no pin — falls through to system trust
    TLSError,          // TLS handshake failed
    HostnameMismatch,  // CN/SAN mismatch
    Revoked
};

struct TrustVerification
{
    std::wstring hostname;
    TrustCheckResult result = TrustCheckResult::TLSError;
    std::string certThumbprint;
    std::string matchedPin;
    bool ok() const
    {
        return result == TrustCheckResult::Trusted || result == TrustCheckResult::NoPinConfigured;
    }
};

class NetworkTrustManager
{
  public:
    static NetworkTrustManager& Get()
    {
        static NetworkTrustManager s_inst;
        return s_inst;
    }

    void AddPin(PinnedEndpoint ep)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_pins[ep.hostname] = std::move(ep);
    }

    // Call from WinHTTP status callback with WINHTTP_CALLBACK_STATUS_CERTIFICATE_ERROR
    TrustVerification Verify(const std::wstring& hostname, PCCERT_CONTEXT certCtx) const
    {
        TrustVerification tv;
        tv.hostname = hostname;

        std::lock_guard<std::mutex> lk(m_mtx);
        auto it = m_pins.find(hostname);
        if (it == m_pins.end()) {
            // No pin configured — rely on system certificate store
            tv.result = TrustCheckResult::NoPinConfigured;
            return tv;
        }

        const PinnedEndpoint& pin = it->second;
        if (!certCtx) {
            tv.result = TrustCheckResult::TLSError;
            return tv;
        }

        // Extract SPKI SHA-256 from the certificate
        std::string spki = ExtractSPKIHash(certCtx);
        tv.certThumbprint = spki;

        for (const auto& p : pin.spkiSHA256Pins) {
            if (p == spki) {
                tv.result = TrustCheckResult::Trusted;
                tv.matchedPin = p;
                return tv;
            }
        }
        tv.result = TrustCheckResult::PinMismatch;
        return tv;
    }

    // Register default ExplorerLens backend pins
    void RegisterDefaults()
    {
        // Production pins — update when certificates rotate
        AddPin({L"activate.explorerlens.io",
                {"47DEQpj8HBSa+/TImW+5JCeuQeRkm5NMpJWZG3hSuFU="},  // placeholder
                90,
                true});
        AddPin({L"feedback.explorerlens.io", {"47DEQpj8HBSa+/TImW+5JCeuQeRkm5NMpJWZG3hSuFU="}, 90, true});
        AddPin({L"telemetry.explorerlens.io", {"47DEQpj8HBSa+/TImW+5JCeuQeRkm5NMpJWZG3hSuFU="}, 90, true});
    }

    bool HasPin(const std::wstring& hostname) const
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_pins.find(hostname) != m_pins.end();
    }

    std::vector<std::wstring> PinnedHosts() const
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        std::vector<std::wstring> hosts;
        for (const auto& [h, _] : m_pins)
            hosts.push_back(h);
        return hosts;
    }

  private:
    NetworkTrustManager()
    {
        RegisterDefaults();
    }

    static std::string ExtractSPKIHash(PCCERT_CONTEXT ctx)
    {
        if (!ctx)
            return {};
        // Hash the SubjectPublicKeyInfo DER blob
        PCERT_INFO info = ctx->pCertInfo;
        // Encode SubjectPublicKeyInfo
        DWORD sz = 0;
        CryptEncodeObjectEx(X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO, &info->SubjectPublicKeyInfo, 0, nullptr, nullptr,
                            &sz);
        if (!sz || sz > 16384)
            return {};

        std::vector<uint8_t> der(sz);
        if (!CryptEncodeObjectEx(X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO, &info->SubjectPublicKeyInfo, 0, nullptr,
                                 der.data(), &sz))
            return {};

        HCRYPTPROV hProv = 0;
        HCRYPTHASH hHash = 0;
        CryptAcquireContextW(&hProv, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
        CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash);
        CryptHashData(hHash, der.data(), static_cast<DWORD>(der.size()), 0);
        uint8_t hash[32] = {};
        DWORD hashSz = 32;
        CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashSz, 0);
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);

        // Base64-encode
        DWORD b64Sz = 0;
        CryptBinaryToStringA(hash, 32, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &b64Sz);
        std::string b64(b64Sz, '\0');
        CryptBinaryToStringA(hash, 32, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, &b64[0], &b64Sz);
        if (!b64.empty() && b64.back() == '\0')
            b64.pop_back();
        return b64;
    }

    mutable std::mutex m_mtx;
    std::unordered_map<std::wstring, PinnedEndpoint> m_pins;
};

}  // namespace Engine
}  // namespace ExplorerLens
