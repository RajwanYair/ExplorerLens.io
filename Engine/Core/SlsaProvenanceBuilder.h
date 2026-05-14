// Engine/Core/SlsaProvenanceBuilder.h
#pragma once

// SlsaProvenanceBuilder — SLSA Level 2 provenance bundle builder (S367)
//
// Complements SlsaAttestationRecord.h (S347) which holds the attestation
// record types. SlsaProvenanceBuilder assembles a complete SLSA provenance
// bundle from its constituent parts (builder, invocation, materials) and
// serialises it to the CycloneDX / SLSA JSON format expected by
// github/attest-build-provenance.
//
// SLSA reference: https://slsa.dev/spec/v1.0/provenance
// ROADMAP ref: Phase 3 exit criterion — "SLSA L2 provenance on releases (H32)"
//
// Usage:
//   auto builder = SlsaProvenanceBuilder::ForGitHubActions();
//   builder.SetSubject(L"ExplorerLens-39.9.0-x64.msi", sha256hex);
//   builder.SetVersion(L"39.9.0");
//   std::string json = builder.Build();

#ifndef EXPLORERLENS_ENGINE_SLSAPROVENANCEBUILDER_H
#define EXPLORERLENS_ENGINE_SLSAPROVENANCEBUILDER_H

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// SlsaProvenanceLevel — SLSA specification level
// ---------------------------------------------------------------------------
enum class SlsaProvenanceLevel : std::uint8_t {
    L0 = 0,  ///< No guarantees
    L1 = 1,  ///< Provenance exists
    L2 = 2,  ///< Hosted build platform (GitHub Actions)
    L3 = 3,  ///< Hardened builds (hermetic, two-party review)
};

// ---------------------------------------------------------------------------
// SlsaSubjectDigest — artifact subject with its content digest
// ---------------------------------------------------------------------------
struct SlsaSubjectDigest {
    std::wstring  artifactName;   ///< e.g. L"ExplorerLens-39.9.0-x64.msi"
    std::string   sha256Hex;      ///< 64-character lower-case hex SHA-256
    std::uint64_t sizeBytes = 0u; ///< Artifact size (0 = unknown)

    [[nodiscard]] bool IsValid() const noexcept {
        return !artifactName.empty() && sha256Hex.size() == 64u;
    }
};

// ---------------------------------------------------------------------------
// SlsaBuilderMetadata — describes the trusted build platform
// ---------------------------------------------------------------------------
struct SlsaBuilderMetadata {
    std::string builderId;         ///< e.g. "https://github.com/actions/runner/..."
    std::string builderVersion;    ///< Runner version string
    std::string workflowRef;       ///< e.g. "RajwanYair/ExplorerLens.io/.github/workflows/release.yml@refs/tags/v39.9.0"
    std::string runId;             ///< GitHub Actions run ID
    std::string repositoryUri;     ///< "https://github.com/RajwanYair/ExplorerLens.io"
    std::string refName;           ///< e.g. "refs/tags/v39.9.0"
    std::string sha1Commit;        ///< 40-character git commit SHA

    [[nodiscard]] bool IsValid() const noexcept {
        return !builderId.empty() && !workflowRef.empty();
    }
};

// ---------------------------------------------------------------------------
// SlsaProvenance Bundle — assembled SLSA v1.0 provenance document
// ---------------------------------------------------------------------------
struct SlsaProvenanceBundle {
    SlsaProvenanceLevel         level    = SlsaProvenanceLevel::L2;
    std::vector<SlsaSubjectDigest> subjects;
    SlsaBuilderMetadata         builder;
    std::string                 buildStartedOn;   ///< ISO-8601 UTC
    std::string                 buildFinishedOn;  ///< ISO-8601 UTC
    std::uint32_t               materialCount = 0u; ///< vcpkg + external lib count

    [[nodiscard]] bool IsComplete() const noexcept {
        return !subjects.empty() && builder.IsValid() && !buildStartedOn.empty();
    }
};

// ---------------------------------------------------------------------------
// SlsaProvenance BuilderStatus
// ---------------------------------------------------------------------------
enum class SlsaProvBuildStatus : std::uint8_t {
    OK              = 0,
    MISSING_SUBJECT = 1,  ///< No subjects added before Build()
    MISSING_BUILDER = 2,  ///< Builder metadata incomplete
    INVALID_DIGEST  = 3,  ///< SHA-256 hex string is not 64 chars
    SERIAL_ERROR    = 4,  ///< JSON serialisation failed
};

// ---------------------------------------------------------------------------
// SlsaProvenanceBuilder — fluent API for constructing a provenance bundle
// ---------------------------------------------------------------------------
class SlsaProvenanceBuilder final {
public:
    SlsaProvenanceBuilder() noexcept = default;

    SlsaProvenanceBuilder(const SlsaProvenanceBuilder&)            = default;
    SlsaProvenanceBuilder& operator=(const SlsaProvenanceBuilder&) = default;

    /// Pre-populated builder for GitHub Actions environment variables.
    [[nodiscard]] static SlsaProvenanceBuilder ForGitHubActions() noexcept;

    /// Sets the SLSA level (default L2 for GitHub Actions).
    SlsaProvenanceBuilder& SetLevel(SlsaProvenanceLevel level) noexcept;

    /// Adds an artifact subject (call once per release artifact).
    SlsaProvenanceBuilder& SetSubject(
        const std::wstring& artifactName,
        const std::string&  sha256Hex,
        std::uint64_t       sizeBytes = 0u) noexcept;

    /// Overrides the auto-populated builder metadata.
    SlsaProvenanceBuilder& SetBuilder(const SlsaBuilderMetadata& meta) noexcept;

    /// Sets the product version string (embedded in the provenance statement).
    SlsaProvenanceBuilder& SetVersion(const std::wstring& version) noexcept;

    /// Sets the ISO-8601 build start timestamp.
    SlsaProvenanceBuilder& SetBuildStartedOn(const std::string& iso8601) noexcept;

    /// Sets the material count (vcpkg + external lib entry count).
    SlsaProvenanceBuilder& SetMaterialCount(std::uint32_t count) noexcept;

    /// Validates the bundle and serialises it to SLSA provenance JSON.
    /// Returns an empty string on error; check status via BuildWithStatus().
    [[nodiscard]] std::string Build() const noexcept;

    /// Returns the assembled bundle for inspection without serialisation.
    [[nodiscard]] const SlsaProvenanceBundle& Bundle() const noexcept { return m_bundle; }

    /// Returns the current subject count.
    [[nodiscard]] std::uint32_t SubjectCount() const noexcept;

    /// Returns the configured SLSA level.
    [[nodiscard]] SlsaProvenanceLevel Level() const noexcept { return m_bundle.level; }

    /// kSlsaPredicateTypeV1 — standard SLSA predicate type URI.
    static constexpr const char* kSlsaPredicateTypeV1 =
        "https://slsa.dev/provenance/v1";

private:
    SlsaProvenanceBundle m_bundle{};
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_SLSAPROVENANCEBUILDER_H
