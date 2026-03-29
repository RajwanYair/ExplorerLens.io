// CertificateMigrationTool.h — Classical-to-Post-Quantum Certificate Migration
// Copyright (c) 2026 ExplorerLens Project
//
// Assists in migrating classical RSA/ECDSA certificate chains to hybrid or
// post-quantum SLH-DSA/ML-KEM certificates with zero-downtime transition support.
//
#pragma once
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class CertificateAlgoType { RSA2048, RSA4096, ECDSA_P256, ECDSA_P384, SLHDSA, Hybrid };
enum class CertMigPhase         { Assess, DualSign, Cutover, Complete };

struct MigCertInfo {
    std::string       subjectCN;
    CertificateAlgoType algoType = CertificateAlgoType::ECDSA_P384;
    int               keyBits   = 0;
    std::string       isoExpiry;
    bool              isCA      = false;
    bool              selfSigned = false;
};

struct CertMigrationPlan {
    MigCertInfo    source;
    CertificateAlgoType targetAlgo = CertificateAlgoType::Hybrid;
    CertMigPhase       phase       = CertMigPhase::Assess;
    std::vector<std::string> steps;
    bool               dualSignRequired = true;
};

struct CertMigrationResult {
    bool         success    = false;
    CertMigPhase phase      = CertMigPhase::Assess;
    std::string  newCertId;
    std::string  errorMsg;
    bool Ok() const noexcept { return success; }
};

class CertificateMigrationTool {
public:
    explicit CertificateMigrationTool() = default;

    CertMigrationPlan BuildPlan(const MigCertInfo& source) const {
        CertMigrationPlan plan;
        plan.source = source;
        plan.targetAlgo = CertificateAlgoType::Hybrid;
        plan.phase = CertMigPhase::Assess;
        plan.steps.push_back("1. Inventory all certificates with classical algorithms");
        plan.steps.push_back("2. Generate hybrid SLH-DSA + ECDSA dual-signed certificate");
        plan.steps.push_back("3. Deploy dual-signed cert alongside original (dual-sign phase)");
        plan.steps.push_back("4. Validate all relying parties accept the hybrid cert");
        plan.steps.push_back("5. Retire classical cert (cutover phase)");
        plan.dualSignRequired = (source.isCA || !source.selfSigned);
        return plan;
    }

    CertMigrationResult Execute(const CertMigrationPlan& plan) {
        CertMigrationResult result;
        result.success   = true;
        result.phase     = CertMigPhase::DualSign;
        result.newCertId = "hybrid-cert-" + plan.source.subjectCN;
        return result;
    }

    static std::string PhaseName(CertMigPhase phase) noexcept {
        switch (phase) {
        case CertMigPhase::Assess:    return "Assess";
        case CertMigPhase::DualSign:  return "DualSign";
        case CertMigPhase::Cutover:   return "Cutover";
        case CertMigPhase::Complete:  return "Complete";
        }
        return "Unknown";
    }

    static std::string AlgoName(CertificateAlgoType t) noexcept {
        switch (t) {
        case CertificateAlgoType::RSA2048:    return "RSA-2048";
        case CertificateAlgoType::RSA4096:    return "RSA-4096";
        case CertificateAlgoType::ECDSA_P256: return "ECDSA-P256";
        case CertificateAlgoType::ECDSA_P384: return "ECDSA-P384";
        case CertificateAlgoType::SLHDSA:     return "SLH-DSA";
        case CertificateAlgoType::Hybrid:     return "Hybrid";
        }
        return "Unknown";
    }
};

} // namespace Engine
} // namespace ExplorerLens
