// Engine/Core/SlsaAttestationRecord.h
// ExplorerLens — SLSA Level 2 build provenance record (H32 / ROADMAP v8.0 Phase 3)
// Sprint S347.
//
// Purpose:
//   Phase 3 exit criterion: "SLSA L2 provenance on releases (H32)"
//   (actions/attest-build-provenance@v2 in CI).
//
//   SlsaAttestationRecord provides:
//     1. Typed enums for SLSA Level and build source.
//     2. A lightweight record struct that the CI pipeline fills in at build time
//        and that the SBOM generator (SBOMGenerator.h) embeds in the CycloneDX JSON.
//     3. A Validate() method that checks field completeness before release.
//
//   Real attestation (Sigstore-backed) is performed by the GitHub Action
//   `actions/attest-build-provenance@v2`.  This header only provides the data
//   contract — no network calls or signing operations are performed in the engine.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_SLSA_ATTESTATION_RECORD_H
#define EXPLORERLENS_ENGINE_SLSA_ATTESTATION_RECORD_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// SlsaLevel — SLSA provenance level
// ---------------------------------------------------------------------------

enum class SlsaLevel : std::uint8_t {
    L0 = 0,  ///< No guarantees (default / local build)
    L1 = 1,  ///< Build script exists; output provenance documented
    L2 = 2,  ///< Hosted build service; signed provenance  ← Phase 3 target
    L3 = 3,  ///< Source + build hardened (hermetic, non-falsifiable)
};

// ---------------------------------------------------------------------------
// SlsaBuildSource — where the build ran
// ---------------------------------------------------------------------------

enum class SlsaBuildSource : std::uint8_t {
    LOCAL           = 0,  ///< Developer workstation
    GITHUB_ACTIONS  = 1,  ///< github.com hosted runner  ← target for Phase 3
    AZURE_DEVOPS    = 2,
    JENKINS         = 3,
    OTHER           = 255,
};

// ---------------------------------------------------------------------------
// SlsaValidationStatus
// ---------------------------------------------------------------------------

enum class SlsaValidationStatus : std::uint8_t {
    OK                   = 0,
    MISSING_BUILDER_ID   = 1,  ///< builderID is empty
    MISSING_SOURCE_URI   = 2,  ///< sourceUri is empty
    MISSING_COMMIT_SHA   = 3,  ///< commitSha is empty or wrong length (< 40 hex chars)
    MISSING_VERSION      = 4,  ///< version string is empty
    LEVEL_TOO_LOW        = 5,  ///< level < L2 for a release build
    ATTESTATION_DISABLED = 6,  ///< attestationEnabled is false
};

// ---------------------------------------------------------------------------
// SlsaAttestationRecord
// ---------------------------------------------------------------------------

struct SlsaAttestationRecord final {
    // ------------------------------------------------------------------
    // Provenance metadata
    // ------------------------------------------------------------------

    /// Builder ID URI — e.g. "https://github.com/actions/runner"
    std::string builderID{};

    /// Source repository URI — e.g. "https://github.com/RajwanYair/ExplorerLens.io"
    std::string sourceUri{};

    /// Git commit SHA-1 (40 hex characters) or SHA-256 for modern repos.
    std::string commitSha{};

    /// Ref / tag at build time — e.g. "refs/tags/v39.5.0"
    std::string ref{};

    /// Product version string — e.g. "39.5.0"
    std::string version{};

    /// Workflow / job name — e.g. "build-and-release"
    std::string buildWorkflow{};

    /// Achieved SLSA level for this build.
    SlsaLevel level = SlsaLevel::L0;

    /// Where the build ran.
    SlsaBuildSource buildSource = SlsaBuildSource::LOCAL;

    /// True when attestation has been submitted to Sigstore / Rekor.
    bool attestationEnabled = false;

    /// True when the provenance bundle has been uploaded to the GitHub Release.
    bool bundleUploaded = false;

    // ------------------------------------------------------------------
    // Validation
    // ------------------------------------------------------------------

    [[nodiscard]] SlsaValidationStatus Validate() const noexcept
    {
        if (!attestationEnabled)
            return SlsaValidationStatus::ATTESTATION_DISABLED;
        if (builderID.empty())
            return SlsaValidationStatus::MISSING_BUILDER_ID;
        if (sourceUri.empty())
            return SlsaValidationStatus::MISSING_SOURCE_URI;
        if (commitSha.size() < 40u)
            return SlsaValidationStatus::MISSING_COMMIT_SHA;
        if (version.empty())
            return SlsaValidationStatus::MISSING_VERSION;
        if (static_cast<std::uint8_t>(level) < static_cast<std::uint8_t>(SlsaLevel::L2))
            return SlsaValidationStatus::LEVEL_TOO_LOW;
        return SlsaValidationStatus::OK;
    }

    [[nodiscard]] bool MeetsPhase3Requirement() const noexcept
    {
        return level >= SlsaLevel::L2 &&
               buildSource == SlsaBuildSource::GITHUB_ACTIONS &&
               attestationEnabled &&
               bundleUploaded;
    }

    // ------------------------------------------------------------------
    // Factory helpers
    // ------------------------------------------------------------------

    /// Build a minimal L2 record for a GitHub Actions release build.
    [[nodiscard]] static SlsaAttestationRecord MakeGitHubActionsL2(
        const std::string& ver,
        const std::string& sha,
        const std::string& refStr = "") noexcept
    {
        SlsaAttestationRecord r;
        r.builderID         = "https://github.com/actions/runner";
        r.sourceUri         = "https://github.com/RajwanYair/ExplorerLens.io";
        r.version           = ver;
        r.commitSha         = sha;
        r.ref               = refStr;
        r.level             = SlsaLevel::L2;
        r.buildSource       = SlsaBuildSource::GITHUB_ACTIONS;
        r.attestationEnabled = true;
        r.bundleUploaded    = false;  // set to true after upload step
        return r;
    }
};

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

/// Minimum commit SHA length accepted by Validate().
inline constexpr std::size_t kSlsaMinCommitShaLength = 40u;

/// Target SLSA level for Phase 3 releases.
inline constexpr SlsaLevel kSlsaPhase3TargetLevel = SlsaLevel::L2;

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_SLSA_ATTESTATION_RECORD_H
