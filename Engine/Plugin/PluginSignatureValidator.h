// PluginSignatureValidator.h — Code signing validation for plugin bundles
// Copyright (c) 2026 ExplorerLens Project
//
// Validates cryptographic signatures on plugin bundles before loading. Supports
// Authenticode (Windows), Ed25519 detached signatures, and SHA-256 manifest
// hashes. Integrates with the plugin trust chain to enforce signature policies.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class SignatureAlgorithm : uint8_t
{
    Authenticode = 0,
    Ed25519      = 1,
    SHA256Manifest = 2,
    None         = 255,
};

enum class ValidationResult : uint8_t
{
    Valid        = 0,
    Invalid      = 1,
    Untrusted    = 2,
    Expired      = 3,
    NotSigned    = 4,
    Error        = 255,
};

struct SignatureInfo
{
    SignatureAlgorithm algorithm  = SignatureAlgorithm::None;
    ValidationResult  result     = ValidationResult::NotSigned;
    std::string       signer;
    std::string       thumbprint;
    bool              trusted    = false;
};

class PluginSignatureValidator
{
public:
    PluginSignatureValidator();
    ~PluginSignatureValidator();

    PluginSignatureValidator(const PluginSignatureValidator&)            = delete;
    PluginSignatureValidator& operator=(const PluginSignatureValidator&) = delete;

    bool          Initialize();
    void          Shutdown();
    SignatureInfo Validate(const std::string& pluginPath) const;
    bool          IsTrustedSigner(const std::string& thumbprint) const;
    bool          AddTrustedThumbprint(const std::string& thumbprint);
    uint32_t      TrustedCount() const noexcept { return static_cast<uint32_t>(m_trustedThumbprints.size()); }

    static PluginSignatureValidator& Instance() noexcept;

private:
    std::vector<std::string>   m_trustedThumbprints;
    bool                       m_initialized = false;
    static PluginSignatureValidator s_instance;
};

}} // namespace ExplorerLens::Engine
