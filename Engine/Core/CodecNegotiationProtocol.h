// CodecNegotiationProtocol.h — Codec Negotiation Protocol
// Copyright (c) 2026 ExplorerLens Project
//
// Negotiates optimal codec between ExplorerLens client and server thumbnail endpoints.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

enum class CNPCodec { NCF_v2, WebP, AVIF, HEIC, JPEG, Raw };

struct CNPOffer {
    std::vector<CNPCodec> supportedCodecs;
    uint32_t              maxBitrateKbps = 10000;
};

struct CNPNegotiateResult {
    bool      success        = false;
    CNPCodec  chosen         = CNPCodec::WebP;
    uint32_t  negotiatedKbps = 0;
    std::string errorMsg;
};

class CodecNegotiationProtocol {
public:
    void SetPreferences(const std::vector<CNPCodec>& codecs) { m_preferences = codecs; }

    CNPNegotiateResult Negotiate(const CNPOffer& peerOffer) {
        CNPNegotiateResult r;
        if (peerOffer.supportedCodecs.empty()) { r.errorMsg = "No codecs offered"; return r; }
        // Pick best match from our preferences
        for (const auto& preferred : m_preferences) {
            auto it = std::find(peerOffer.supportedCodecs.begin(),
                                peerOffer.supportedCodecs.end(), preferred);
            if (it != peerOffer.supportedCodecs.end()) {
                r.chosen         = preferred;
                r.negotiatedKbps = std::min(peerOffer.maxBitrateKbps, 5000u);
                r.success        = true;
                return r;
            }
        }
        // Fallback to peer's first
        r.chosen         = peerOffer.supportedCodecs.front();
        r.negotiatedKbps = std::min(peerOffer.maxBitrateKbps, 2000u);
        r.success        = true;
        return r;
    }
    static std::string CodecName(CNPCodec codec) {
        switch (codec) {
            case CNPCodec::NCF_v2: return "NCF_v2";
            case CNPCodec::WebP:   return "WebP";
            case CNPCodec::AVIF:   return "AVIF";
            case CNPCodec::HEIC:   return "HEIC";
            case CNPCodec::JPEG:   return "JPEG";
            case CNPCodec::Raw:    return "Raw";
        }
        return "Unknown";
    }
    uint32_t PreferenceCount() const { return static_cast<uint32_t>(m_preferences.size()); }

private:
    std::vector<CNPCodec> m_preferences = { CNPCodec::NCF_v2, CNPCodec::AVIF, CNPCodec::WebP };
};

}} // namespace ExplorerLens::Engine
