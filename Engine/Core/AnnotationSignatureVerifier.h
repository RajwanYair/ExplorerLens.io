// AnnotationSignatureVerifier.h — Annotation Signature Verifier
// Copyright (c) 2026 ExplorerLens Project
//
// Verifies cryptographic signatures on annotations to ensure authorship integrity.
//
#pragma once
#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ASVSignedAnnotation
{
    std::string payload;
    std::string authorId;
    std::string signature;  // hex-encoded hash for simulation
};

struct ASVVerifyResult
{
    bool valid = false;
    std::string authorId;
    std::string errorMsg;
};

class AnnotationSignatureVerifier
{
  public:
    void RegisterPublicKey(const std::string& authorId, const std::string& pubKeyHex)
    {
        m_keys[authorId] = pubKeyHex;
    }
    ASVVerifyResult Verify(const ASVSignedAnnotation& ann)
    {
        ASVVerifyResult r;
        r.authorId = ann.authorId;
        if (m_keys.find(ann.authorId) == m_keys.end()) {
            r.errorMsg = "Unknown author";
            return r;
        }
        // Simulate sig check via hash comparison
        std::ostringstream oss;
        oss << std::hex << std::hash<std::string>{}(ann.payload + m_keys[ann.authorId]);
        r.valid = (oss.str() == ann.signature);
        if (!r.valid)
            r.errorMsg = "Signature mismatch";
        return r;
    }
    static std::string Sign(const std::string& payload, const std::string& privKeyHex)
    {
        std::ostringstream oss;
        oss << std::hex << std::hash<std::string>{}(payload + privKeyHex);
        return oss.str();
    }

  private:
    std::unordered_map<std::string, std::string> m_keys;
};

}  // namespace Engine
}  // namespace ExplorerLens
