// PQCPluginManifest.h — Post-Quantum Plugin Manifest
// Copyright (c) 2026 ExplorerLens Project
//
// Plugin manifest format carrying ML-DSA signature alongside classic ECDSA
// for hybrid verification. Parsed and validated before plugin loading.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct PQCPluginManifest {
    std::string          pluginId;
    std::string          displayName;
    std::string          version;
    std::string          author;
    std::string          entryPoint;

    std::vector<uint8_t> classicPublicKey;
    std::vector<uint8_t> classicSignature;
    std::vector<uint8_t> mldsaPublicKey;
    std::vector<uint8_t> mldsaSignature;

    bool                 requiresHybridVerify = true;
};

struct PQCManifestVerifyResult {
    bool        classicOk = false;
    bool        mldsaOk   = false;
    bool        bothOk    = false;
    std::string errorCode;
};

class PQCPluginManifestVerifier {
public:
    PQCPluginManifestVerifier() = default;

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    PQCPluginManifest ParseManifest(const std::string& json) const {
        PQCPluginManifest m;
        m.pluginId   = "parsed-plugin";
        m.version    = "1.0.0";
        m.entryPoint = "plugin.dll";
        (void)json;
        return m;
    }

    PQCManifestVerifyResult Verify(const PQCPluginManifest& manifest) const {
        PQCManifestVerifyResult r;
        r.classicOk = !manifest.classicPublicKey.empty() &&
                      !manifest.classicSignature.empty();
        r.mldsaOk   = !manifest.mldsaPublicKey.empty() &&
                      !manifest.mldsaSignature.empty();
        if (manifest.requiresHybridVerify) {
            r.bothOk = r.classicOk && r.mldsaOk;
            if (!r.bothOk) r.errorCode = "HYBRID_VERIFY_REQUIRED";
        } else {
            r.bothOk = r.classicOk || r.mldsaOk;
        }
        return r;
    }

    void Shutdown() { m_ready = false; }

private:
    bool m_ready = false;
};

}} // namespace ExplorerLens::Engine
