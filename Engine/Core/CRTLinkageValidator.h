// CRTLinkageValidator.h — Compile-Time CRT Linkage Validation
// Copyright (c) 2026 ExplorerLens Project
//
// Validates that all translation units use the correct CRT linkage (/MD).
// Detects mismatched CRT settings that cause linker conflicts.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

// Compile-time assertion: we must be using /MD (dynamic CRT)
#ifdef _MSC_VER
#if defined(_MT) && defined(_DLL)
    // Correct: /MD or /MDd
#elif defined(_MT)
static_assert(false, "ExplorerLens requires /MD (dynamic CRT). Detected /MT (static CRT).");
#endif
#endif

namespace ExplorerLens {
namespace Engine {

enum class CRTLinkageType : uint8_t {
    Unknown = 0,
    StaticMT = 1,   // /MT
    StaticMTd = 2,   // /MTd
    DynamicMD = 3,   // /MD
    DynamicMDd = 4    // /MDd
};

enum class CRTMismatchSeverity : uint8_t {
    None = 0,
    Warning = 1,
    Error = 2,
    Critical = 3
};

struct CRTLinkageReport {
    CRTLinkageType       detected = CRTLinkageType::Unknown;
    CRTLinkageType       expected = CRTLinkageType::DynamicMD;
    CRTMismatchSeverity  severity = CRTMismatchSeverity::None;
    bool                 isDebugBuild = false;
    bool                 isValid = false;
    uint32_t             mismatchCount = 0;
    std::string          details;
};

struct CRTModuleInfo {
    std::string    moduleName;
    CRTLinkageType linkage = CRTLinkageType::Unknown;
    bool           isExternal = false;
};

class CRTLinkageValidator {
public:
    static CRTLinkageValidator& Instance() { static CRTLinkageValidator s; return s; }

    CRTLinkageType DetectCurrentLinkage() const {
#ifdef _MSC_VER
#ifdef _DEBUG
#if defined(_MT) && defined(_DLL)
        return CRTLinkageType::DynamicMDd;
#elif defined(_MT)
        return CRTLinkageType::StaticMTd;
#endif
#else
#if defined(_MT) && defined(_DLL)
        return CRTLinkageType::DynamicMD;
#elif defined(_MT)
        return CRTLinkageType::StaticMT;
#endif
#endif
#endif
        return CRTLinkageType::Unknown;
    }

    CRTLinkageReport ValidateLinkage() const {
        CRTLinkageReport report{};
        report.detected = DetectCurrentLinkage();
#ifdef _DEBUG
        report.isDebugBuild = true;
        report.expected = CRTLinkageType::DynamicMDd;
#else
        report.isDebugBuild = false;
        report.expected = CRTLinkageType::DynamicMD;
#endif
        report.isValid = (report.detected == report.expected);
        report.severity = report.isValid ? CRTMismatchSeverity::None : CRTMismatchSeverity::Critical;
        report.details = report.isValid ? "CRT linkage OK" : "CRT linkage mismatch detected";
        return report;
    }

    bool CheckMismatch(const std::vector<CRTModuleInfo>& modules) const {
        if (modules.empty()) return true;
        CRTLinkageType baseline = modules[0].linkage;
        for (size_t i = 1; i < modules.size(); ++i) {
            if (modules[i].linkage != baseline &&
                modules[i].linkage != CRTLinkageType::Unknown) {
                return false;
            }
        }
        return true;
    }

    void RegisterModule(const std::string& name, CRTLinkageType linkage, bool external = false) {
        CRTModuleInfo info;
        info.moduleName = name;
        info.linkage = linkage;
        info.isExternal = external;
        m_modules.push_back(info);
    }

    uint32_t CountMismatches() const {
        if (m_modules.empty()) return 0;
        CRTLinkageType expected = DetectCurrentLinkage();
        uint32_t count = 0;
        for (const auto& mod : m_modules) {
            if (mod.linkage != expected && mod.linkage != CRTLinkageType::Unknown) {
                ++count;
            }
        }
        return count;
    }

    const std::vector<CRTModuleInfo>& GetModules() const { return m_modules; }

    bool Validate() const {
        auto report = ValidateLinkage();
        if (!report.isValid) return false;
        if (!m_modules.empty() && !CheckMismatch(m_modules)) return false;
        return true;
    }

private:
    CRTLinkageValidator() = default;
    ~CRTLinkageValidator() = default;
    CRTLinkageValidator(const CRTLinkageValidator&) = delete;
    CRTLinkageValidator& operator=(const CRTLinkageValidator&) = delete;

    std::vector<CRTModuleInfo> m_modules;
};

} // namespace Engine
} // namespace ExplorerLens
