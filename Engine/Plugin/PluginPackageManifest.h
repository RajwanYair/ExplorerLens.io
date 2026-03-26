// PluginPackageManifest.h — .lenspkg Package Format Manifest
// Copyright (c) 2026 ExplorerLens Project
//
// Defines the structure of a .lenspkg plugin package (ZIP archive with a
// manifest.json root entry) including metadata, capability declarations,
// and dependency requirements.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---- Package Capability Flags -----------------------------------------------

enum class PluginCapabilityFlag : uint32_t {
    None            = 0,
    Decode          = 1 << 0,   // Provides an IThumbnailPlugin decoder
    Encode          = 1 << 1,   // Reserved for future encode plugins
    Overlay         = 1 << 2,   // Composites an overlay badge on top of existing thumb
    MetadataSource  = 1 << 3,   // Provides file metadata for Explorer columns
    AIAssist        = 1 << 4,   // Uses AI/ML inference (GPU memory impact)
    RequiresNetwork = 1 << 5,   // Needs internet access (cloud-side decoding)
    RequiresAdmin   = 1 << 6,   // Requests elevated permissions at install
};

// ---- Dependency Spec --------------------------------------------------------

struct PluginDependency {
    std::string id;             // Plugin ID or "engine" for engine version req
    std::string minVersion;
    std::string maxVersion;     // Empty = no upper bound
    bool        optional = false;
};

// ---- Manifest ---------------------------------------------------------------

struct PluginPackageManifest {
    // Package identity
    std::string  id;                  // "com.vendor.pluginname" reverse-DNS
    std::string  displayName;
    std::string  version;             // SemVer "1.0.0"
    std::string  author;
    std::string  authorEmail;
    std::string  licenseId;           // SPDX identifier e.g. "MIT"
    std::string  description;
    std::string  homepageUrl;

    // Engine compatibility
    std::string  minEngineVersion;    // e.g. "18.2.0"
    std::string  maxEngineVersion;    // Empty = compatible with all future versions

    // Content declarations
    std::string  dllName;             // DLL filename inside the package
    std::string  entryPoint;          // Exported C function or COM CLSID
    uint32_t     capabilities = 0;    // Bitmask of PluginCapabilityFlag

    // Supported formats
    std::vector<std::string> extensions;  // e.g. {".xyz", ".abc"}
    std::vector<std::string> mimeTypes;

    // Dependencies
    std::vector<PluginDependency> dependencies;

    // Sandbox policy hints
    bool   requestsFileSystem = false;  // Needs disk I/O beyond the input file
    bool   requestsNetwork    = false;  // Needs outbound network
    std::vector<std::string> allowedRegistryPaths;  // HKLM/HKCU sub-key patterns

    // Package integrity
    std::string  sha256;          // SHA-256 of the .lenspkg ZIP before signing
    std::string  signatureB64;    // Base64 Authenticode/RSA-PSS signature over sha256

    // Parse manifest.json from raw JSON bytes.
    static bool Parse(const std::string& jsonText, PluginPackageManifest& out);

    // Serialize to compact JSON.
    std::string ToJson() const;
};

} // namespace Engine
} // namespace ExplorerLens
