// ThumbnailManifestSigner.h — ECDSA-P256 Manifest Signer
// Copyright (c) 2026 ExplorerLens Project
//
// Signs thumbnail manifests with ECDSA-P256 + SHA-256 via Windows CNG
// (BCryptProvider) so downstream consumers can verify integrity before
// trusting a cached thumbnail.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

struct ManifestSignature {
    std::vector<uint8_t> sig;          // DER-encoded ECDSA signature
    std::string          keyId;        // Identifies the signing key
    uint64_t             signedAtMs    = 0;
    bool                 valid         = false;
};

struct ThumbnailManifestEntry {
    std::wstring path;
    std::string  contentHash;    // SHA-256 of thumbnail bytes
    uint64_t     mtimeMs        = 0;
    uint64_t     sizeBytes      = 0;
};

class ThumbnailManifestSigner {
public:
    struct Config {
        std::string  keyId          = "default";
        bool         useHardwareKey = false;
    };

    explicit ThumbnailManifestSigner(const Config& cfg = {}) : m_cfg(cfg) {}

    ManifestSignature Sign(const std::vector<ThumbnailManifestEntry>& entries,
                           uint64_t nowMs);
    bool              Verify(const std::vector<ThumbnailManifestEntry>& entries,
                             const ManifestSignature& sig) const;

    uint32_t SignCount()   const { return m_signCount; }
    uint32_t VerifyCount() const { return m_verifyCount; }

    const Config& GetConfig() const { return m_cfg; }

private:
    std::string BuildCanonical(const std::vector<ThumbnailManifestEntry>& entries) const;

    Config   m_cfg;
    uint32_t m_signCount   = 0;
    mutable uint32_t m_verifyCount = 0;
};

}} // namespace ExplorerLens::Engine
