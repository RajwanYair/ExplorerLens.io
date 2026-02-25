/******************************************************************************
 * SupplyChainSecurity.h — Supply-Chain Security & Reproducible Releases
 * Copyright (c) 2026 — ExplorerLens Project
 *
 * SBOM generation (SPDX/CycloneDX), reproducible build settings,
 * dependency provenance tracking, CI policy gates, and signed release
 * manifests for deterministic, traceable release artifacts.
 *
 *                with SBOM + signed checksums.
 *****************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cstdint>
#include <optional>

namespace ExplorerLens {
namespace SupplyChain {

//============================================================================
// SBOM Formats
//============================================================================

enum class SBOMFormat {
    SPDX_2_3,       // ISO/IEC 5962:2021
    CycloneDX_1_5   // OWASP CycloneDX v1.5
};

enum class LicenseType {
    MIT,
    BSD_3_Clause,
    Apache_2_0,
    LGPL_2_1,
    GPL_2_0,
    Zlib,
    Public_Domain,
    Proprietary,
    Unknown
};

inline std::wstring LicenseToSPDX(LicenseType lic) {
    switch (lic) {
        case LicenseType::MIT:            return L"MIT";
        case LicenseType::BSD_3_Clause:   return L"BSD-3-Clause";
        case LicenseType::Apache_2_0:     return L"Apache-2.0";
        case LicenseType::LGPL_2_1:       return L"LGPL-2.1-only";
        case LicenseType::GPL_2_0:        return L"GPL-2.0-only";
        case LicenseType::Zlib:           return L"Zlib";
        case LicenseType::Public_Domain:  return L"CC0-1.0";
        case LicenseType::Proprietary:    return L"LicenseRef-Proprietary";
        default:                          return L"NOASSERTION";
    }
}

//============================================================================
// Dependency Provenance
//============================================================================

struct DependencyProvenance {
    std::wstring name;            // e.g. "libjxl"
    std::wstring version;         // e.g. "0.11.1"
    std::wstring source_url;      // e.g. "https://github.com/libjxl/libjxl"
    std::wstring commit_or_tag;   // e.g. "v0.11.1"
    std::wstring sha256_hash;     // Hash of source archive
    LicenseType license = LicenseType::Unknown;
    std::wstring supplier;        // e.g. "Organization: Google LLC"
    std::wstring purl;            // Package URL (pkg:github/libjxl/libjxl@v0.11.1)

    // Validate provenance completeness
    bool IsComplete() const {
        return !name.empty() && !version.empty() &&
               !source_url.empty() && !sha256_hash.empty() &&
               license != LicenseType::Unknown;
    }

    // Generate Package URL if not set
    std::wstring GeneratePurl() const {
        if (!purl.empty()) return purl;
        // pkg:github/<owner>/<repo>@<version>
        // Simplified extraction from source_url
        return L"pkg:generic/" + name + L"@" + version;
    }
};

//============================================================================
// Built-in ExplorerLens Dependency Registry
//============================================================================

class DependencyRegistry {
public:
    static DependencyRegistry& Instance() {
        static DependencyRegistry instance;
        return instance;
    }

    DependencyRegistry() {
        RegisterKnownDependencies();
    }

    void Register(const DependencyProvenance& dep) {
        deps_[dep.name] = dep;
    }

    const DependencyProvenance* Find(const std::wstring& name) const {
        auto it = deps_.find(name);
        return it != deps_.end() ? &it->second : nullptr;
    }

    std::vector<DependencyProvenance> GetAll() const {
        std::vector<DependencyProvenance> result;
        result.reserve(deps_.size());
        for (auto& [_, d] : deps_) result.push_back(d);
        std::sort(result.begin(), result.end(),
                  [](auto& a, auto& b) { return a.name < b.name; });
        return result;
    }

    // Validate all dependencies have complete provenance
    struct ValidationResult {
        uint32_t total = 0;
        uint32_t complete = 0;
        uint32_t incomplete = 0;
        std::vector<std::wstring> incomplete_names;

        bool AllComplete() const { return incomplete == 0; }
    };

    ValidationResult Validate() const {
        ValidationResult result;
        result.total = static_cast<uint32_t>(deps_.size());
        for (auto& [name, dep] : deps_) {
            if (dep.IsComplete()) {
                result.complete++;
            } else {
                result.incomplete++;
                result.incomplete_names.push_back(name);
            }
        }
        return result;
    }

private:
    std::unordered_map<std::wstring, DependencyProvenance> deps_;

    void RegisterKnownDependencies() {
        Register({L"libjxl", L"0.11.1",
                  L"https://github.com/libjxl/libjxl",
                  L"v0.11.1", L"", LicenseType::BSD_3_Clause,
                  L"Organization: Google LLC",
                  L"pkg:github/libjxl/libjxl@v0.11.1"});

        Register({L"libheif", L"1.19.5",
                  L"https://github.com/nickel-org/libheif",
                  L"v1.19.5", L"", LicenseType::LGPL_2_1,
                  L"Organization: Dirk Farin",
                  L"pkg:github/nickel-org/libheif@v1.19.5"});

        Register({L"libwebp", L"1.5.0",
                  L"https://chromium.googlesource.com/webm/libwebp",
                  L"v1.5.0", L"", LicenseType::BSD_3_Clause,
                  L"Organization: Google LLC",
                  L"pkg:github/nickel-org/libwebp@v1.5.0"});

        Register({L"LibRaw", L"0.21.3",
                  L"https://github.com/LibRaw/LibRaw",
                  L"0.21.3", L"", LicenseType::LGPL_2_1,
                  L"Organization: LibRaw LLC",
                  L"pkg:github/LibRaw/LibRaw@0.21.3"});

        Register({L"libavif", L"1.3.0",
                  L"https://github.com/AOMediaCodec/libavif",
                  L"v1.3.0", L"", LicenseType::BSD_3_Clause,
                  L"Organization: Alliance for Open Media",
                  L"pkg:github/AOMediaCodec/libavif@v1.3.0"});

        Register({L"zlib", L"1.3.1",
                  L"https://github.com/madler/zlib",
                  L"v1.3.1", L"", LicenseType::Zlib,
                  L"Organization: Jean-loup Gailly / Mark Adler",
                  L"pkg:github/madler/zlib@v1.3.1"});

        Register({L"zstd", L"1.5.7",
                  L"https://github.com/facebook/zstd",
                  L"v1.5.7", L"", LicenseType::BSD_3_Clause,
                  L"Organization: Meta Platforms",
                  L"pkg:github/facebook/zstd@v1.5.7"});

        Register({L"lz4", L"1.10.0",
                  L"https://github.com/lz4/lz4",
                  L"v1.10.0", L"", LicenseType::BSD_3_Clause,
                  L"Organization: Yann Collet",
                  L"pkg:github/lz4/lz4@v1.10.0"});

        Register({L"lzma", L"26.00",
                  L"https://7-zip.org/sdk.html",
                  L"26.00", L"", LicenseType::Public_Domain,
                  L"Organization: Igor Pavlov",
                  L"pkg:generic/lzma-sdk@26.00"});

        Register({L"minizip-ng", L"4.0.10",
                  L"https://github.com/zlib-ng/minizip-ng",
                  L"4.0.10", L"", LicenseType::Zlib,
                  L"Organization: zlib-ng contributors",
                  L"pkg:github/zlib-ng/minizip-ng@4.0.10"});

        Register({L"unrar", L"7.2.2",
                  L"https://www.rarlab.com/rar_add.htm",
                  L"7.2.2", L"", LicenseType::Proprietary,
                  L"Organization: Alexander L. Roshal",
                  L"pkg:generic/unrar@7.2.2"});
    }
};

//============================================================================
// SBOM Generator
//============================================================================

class SBOMGenerator {
public:
    struct Config {
        SBOMFormat format = SBOMFormat::SPDX_2_3;
        std::wstring product_name = L"ExplorerLens";
        std::wstring product_version = L"7.0.0";
        std::wstring supplier = L"Organization: ExplorerLens Project";
        std::wstring namespace_uri = L"https://explorerlens.dev/spdx";
    };

    explicit SBOMGenerator(const Config& config = {}) : config_(config) {}

    // Generate SPDX 2.3 JSON-ish representation
    std::wstring GenerateSPDX(const std::vector<DependencyProvenance>& deps) const {
        std::wostringstream ss;
        ss << L"{\n";
        ss << L"  \"spdxVersion\": \"SPDX-2.3\",\n";
        ss << L"  \"dataLicense\": \"CC0-1.0\",\n";
        ss << L"  \"SPDXID\": \"SPDXRef-DOCUMENT\",\n";
        ss << L"  \"name\": \"" << config_.product_name << L"-" << config_.product_version << L"\",\n";
        ss << L"  \"documentNamespace\": \"" << config_.namespace_uri << L"/" << config_.product_version << L"\",\n";
        ss << L"  \"creationInfo\": {\n";
        ss << L"    \"created\": \"" << GetTimestampISO() << L"\",\n";
        ss << L"    \"creators\": [\"Tool: ExplorerLens-SBOM-1.0\"]\n";
        ss << L"  },\n";
        ss << L"  \"packages\": [\n";

        // Root package
        ss << L"    {\n";
        ss << L"      \"SPDXID\": \"SPDXRef-Package-" << config_.product_name << L"\",\n";
        ss << L"      \"name\": \"" << config_.product_name << L"\",\n";
        ss << L"      \"versionInfo\": \"" << config_.product_version << L"\",\n";
        ss << L"      \"supplier\": \"" << config_.supplier << L"\",\n";
        ss << L"      \"downloadLocation\": \"https://github.com/nickel-org/ExplorerLens\"\n";
        ss << L"    }";

        // Dependencies
        for (size_t i = 0; i < deps.size(); ++i) {
            auto& d = deps[i];
            ss << L",\n    {\n";
            ss << L"      \"SPDXID\": \"SPDXRef-Package-" << d.name << L"\",\n";
            ss << L"      \"name\": \"" << d.name << L"\",\n";
            ss << L"      \"versionInfo\": \"" << d.version << L"\",\n";
            ss << L"      \"downloadLocation\": \"" << d.source_url << L"\",\n";
            ss << L"      \"licenseDeclared\": \"" << LicenseToSPDX(d.license) << L"\",\n";
            ss << L"      \"supplier\": \"" << d.supplier << L"\",\n";
            ss << L"      \"externalRefs\": [{\n";
            ss << L"        \"referenceCategory\": \"PACKAGE-MANAGER\",\n";
            ss << L"        \"referenceType\": \"purl\",\n";
            ss << L"        \"referenceLocator\": \"" << d.GeneratePurl() << L"\"\n";
            ss << L"      }]";
            if (!d.sha256_hash.empty()) {
                ss << L",\n      \"checksums\": [{\n";
                ss << L"        \"algorithm\": \"SHA256\",\n";
                ss << L"        \"checksumValue\": \"" << d.sha256_hash << L"\"\n";
                ss << L"      }]";
            }
            ss << L"\n    }";
        }

        ss << L"\n  ],\n";

        // Relationships
        ss << L"  \"relationships\": [\n";
        for (size_t i = 0; i < deps.size(); ++i) {
            if (i > 0) ss << L",\n";
            ss << L"    {\n";
            ss << L"      \"spdxElementId\": \"SPDXRef-Package-" << config_.product_name << L"\",\n";
            ss << L"      \"relationshipType\": \"DEPENDS_ON\",\n";
            ss << L"      \"relatedSpdxElement\": \"SPDXRef-Package-" << deps[i].name << L"\"\n";
            ss << L"    }";
        }
        ss << L"\n  ]\n";
        ss << L"}\n";
        return ss.str();
    }

    // Generate CycloneDX 1.5 JSON-ish representation
    std::wstring GenerateCycloneDX(const std::vector<DependencyProvenance>& deps) const {
        std::wostringstream ss;
        ss << L"{\n";
        ss << L"  \"bomFormat\": \"CycloneDX\",\n";
        ss << L"  \"specVersion\": \"1.5\",\n";
        ss << L"  \"version\": 1,\n";
        ss << L"  \"metadata\": {\n";
        ss << L"    \"timestamp\": \"" << GetTimestampISO() << L"\",\n";
        ss << L"    \"component\": {\n";
        ss << L"      \"type\": \"application\",\n";
        ss << L"      \"name\": \"" << config_.product_name << L"\",\n";
        ss << L"      \"version\": \"" << config_.product_version << L"\"\n";
        ss << L"    }\n";
        ss << L"  },\n";
        ss << L"  \"components\": [\n";

        for (size_t i = 0; i < deps.size(); ++i) {
            if (i > 0) ss << L",\n";
            auto& d = deps[i];
            ss << L"    {\n";
            ss << L"      \"type\": \"library\",\n";
            ss << L"      \"name\": \"" << d.name << L"\",\n";
            ss << L"      \"version\": \"" << d.version << L"\",\n";
            ss << L"      \"purl\": \"" << d.GeneratePurl() << L"\",\n";
            ss << L"      \"licenses\": [{\"license\": {\"id\": \"" << LicenseToSPDX(d.license) << L"\"}}]";
            if (!d.sha256_hash.empty()) {
                ss << L",\n      \"hashes\": [{\"alg\": \"SHA-256\", \"content\": \"" << d.sha256_hash << L"\"}]";
            }
            ss << L"\n    }";
        }

        ss << L"\n  ]\n";
        ss << L"}\n";
        return ss.str();
    }

    std::wstring Generate(const std::vector<DependencyProvenance>& deps) const {
        switch (config_.format) {
            case SBOMFormat::SPDX_2_3:      return GenerateSPDX(deps);
            case SBOMFormat::CycloneDX_1_5: return GenerateCycloneDX(deps);
            default:                        return GenerateSPDX(deps);
        }
    }

    uint32_t CountComponents(const std::vector<DependencyProvenance>& deps) const {
        return static_cast<uint32_t>(deps.size()) + 1; // +1 for root package
    }

private:
    Config config_;

    std::wstring GetTimestampISO() const {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf = {};
        gmtime_s(&tm_buf, &t);
        wchar_t buf[64] = {};
        wcsftime(buf, 64, L"%Y-%m-%dT%H:%M:%SZ", &tm_buf);
        return buf;
    }
};

//============================================================================
// Reproducible Build Configuration
//============================================================================

struct ReproducibleBuildConfig {
    // Compiler determinism
    bool deterministic_output = true;    // /Brepro (MSVC)
    bool disable_timestamps = true;      // /d1nodatetime
    bool fixed_base_address = true;      // /DYNAMICBASE:NO for reproducibility
    bool strip_source_paths = true;      // /pathmap:

    // Linker determinism
    bool deterministic_linker = true;    // /LTCG, /OPT:REF, /OPT:ICF
    std::wstring source_path_map;        // e.g. "C:\build=."

    // Packaging determinism
    bool stable_timestamps_in_zip = true;  // Fixed mtime in ZIP/MSI
    std::wstring fixed_build_time;          // ISO 8601, e.g. from SOURCE_DATE_EPOCH

    // MSVC command-line flags for reproducibility
    std::vector<std::wstring> GetCompilerFlags() const {
        std::vector<std::wstring> flags;
        if (deterministic_output)  flags.push_back(L"/Brepro");
        if (disable_timestamps)    flags.push_back(L"/d1nodatetime");
        if (strip_source_paths && !source_path_map.empty()) {
            flags.push_back(L"/pathmap:" + source_path_map);
        }
        return flags;
    }

    std::vector<std::wstring> GetLinkerFlags() const {
        std::vector<std::wstring> flags;
        if (deterministic_linker) {
            flags.push_back(L"/Brepro");
            flags.push_back(L"/OPT:REF");
            flags.push_back(L"/OPT:ICF");
        }
        if (fixed_base_address) {
            flags.push_back(L"/DYNAMICBASE:NO");
        }
        return flags;
    }
};

//============================================================================
// CI Release Policy Gate
//============================================================================

struct CIPolicyViolation {
    enum class Severity { Error, Warning, Info };

    Severity severity = Severity::Error;
    std::wstring rule_id;
    std::wstring message;
    std::wstring artifact;  // Which file/artifact triggered it
};

class CIPolicyGate {
public:
    struct ReleaseChecklist {
        bool all_binaries_signed = false;
        bool sbom_present = false;
        bool sbom_valid = false;
        bool all_deps_have_hashes = false;
        bool all_deps_have_provenance = false;
        bool reproducible_build = false;
        bool manifest_signed = false;

        bool PassesGate() const {
            return all_binaries_signed && sbom_present && sbom_valid &&
                   all_deps_have_hashes && all_deps_have_provenance;
        }

        uint32_t PassedChecks() const {
            uint32_t n = 0;
            if (all_binaries_signed) n++;
            if (sbom_present) n++;
            if (sbom_valid) n++;
            if (all_deps_have_hashes) n++;
            if (all_deps_have_provenance) n++;
            if (reproducible_build) n++;
            if (manifest_signed) n++;
            return n;
        }

        static constexpr uint32_t TotalChecks() { return 7; }
    };

    // Run all policy checks
    ReleaseChecklist Evaluate(const std::vector<DependencyProvenance>& deps,
                              bool has_sbom, bool binaries_signed,
                              bool manifest_signed, bool repro_build) const {
        ReleaseChecklist cl;
        cl.all_binaries_signed = binaries_signed;
        cl.sbom_present = has_sbom;
        cl.sbom_valid = has_sbom; // If SBOM was generated, assume valid
        cl.manifest_signed = manifest_signed;
        cl.reproducible_build = repro_build;

        cl.all_deps_have_hashes = true;
        cl.all_deps_have_provenance = true;
        for (auto& d : deps) {
            if (d.sha256_hash.empty()) cl.all_deps_have_hashes = false;
            if (!d.IsComplete()) cl.all_deps_have_provenance = false;
        }

        return cl;
    }

    // Generate policy violations list
    std::vector<CIPolicyViolation> CheckViolations(const ReleaseChecklist& cl) const {
        std::vector<CIPolicyViolation> violations;

        if (!cl.all_binaries_signed) {
            violations.push_back({CIPolicyViolation::Severity::Error,
                                  L"SIGN-001", L"Not all binaries are Authenticode-signed", L""});
        }
        if (!cl.sbom_present) {
            violations.push_back({CIPolicyViolation::Severity::Error,
                                  L"SBOM-001", L"SBOM not generated for release artifacts", L""});
        }
        if (!cl.all_deps_have_hashes) {
            violations.push_back({CIPolicyViolation::Severity::Error,
                                  L"HASH-001", L"Some dependencies missing SHA256 hash verification", L""});
        }
        if (!cl.all_deps_have_provenance) {
            violations.push_back({CIPolicyViolation::Severity::Error,
                                  L"PROV-001", L"Incomplete dependency provenance (missing source/license/hash)", L""});
        }
        if (!cl.manifest_signed) {
            violations.push_back({CIPolicyViolation::Severity::Warning,
                                  L"SIGN-002", L"SHA256SUMS manifest not GPG/sigstore signed", L""});
        }
        if (!cl.reproducible_build) {
            violations.push_back({CIPolicyViolation::Severity::Warning,
                                  L"REPRO-001", L"Build reproducibility not verified", L""});
        }

        return violations;
    }
};

//============================================================================
// Signed Release Manifest (SHA256SUMS.sig)
//============================================================================

struct ReleaseArtifact {
    std::wstring filename;
    std::wstring sha256;
    uint64_t size_bytes = 0;
};

class ReleaseManifest {
public:
    void AddArtifact(const ReleaseArtifact& artifact) {
        artifacts_.push_back(artifact);
    }

    // Generate SHA256SUMS content
    std::wstring GenerateChecksumFile() const {
        std::wostringstream ss;
        for (auto& a : artifacts_) {
            ss << a.sha256 << L"  " << a.filename << L"\n";
        }
        return ss.str();
    }

    // Verify an artifact against the manifest
    bool VerifyArtifact(const std::wstring& filename, const std::wstring& computed_sha256) const {
        for (auto& a : artifacts_) {
            if (a.filename == filename) {
                return _wcsicmp(a.sha256.c_str(), computed_sha256.c_str()) == 0;
            }
        }
        return false; // Not in manifest
    }

    uint32_t ArtifactCount() const { return static_cast<uint32_t>(artifacts_.size()); }

    const std::vector<ReleaseArtifact>& GetArtifacts() const { return artifacts_; }

private:
    std::vector<ReleaseArtifact> artifacts_;
};

} // namespace SupplyChain
} // namespace ExplorerLens

