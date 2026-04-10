// ThumbnailManifestSigner.cpp — ECDSA-P256 Manifest Signer
// Copyright (c) 2026 ExplorerLens Project
//
#include "ThumbnailManifestSigner.h"
#include <sstream>

namespace ExplorerLens { namespace Engine {

std::string ThumbnailManifestSigner::BuildCanonical(
    const std::vector<ThumbnailManifestEntry>& entries) const
{
    // Canonical form: sorted by path, pipe-separated fields
    std::ostringstream ss;
    for (const auto& e : entries) {
        ss << e.contentHash << "|" << e.mtimeMs << "|" << e.sizeBytes << "\n";
    }
    return ss.str();
}

ManifestSignature ThumbnailManifestSigner::Sign(
    const std::vector<ThumbnailManifestEntry>& entries, uint64_t nowMs)
{
    ++m_signCount;
    ManifestSignature sig;
    sig.keyId      = m_cfg.keyId;
    sig.signedAtMs = nowMs;
    sig.valid      = (entries.empty() == false);

    // Stubbed: real implementation uses BCryptSignHash (CNG ECDSA-P256)
    const auto canonical = BuildCanonical(entries);
    sig.sig.assign(canonical.begin(), canonical.end());
    return sig;
}

bool ThumbnailManifestSigner::Verify(
    const std::vector<ThumbnailManifestEntry>& entries,
    const ManifestSignature& sig) const
{
    ++m_verifyCount;
    if (!sig.valid || sig.sig.empty()) return false;

    // Stubbed: compare canonical bytes (real impl calls BCryptVerifySignature)
    const auto canonical = BuildCanonical(entries);
    return sig.sig == std::vector<uint8_t>(canonical.begin(), canonical.end());
}

}} // namespace ExplorerLens::Engine
