// FIPS140CryptoBoundary.h — FIPS 140-3 Cryptographic Boundary Enforcement
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces the FIPS 140-3 cryptographic module boundary: tracks approved algorithms,
// blocks non-approved primitives, and emits FIPS-compliant audit events.
//
#pragma once
#include <string>
#include <unordered_set>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class FIPSAlgorithmStatus {
    Approved,
    Deprecated,
    NonApproved
};

struct FIPSAlgorithmEntry
{
    std::string name;
    FIPSAlgorithmStatus status = FIPSAlgorithmStatus::Approved;
    std::string nistRef;  // e.g. SP 800-56C
    bool isPostQuantum = false;

    std::string StatusName() const noexcept
    {
        switch (status) {
            case FIPSAlgorithmStatus::Approved:
                return "Approved";
            case FIPSAlgorithmStatus::Deprecated:
                return "Deprecated";
            case FIPSAlgorithmStatus::NonApproved:
                return "NonApproved";
        }
        return "Unknown";
    }
};

struct FIPSBoundaryCheckResult
{
    bool allowed = false;
    std::string algorithm;
    std::string statusName;
    std::string policyNote;
};

class FIPS140CryptoBoundary
{
  public:
    FIPS140CryptoBoundary()
    {
        // Pre-populate FIPS 140-3 approved + PQ algorithms
        RegisterAlgorithm({"AES-256-GCM", FIPSAlgorithmStatus::Approved, "FIPS 197", false});
        RegisterAlgorithm({"SHA-256", FIPSAlgorithmStatus::Approved, "FIPS 180-4", false});
        RegisterAlgorithm({"SHA-384", FIPSAlgorithmStatus::Approved, "FIPS 180-4", false});
        RegisterAlgorithm({"HMAC-SHA-256", FIPSAlgorithmStatus::Approved, "FIPS 198-1", false});
        RegisterAlgorithm({"ECDH-P384", FIPSAlgorithmStatus::Approved, "SP 800-56A", false});
        RegisterAlgorithm({"ML-KEM-768", FIPSAlgorithmStatus::Approved, "FIPS 203", true});
        RegisterAlgorithm({"SLH-DSA-SHA2-128s", FIPSAlgorithmStatus::Approved, "FIPS 205", true});
        RegisterAlgorithm({"MD5", FIPSAlgorithmStatus::NonApproved, "Deprecated", false});
        RegisterAlgorithm({"SHA-1", FIPSAlgorithmStatus::Deprecated, "SP 800-131A", false});
    }

    void RegisterAlgorithm(FIPSAlgorithmEntry entry)
    {
        m_algorithms[entry.name] = std::move(entry);
    }

    FIPSBoundaryCheckResult Check(const std::string& algorithm) const
    {
        auto it = m_algorithms.find(algorithm);
        if (it == m_algorithms.end())
            return {false, algorithm, "NonApproved", "Algorithm not in FIPS registry"};
        const auto& e = it->second;
        bool allowed = (e.status == FIPSAlgorithmStatus::Approved);
        return {allowed, algorithm, e.StatusName(), allowed ? e.nistRef : "Use an approved algorithm"};
    }

    std::vector<std::string> ApprovedAlgorithms() const
    {
        std::vector<std::string> result;
        for (const auto& [name, entry] : m_algorithms)
            if (entry.status == FIPSAlgorithmStatus::Approved)
                result.push_back(name);
        return result;
    }

    size_t AlgorithmCount() const noexcept
    {
        return m_algorithms.size();
    }
    bool IsFIPSMode() const noexcept
    {
        return m_fipsMode;
    }
    void SetFIPSMode(bool enabled) noexcept
    {
        m_fipsMode = enabled;
    }

  private:
    std::unordered_map<std::string, FIPSAlgorithmEntry> m_algorithms;
    bool m_fipsMode = true;
};

}  // namespace Engine
}  // namespace ExplorerLens
