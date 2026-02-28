// plugin_trust.h - ExplorerLens Plugin Trust and Signature Verification v1.0
//
// PURPOSE:
//   Provides signature verification, certificate chain validation, hash checking,
//   and trust decisions for third-party plugins. Implements defense-in-depth
//   security model with multiple verification layers.
//
// TRUST MODEL:
//   - Level 0: Unsigned (blocked by default in enterprise)
//   - Level 1: Self-signed (user must explicitly trust)
//   - Level 2: Code-signed by known CA (standard trust)
//   - Level 3: Signed by ExplorerLens-verified publisher (full trust)
//   - Level 4: Signed by ExplorerLens team (system trust)
//
// VERIFICATION LAYERS:
//   1. Authenticode signature validation (Windows crypto APIs)
//   2. Certificate chain validation to trusted root
//   3. Revocation checking (CRL + OCSP)
//   4. Publisher allowlist/blocklist
//   5. File hash verification against known-good database
//   6. Time-of-signing validation (not expired)
//
// Created: January 6, 2026
// Version: 1.0.0

#pragma once

#include <Windows.h>
#include <WinTrust.h>
#include <SoftPub.h>
#include <wincrypt.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <optional>

#pragma comment(lib, "wintrust.lib")
#pragma comment(lib, "crypt32.lib")

namespace ExplorerLens {
namespace PluginTrust {

// Version
constexpr uint32_t PLUGIN_TRUST_VERSION = 1;

//=============================================================================
// TRUST LEVELS
//=============================================================================

enum class TrustLevel : uint32_t {
    BLOCKED = 0,            // Explicitly blocked (revoked or malicious)
    UNTRUSTED = 1,          // Not signed or invalid signature
    SELF_SIGNED = 2,        // Self-signed certificate
    CODE_SIGNED = 3,        // Signed by recognized CA
    VERIFIED_PUBLISHER = 4, // Signed by ExplorerLens-verified publisher
    SYSTEM_TRUST = 5        // Signed by ExplorerLens team
};

inline const wchar_t* ToString(TrustLevel level) {
    switch (level) {
        case TrustLevel::BLOCKED: return L"BLOCKED";
        case TrustLevel::UNTRUSTED: return L"UNTRUSTED";
        case TrustLevel::SELF_SIGNED: return L"SELF_SIGNED";
        case TrustLevel::CODE_SIGNED: return L"CODE_SIGNED";
        case TrustLevel::VERIFIED_PUBLISHER: return L"VERIFIED_PUBLISHER";
        case TrustLevel::SYSTEM_TRUST: return L"SYSTEM_TRUST";
        default: return L"UNKNOWN";
    }
}

//=============================================================================
// SIGNATURE INFORMATION
//=============================================================================

// Certificate information
struct CertificateInfo {
    std::wstring subjectName;           // CN from certificate
    std::wstring issuerName;            // Issuing CA
    std::wstring serialNumber;          // Certificate serial number
    std::wstring thumbprint;            // SHA-256 thumbprint
    
    std::chrono::system_clock::time_point notBefore;
    std::chrono::system_clock::time_point notAfter;
    bool isExpired = false;
    bool isRevoked = false;
    
    std::vector<std::wstring> enhancedKeyUsage;  // EKU OIDs
    
    CertificateInfo() = default;
};

// Signature details
struct SignatureInfo {
    bool isSigned = false;
    bool isValid = false;
    bool isTimestamped = false;
    
    CertificateInfo signingCert;
    CertificateInfo timestampCert;
    
    std::chrono::system_clock::time_point signingTime;
    std::wstring signatureAlgorithm;    // e.g., "sha256RSA"
    std::wstring digestAlgorithm;       // e.g., "SHA256"
    
    // Authenticode verification result
    DWORD wintrustStatus = 0;
    std::wstring wintrustError;
    
    // Trust chain validation
    bool chainValidated = false;
    std::vector<CertificateInfo> chainCerts;
    
    SignatureInfo() = default;
};

//=============================================================================
// FILE HASH
//=============================================================================

// Hash algorithms
enum class HashAlgorithm : uint32_t {
    SHA256 = 0,
    SHA384 = 1,
    SHA512 = 2
};

struct FileHash {
    HashAlgorithm algorithm = HashAlgorithm::SHA256;
    std::vector<uint8_t> hash;
    std::wstring hashString;  // Hex representation
    
    bool IsValid() const { return !hash.empty(); }
    
    FileHash() = default;
};

//=============================================================================
// TRUST DECISION
//=============================================================================

// Complete trust evaluation result
struct TrustDecision {
    TrustLevel trustLevel = TrustLevel::UNTRUSTED;
    bool allowExecution = false;
    
    // Verification results
    SignatureInfo signature;
    FileHash fileHash;
    
    // Trust reasons (why this level was assigned)
    std::vector<std::wstring> trustReasons;
    std::vector<std::wstring> warnings;
    std::vector<std::wstring> errors;
    
    // Policy overrides
    bool userTrusted = false;           // User explicitly trusted
    bool policyBlocked = false;         // Policy explicitly blocked
    bool publisherAllowlisted = false;  // Publisher on allowlist
    bool publisherBlocklisted = false;  // Publisher on blocklist
    bool hashAllowlisted = false;       // Hash on allowlist
    bool hashBlocklisted = false;       // Hash on blocklist
    
    // Metadata
    std::chrono::system_clock::time_point verificationTime;
    std::chrono::milliseconds verificationDuration{0};
    
    TrustDecision() : verificationTime(std::chrono::system_clock::now()) {}
    
    // Helper: Is execution allowed?
    bool IsAllowedToExecute() const {
        return allowExecution && !policyBlocked && !publisherBlocklisted && !hashBlocklisted;
    }
};

//=============================================================================
// TRUST VERIFIER
//=============================================================================

// Configuration for trust verification
struct TrustConfig {
    // Signature verification
    bool requireSignature = true;           // Reject unsigned plugins
    bool requireTimestamp = false;          // Require valid timestamp
    bool requireChainValidation = true;     // Validate full cert chain
    
    // Revocation checking
    bool checkRevocation = true;            // Check CRL/OCSP
    bool allowRevocationFailure = false;    // Allow if revocation check fails
    
    // Trust decisions
    TrustLevel minTrustLevel = TrustLevel::CODE_SIGNED;
    bool allowSelfSigned = false;           // Allow self-signed in dev mode
    bool allowExpiredCerts = false;         // Allow expired certs (not recommended)
    
    // Hash verification
    bool verifyFileHash = true;
    HashAlgorithm hashAlgorithm = HashAlgorithm::SHA256;
    
    // Allowlists/Blocklists
    std::vector<std::wstring> allowedPublishers;    // Publisher CNs
    std::vector<std::wstring> blockedPublishers;
    std::vector<std::wstring> allowedHashes;        // Hex-encoded hashes
    std::vector<std::wstring> blockedHashes;
    
    // Trusted root certificates (thumbprints)
    std::vector<std::wstring> trustedRoots;
    
    // Performance
    std::chrono::milliseconds verificationTimeout{10000};  // 10 seconds
    bool enableCaching = true;
    std::chrono::hours cacheExpiration{24};
    
    TrustConfig() = default;
};

// Main trust verification class
class TrustVerifier {
public:
    explicit TrustVerifier(const TrustConfig& config);
    ~TrustVerifier();
    
    // Primary verification method
    TrustDecision VerifyPlugin(const std::wstring& pluginPath);
    
    // Individual verification steps (can be called separately)
    SignatureInfo VerifySignature(const std::wstring& filePath);
    FileHash ComputeFileHash(const std::wstring& filePath, HashAlgorithm algorithm);
    bool ValidateCertificateChain(PCCERT_CONTEXT pCert);
    bool CheckRevocation(PCCERT_CONTEXT pCert);
    
    // Trust level determination
    TrustLevel DetermineTrustLevel(const SignatureInfo& sigInfo, const std::wstring& filePath);
    
    // Publisher checks
    bool IsPublisherAllowlisted(const std::wstring& publisherName) const;
    bool IsPublisherBlocklisted(const std::wstring& publisherName) const;
    bool IsHashAllowlisted(const std::wstring& hashString) const;
    bool IsHashBlocklisted(const std::wstring& hashString) const;
    
    // Configuration
    const TrustConfig& GetConfig() const { return config_; }
    void SetConfig(const TrustConfig& config) { config_ = config; }
    
    // Cache management
    void ClearCache();
    size_t GetCacheSize() const;
    
private:
    // Authenticode verification
    bool VerifyAuthenticode(const std::wstring& filePath, SignatureInfo& sigInfo);
    bool ExtractSignatureInfo(WINTRUST_DATA& trustData, SignatureInfo& sigInfo);
    
    // Certificate processing
    bool ExtractCertificateInfo(PCCERT_CONTEXT pCert, CertificateInfo& certInfo);
    bool ValidateChain(PCCERT_CONTEXT pCert, std::vector<CertificateInfo>& chain);
    std::wstring GetCertificateThumbprint(PCCERT_CONTEXT pCert);
    
    // Hash computation
    bool ComputeHash(const std::wstring& filePath, HashAlgorithm algorithm, std::vector<uint8_t>& hash);
    std::wstring HashToString(const std::vector<uint8_t>& hash);
    
    // Trust decision logic
    bool ShouldAllowExecution(const TrustDecision& decision) const;
    
    // Caching
    struct CacheEntry {
        TrustDecision decision;
        std::chrono::system_clock::time_point cacheTime;
        FileHash fileHash;  // For cache invalidation
    };
    
    std::optional<TrustDecision> GetCachedDecision(const std::wstring& pluginPath);
    void CacheDecision(const std::wstring& pluginPath, const TrustDecision& decision, const FileHash& hash);
    bool IsCacheValid(const CacheEntry& entry, const FileHash& currentHash) const;
    
    TrustConfig config_;
    mutable std::mutex cacheMutex_;
    std::map<std::wstring, CacheEntry> cache_;
};

//=============================================================================
// TRUST STORE
//=============================================================================

// Persistent storage for trust decisions and allowlists/blocklists
class TrustStore {
public:
    static TrustStore& Instance();
    
    // Initialization
    bool Load(const std::wstring& storePath);
    bool Save();
    
    // Publisher management
    void AddTrustedPublisher(const std::wstring& publisherName);
    void RemoveTrustedPublisher(const std::wstring& publisherName);
    bool IsTrustedPublisher(const std::wstring& publisherName) const;
    
    void BlockPublisher(const std::wstring& publisherName);
    void UnblockPublisher(const std::wstring& publisherName);
    bool IsBlockedPublisher(const std::wstring& publisherName) const;
    
    // Hash management
    void AddTrustedHash(const std::wstring& hashString);
    void RemoveTrustedHash(const std::wstring& hashString);
    bool IsTrustedHash(const std::wstring& hashString) const;
    
    void BlockHash(const std::wstring& hashString);
    void UnblockHash(const std::wstring& hashString);
    bool IsBlockedHash(const std::wstring& hashString) const;
    
    // User trust decisions (per-plugin)
    void SetUserTrust(const std::wstring& pluginPath, bool trusted);
    bool GetUserTrust(const std::wstring& pluginPath) const;
    void ClearUserTrust(const std::wstring& pluginPath);
    
    // Certificate pinning
    void PinCertificate(const std::wstring& thumbprint);
    void UnpinCertificate(const std::wstring& thumbprint);
    bool IsCertificatePinned(const std::wstring& thumbprint) const;
    
    // Revocation list (local supplement to CRL/OCSP)
    void RevokePlugin(const std::wstring& pluginPath, const std::wstring& reason);
    bool IsRevoked(const std::wstring& pluginPath) const;
    std::wstring GetRevocationReason(const std::wstring& pluginPath) const;
    
    // Statistics
    struct TrustStats {
        size_t trustedPublishers = 0;
        size_t blockedPublishers = 0;
        size_t trustedHashes = 0;
        size_t blockedHashes = 0;
        size_t userTrustedPlugins = 0;
        size_t pinnedCertificates = 0;
        size_t revokedPlugins = 0;
    };
    
    TrustStats GetStats() const;
    
private:
    TrustStore() = default;
    ~TrustStore() = default;
    
    bool LoadFromFile(const std::wstring& path);
    bool SaveToFile(const std::wstring& path);
    
    std::wstring storePath_;
    mutable std::mutex storeMutex_;
    
    std::vector<std::wstring> trustedPublishers_;
    std::vector<std::wstring> blockedPublishers_;
    std::vector<std::wstring> trustedHashes_;
    std::vector<std::wstring> blockedHashes_;
    std::map<std::wstring, bool> userTrustDecisions_;
    std::vector<std::wstring> pinnedCertificates_;
    std::map<std::wstring, std::wstring> revokedPlugins_;
};

//=============================================================================
// TRUST POLICY
//=============================================================================

// Trust policy enforcement (can be controlled by Group Policy)
class TrustPolicy {
public:
    static TrustPolicy& Instance();
    
    // Policy configuration
    struct PolicyConfig {
        bool enforceSignature = true;           // Require all plugins signed
        bool blockUnsigned = true;              // Reject unsigned plugins
        bool blockSelfSigned = true;            // Reject self-signed
        bool requireVerifiedPublisher = false;  // Require verified publisher
        
        TrustLevel minimumTrustLevel = TrustLevel::CODE_SIGNED;
        
        bool allowUserOverride = true;          // Allow user to trust
        bool enableRevocationCheck = true;
        bool enforceTimestamp = false;
        
        // Enterprise controls
        bool enterpriseMode = false;            // Stricter rules
        bool allowDevMode = true;               // Allow self-signed in dev
        
        PolicyConfig() = default;
    };
    
    void LoadPolicy();                          // Load from registry/GP
    void SavePolicy();                          // Save to registry
    
    const PolicyConfig& GetPolicy() const { return policy_; }
    void SetPolicy(const PolicyConfig& policy);
    
    // Policy checks
    bool IsExecutionAllowed(const TrustDecision& decision) const;
    bool CanUserOverride(const TrustDecision& decision) const;
    
private:
    TrustPolicy() = default;
    ~TrustPolicy() = default;
    
    PolicyConfig policy_;
    mutable std::mutex policyMutex_;
};

//=============================================================================
// HELPER FUNCTIONS
//=============================================================================

// Convert Windows crypto status to readable string
std::wstring GetCryptoErrorString(DWORD error);

// Extract subject name from certificate
std::wstring GetCertificateSubjectName(PCCERT_CONTEXT pCert);

// Extract issuer name from certificate
std::wstring GetCertificateIssuerName(PCCERT_CONTEXT pCert);

// Check if certificate is expired
bool IsCertificateExpired(PCCERT_CONTEXT pCert);

// Get certificate validity period
void GetCertificateValidity(
    PCCERT_CONTEXT pCert,
    std::chrono::system_clock::time_point& notBefore,
    std::chrono::system_clock::time_point& notAfter
);

// Verify file hasn't been tampered with
bool VerifyFileIntegrity(const std::wstring& filePath, const FileHash& expectedHash);

} // namespace PluginTrust
} // namespace ExplorerLens

