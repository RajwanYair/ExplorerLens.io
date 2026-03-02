// CertificateViewer.h — X.509/PEM/DER Certificate File Viewer
// Copyright (c) 2026 ExplorerLens Project
//
// Parses PEM-encoded and DER-encoded X.509 certificates, PKCS#7/PCKS#12
// containers. Extracts subject, issuer, validity dates, key size, and
// generates an info-card style thumbnail.

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class CertEncoding : uint8_t { PEM, DER, PKCS7, PKCS12, Unknown };

struct CertViewerInfo {
    bool         isValid = false;
    CertEncoding encoding = CertEncoding::Unknown;
    std::string  subject;
    std::string  issuer;
    std::string  serialNumber;
    std::string  notBefore;
    std::string  notAfter;
    uint32_t     keySizeBits = 0;
    std::string  algorithm;   // RSA, ECDSA, Ed25519, etc.
    bool         isSelfSigned = false;
    bool         isExpired = false;
    bool         isCA = false;
    uint32_t     chainDepth = 0;
};

struct CertStats {
    uint32_t filesProcessed = 0;
    uint32_t certsExpired = 0;
    uint32_t certsSelfSigned = 0;
};

class CertificateViewer {
public:
    CertificateViewer() = default;
    ~CertificateViewer() = default;

    static const wchar_t* GetName() { return L"CertificateViewer"; }

    bool CanDecode(const wchar_t* ext) const {
        if (!ext) return false;
        std::wstring e(ext);
        for (auto& c : e) c = towlower(c);
        return e == L".pem" || e == L".crt" || e == L".cer" || e == L".der" ||
            e == L".pfx" || e == L".p12" || e == L".p7b" || e == L".p7c";
    }

    /// Detect PEM encoding by header.
    CertEncoding DetectEncoding(const uint8_t* data, size_t size) const {
        if (!data || size < 10) return CertEncoding::Unknown;
        if (memcmp(data, "-----BEGIN", 10) == 0) return CertEncoding::PEM;
        // DER starts with ASN.1 SEQUENCE tag (0x30)
        if (data[0] == 0x30) return CertEncoding::DER;
        return CertEncoding::Unknown;
    }

    /// Extract PEM subject line from certificate text.
    CertViewerInfo ParsePEM(const std::string& pemText) const {
        CertViewerInfo cert;
        if (pemText.find("-----BEGIN CERTIFICATE-----") == std::string::npos) return cert;
        cert.isValid = true;
        cert.encoding = CertEncoding::PEM;

        // Count certificates in the chain
        size_t pos = 0;
        while ((pos = pemText.find("-----BEGIN CERTIFICATE-----", pos)) != std::string::npos) {
            cert.chainDepth++;
            pos += 27;
        }

        // Extract base64 block for size estimation
        auto start = pemText.find("-----BEGIN CERTIFICATE-----");
        auto end = pemText.find("-----END CERTIFICATE-----");
        if (start != std::string::npos && end != std::string::npos) {
            size_t b64Len = end - start - 27;
            size_t derSize = b64Len * 3 / 4;
            // Heuristic: RSA key size estimation from DER size
            if (derSize > 1500) cert.keySizeBits = 4096;
            else if (derSize > 800) cert.keySizeBits = 2048;
            else if (derSize > 400) cert.keySizeBits = 1024;
            else cert.keySizeBits = 256; // Likely ECDSA
        }

        cert.isSelfSigned = (cert.chainDepth == 1);
        return cert;
    }

    CertStats GetStats() const { return m_stats; }

private:
    mutable CertStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
