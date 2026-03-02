//==============================================================================
// ExplorerLens Engine — Plugin Trust Chain Validator
//
// Validates Authenticode signatures and certificate chains on plugin DLLs.
// Uses dynamically-loaded wintrust.dll for WinVerifyTrust and crypt32.dll
// for CryptQueryObject/CryptMsgGetParam to extract signer certificate info.
// Avoids including <wintrust.h> (conflicts with WIN32_LEAN_AND_MEAN) by
// defining the necessary structures inline.
//
// Supports trust levels from Untrusted to MicrosoftSigned, with a
// configurable minimum policy and per-thumbprint trusted publisher list.
//
// Thread-safe with SRWLOCK. Header-only, C++20, MSVC /W4 clean.
//==============================================================================
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <wincrypt.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <cstdint>
#include <chrono>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "crypt32.lib")

namespace ExplorerLens {
namespace Engine {

// ──────────────────────────────────────────────────────────────────────────────
// Minimal WinTrust structures defined inline (avoids <wintrust.h> dependency)
// ──────────────────────────────────────────────────────────────────────────────
#pragma pack(push, 8)
struct WINTRUST_FILE_INFO_INLINE {
    DWORD    cbStruct;
    LPCWSTR  pcwszFilePath;
    HANDLE   hFile;
    GUID* pgKnownSubject;
};

struct WINTRUST_DATA_INLINE {
    DWORD   cbStruct;
    LPVOID  pPolicyCallbackData;
    LPVOID  pSIPClientData;
    DWORD   dwUIChoice;
    DWORD   fdwRevocationChecks;
    DWORD   dwUnionChoice;
    union {
        WINTRUST_FILE_INFO_INLINE* pFile;
        LPVOID pCatalog;
        LPVOID pBlob;
        LPVOID pSgnr;
        LPVOID pCert;
    };
    DWORD   dwStateAction;
    HANDLE  hWVTStateData;
    WCHAR* pwszURLReference;
    DWORD   dwProvFlags;
    DWORD   dwUIContext;
    LPVOID  pSignatureSettings;
};
#pragma pack(pop)

// WinTrust constants
static const GUID WINTRUST_ACTION_GENERIC_VERIFY_V2_INLINE =
{ 0xaac56b, 0xcd44, 0x11d0, { 0x8c, 0xc2, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee } };

static constexpr DWORD WTD_UI_NONE_VAL = 2;
static constexpr DWORD WTD_REVOKE_NONE_VAL = 0;
static constexpr DWORD WTD_CHOICE_FILE_VAL = 1;
static constexpr DWORD WTD_STATEACTION_VERIFY_VAL = 1;
static constexpr DWORD WTD_STATEACTION_CLOSE_VAL = 2;

// ──────────────────────────────────────────────────────────────────────────────

enum class TrustLevel : uint32_t {
    Untrusted = 0,
    SelfSigned = 1,
    ValidSignature = 2,
    TrustedPublisher = 3,
    MicrosoftSigned = 4
};

struct TrustChainCertificateInfo {
    std::wstring subject;
    std::wstring issuer;
    std::wstring serial;
    std::wstring thumbprint;
    FILETIME     validFrom{};
    FILETIME     validTo{};
    bool         isExpired = false;
};

struct TrustStats {
    uint64_t pluginsValidated = 0;
    uint64_t untrustedCount = 0;
    uint64_t selfSignedCount = 0;
    uint64_t validSignatureCount = 0;
    uint64_t trustedPublisherCount = 0;
    uint64_t microsoftSignedCount = 0;
    uint64_t rejectedCount = 0;
};

class PluginTrustChainValidator {
public:
    PluginTrustChainValidator() {
        InitializeSRWLock(&m_lock);
        LoadApis();
    }

    ~PluginTrustChainValidator() {
        if (m_wintrustModule) FreeLibrary(m_wintrustModule);
        // crypt32 is loaded via pragma comment(lib), no need to free
    }

    PluginTrustChainValidator(const PluginTrustChainValidator&) = delete;
    PluginTrustChainValidator& operator=(const PluginTrustChainValidator&) = delete;

    inline TrustLevel ValidateSignature(const std::wstring& dllPath) {
        AcquireSRWLockExclusive(&m_lock);
        m_stats.pluginsValidated++;
        ReleaseSRWLockExclusive(&m_lock);

        TrustLevel level = TrustLevel::Untrusted;

        if (!m_fnWinVerifyTrust) {
            RecordTrustLevel(level);
            return level;
        }

        // Prepare WinVerifyTrust call
        WINTRUST_FILE_INFO_INLINE fileInfo{};
        fileInfo.cbStruct = sizeof(fileInfo);
        fileInfo.pcwszFilePath = dllPath.c_str();
        fileInfo.hFile = nullptr;
        fileInfo.pgKnownSubject = nullptr;

        WINTRUST_DATA_INLINE wtd{};
        memset(&wtd, 0, sizeof(wtd));
        wtd.cbStruct = sizeof(wtd);
        wtd.dwUIChoice = WTD_UI_NONE_VAL;
        wtd.fdwRevocationChecks = WTD_REVOKE_NONE_VAL;
        wtd.dwUnionChoice = WTD_CHOICE_FILE_VAL;
        wtd.pFile = &fileInfo;
        wtd.dwStateAction = WTD_STATEACTION_VERIFY_VAL;
        wtd.hWVTStateData = nullptr;
        wtd.dwProvFlags = 0;

        using WinVerifyTrustFn = LONG(WINAPI*)(HWND, GUID*, LPVOID);
        auto fnVerify = reinterpret_cast<WinVerifyTrustFn>(m_fnWinVerifyTrust);

        GUID action = WINTRUST_ACTION_GENERIC_VERIFY_V2_INLINE;
        LONG status = fnVerify(
            static_cast<HWND>(INVALID_HANDLE_VALUE),
            &action,
            &wtd);

        if (status == 0) {
            // Signature is valid
            level = TrustLevel::ValidSignature;

            // Check certificate info for trust level upgrade
            TrustChainCertificateInfo certInfo = GetSignerInfo(dllPath);
            if (!certInfo.thumbprint.empty()) {
                // Check Microsoft signature
                std::wstring lowerIssuer = certInfo.issuer;
                for (auto& ch : lowerIssuer) ch = static_cast<wchar_t>(towlower(ch));
                if (lowerIssuer.find(L"microsoft") != std::wstring::npos) {
                    level = TrustLevel::MicrosoftSigned;
                }
                else if (IsTrustedPublisher(certInfo.thumbprint)) {
                    level = TrustLevel::TrustedPublisher;
                }
            }
        }
        else if (status == static_cast<LONG>(0x800B0100L)) {
            // TRUST_E_NOSIGNATURE — no signature found
            level = TrustLevel::Untrusted;
        }
        else if (status == static_cast<LONG>(0x800B0101L)) {
            // CERT_E_EXPIRED
            level = TrustLevel::SelfSigned; // Expired but was signed
        }
        else if (status == static_cast<LONG>(0x800B010AL)) {
            // CERT_E_CHAINING — self-signed or untrusted root
            level = TrustLevel::SelfSigned;
        }

        // Close state
        wtd.dwStateAction = WTD_STATEACTION_CLOSE_VAL;
        fnVerify(static_cast<HWND>(INVALID_HANDLE_VALUE), &action, &wtd);

        RecordTrustLevel(level);
        return level;
    }

    inline TrustChainCertificateInfo GetSignerInfo(const std::wstring& dllPath) {
        TrustChainCertificateInfo info;

        // Use CryptQueryObject to get the signer certificate from the file
        DWORD dwEncoding = 0, dwContentType = 0, dwFormatType = 0;
        HCERTSTORE hStore = nullptr;
        HCRYPTMSG  hMsg = nullptr;

        BOOL ok = CryptQueryObject(
            CERT_QUERY_OBJECT_FILE,
            dllPath.c_str(),
            CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
            CERT_QUERY_FORMAT_FLAG_BINARY,
            0, &dwEncoding, &dwContentType, &dwFormatType,
            &hStore, &hMsg, nullptr);

        if (!ok || !hMsg || !hStore) {
            if (hMsg)   CryptMsgClose(hMsg);
            if (hStore) CertCloseStore(hStore, 0);
            return info;
        }

        // Get signer info size
        DWORD signerInfoSize = 0;
        if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, nullptr, &signerInfoSize) ||
            signerInfoSize == 0) {
            CryptMsgClose(hMsg);
            CertCloseStore(hStore, 0);
            return info;
        }

        std::vector<uint8_t> signerInfoBuf(signerInfoSize);
        if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0,
            signerInfoBuf.data(), &signerInfoSize)) {
            CryptMsgClose(hMsg);
            CertCloseStore(hStore, 0);
            return info;
        }

        auto* signerInfo = reinterpret_cast<CMSG_SIGNER_INFO*>(signerInfoBuf.data());

        // Find the signer certificate in the store
        CERT_INFO certSearchInfo{};
        certSearchInfo.Issuer = signerInfo->Issuer;
        certSearchInfo.SerialNumber = signerInfo->SerialNumber;

        PCCERT_CONTEXT pCert = CertFindCertificateInStore(
            hStore, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            0, CERT_FIND_SUBJECT_CERT,
            &certSearchInfo, nullptr);

        if (pCert) {
            // Extract subject name
            wchar_t nameBuf[512]{};
            DWORD nameLen = CertGetNameStringW(pCert, CERT_NAME_SIMPLE_DISPLAY_TYPE,
                0, nullptr, nameBuf, 512);
            if (nameLen > 1) {
                info.subject.assign(nameBuf, nameLen - 1);
            }

            // Extract issuer name
            nameLen = CertGetNameStringW(pCert, CERT_NAME_SIMPLE_DISPLAY_TYPE,
                CERT_NAME_ISSUER_FLAG, nullptr, nameBuf, 512);
            if (nameLen > 1) {
                info.issuer.assign(nameBuf, nameLen - 1);
            }

            // Extract serial number
            info.serial = BlobToHexString(
                pCert->pCertInfo->SerialNumber.pbData,
                pCert->pCertInfo->SerialNumber.cbData);

            // Compute SHA-1 thumbprint
            uint8_t thumbHash[20]{};
            DWORD thumbSize = sizeof(thumbHash);
            if (CertGetCertificateContextProperty(pCert,
                CERT_HASH_PROP_ID, thumbHash, &thumbSize)) {
                info.thumbprint = BlobToHexString(thumbHash, thumbSize);
            }

            // Validity dates
            info.validFrom = pCert->pCertInfo->NotBefore;
            info.validTo = pCert->pCertInfo->NotAfter;

            // Check expiration
            FILETIME now{};
            GetSystemTimeAsFileTime(&now);
            info.isExpired = (CompareFileTime(&now, &info.validTo) > 0);

            CertFreeCertificateContext(pCert);
        }

        CryptMsgClose(hMsg);
        CertCloseStore(hStore, 0);

        return info;
    }

    inline void AddTrustedPublisher(const std::wstring& thumbprint) {
        std::wstring lower = NormalizeThumbprint(thumbprint);
        AcquireSRWLockExclusive(&m_lock);
        m_trustedPublishers.insert(lower);
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline void RemoveTrustedPublisher(const std::wstring& thumbprint) {
        std::wstring lower = NormalizeThumbprint(thumbprint);
        AcquireSRWLockExclusive(&m_lock);
        m_trustedPublishers.erase(lower);
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline bool IsTrustedPublisher(const std::wstring& thumbprint) const {
        std::wstring lower = NormalizeThumbprint(thumbprint);
        AcquireSRWLockShared(&m_lock);
        bool found = m_trustedPublishers.count(lower) > 0;
        ReleaseSRWLockShared(&m_lock);
        return found;
    }

    inline void SetPolicy(TrustLevel minimumLevel) {
        AcquireSRWLockExclusive(&m_lock);
        m_minimumTrustLevel = minimumLevel;
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline TrustLevel GetPolicy() const {
        AcquireSRWLockShared(&m_lock);
        TrustLevel lvl = m_minimumTrustLevel;
        ReleaseSRWLockShared(&m_lock);
        return lvl;
    }

    inline bool MeetsPolicy(TrustLevel level) const {
        AcquireSRWLockShared(&m_lock);
        bool ok = static_cast<uint32_t>(level) >= static_cast<uint32_t>(m_minimumTrustLevel);
        ReleaseSRWLockShared(&m_lock);
        return ok;
    }

    inline TrustStats GetStats() const {
        AcquireSRWLockShared(&m_lock);
        TrustStats s = m_stats;
        ReleaseSRWLockShared(&m_lock);
        return s;
    }

private:
    inline void LoadApis() {
        m_wintrustModule = LoadLibraryW(L"wintrust.dll");
        if (m_wintrustModule) {
            m_fnWinVerifyTrust = reinterpret_cast<void*>(
                GetProcAddress(m_wintrustModule, "WinVerifyTrust"));
        }
    }

    inline void RecordTrustLevel(TrustLevel level) {
        AcquireSRWLockExclusive(&m_lock);
        switch (level) {
        case TrustLevel::Untrusted:        m_stats.untrustedCount++; break;
        case TrustLevel::SelfSigned:       m_stats.selfSignedCount++; break;
        case TrustLevel::ValidSignature:   m_stats.validSignatureCount++; break;
        case TrustLevel::TrustedPublisher: m_stats.trustedPublisherCount++; break;
        case TrustLevel::MicrosoftSigned:  m_stats.microsoftSignedCount++; break;
        }
        if (static_cast<uint32_t>(level) < static_cast<uint32_t>(m_minimumTrustLevel)) {
            m_stats.rejectedCount++;
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    static inline std::wstring NormalizeThumbprint(const std::wstring& tp) {
        std::wstring result;
        result.reserve(tp.size());
        for (wchar_t ch : tp) {
            if (ch != L' ' && ch != L':') {
                result += static_cast<wchar_t>(towlower(ch));
            }
        }
        return result;
    }

    static inline std::wstring BlobToHexString(const uint8_t* data, DWORD size) {
        std::wostringstream oss;
        for (DWORD i = 0; i < size; ++i) {
            oss << std::hex << std::setw(2) << std::setfill(L'0')
                << static_cast<unsigned>(data[i]);
        }
        return oss.str();
    }

    // Members
    mutable SRWLOCK           m_lock{};
    HMODULE                   m_wintrustModule = nullptr;
    void* m_fnWinVerifyTrust = nullptr;
    TrustLevel                m_minimumTrustLevel = TrustLevel::Untrusted;
    std::unordered_set<std::wstring> m_trustedPublishers;
    TrustStats                m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
