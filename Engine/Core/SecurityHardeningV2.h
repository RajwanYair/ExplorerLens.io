#pragma once
//==============================================================================
// SecurityHardeningV2 — Sprint 219
// Advanced security hardening: code signing verification, integrity checks,
// anti-tampering, memory protection, and runtime defense.
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace DarkThumbs { namespace Engine {

enum class SecurityLevel : uint8_t {
    None     = 0,
    Basic    = 1,  // Signature check
    Standard = 2,  // Signature + integrity
    Enhanced = 3,  // + anti-tamper
    Maximum  = 4,  // + memory protection + DEP/ASLR
    LevelCount = 5
};

enum class IntegrityCheck : uint8_t {
    FileHash    = 0,
    CodeSign    = 1,
    DLLInjection = 2,
    DebugDetect  = 3,
    MemoryGuard  = 4,
    CheckCount   = 5
};

struct SecurityAuditResult {
    bool passed = false;
    SecurityLevel currentLevel = SecurityLevel::None;
    uint32_t checksRun = 0;
    uint32_t checksPassed = 0;
    uint32_t checksFailed = 0;
    double auditTimeMs = 0.0;
    std::vector<std::wstring> failures;
    std::vector<std::wstring> warnings;
};

struct IntegrityCheckResult {
    IntegrityCheck check = IntegrityCheck::FileHash;
    bool passed = false;
    std::wstring details;
};

class SecurityHardeningV2 {
public:
    SecurityHardeningV2();

    SecurityAuditResult RunAudit(SecurityLevel requiredLevel = SecurityLevel::Standard);
    IntegrityCheckResult RunCheck(IntegrityCheck check) const;

    bool VerifyCodeSignature(const std::wstring& filePath) const;
    bool VerifyFileHash(const std::wstring& filePath, const std::wstring& expectedHash) const;
    bool IsDebuggerAttached() const;
    bool IsASLREnabled() const;
    bool IsDEPEnabled() const;

    void SetSecurityLevel(SecurityLevel level) { m_level = level; }
    SecurityLevel GetSecurityLevel() const { return m_level; }

    static const wchar_t* GetLevelName(SecurityLevel level);
    static const wchar_t* GetCheckName(IntegrityCheck check);
    static uint32_t GetCheckCount() { return static_cast<uint32_t>(IntegrityCheck::CheckCount); }
    static uint32_t GetLevelCount() { return static_cast<uint32_t>(SecurityLevel::LevelCount); }

private:
    SecurityLevel m_level = SecurityLevel::Standard;
};

}} // namespace DarkThumbs::Engine
