// CodeIntegrityChecker.h — PE Signature and Binary Integrity Verification
// Copyright (c) 2026 ExplorerLens Project
//
// Verifies the Authenticode digital signature and SHA-256 hash of ExplorerLens
// DLL and EXE components at startup, detecting tampering before execution.
//
#pragma once
#include <softpub.h>
#include <wincrypt.h>
#include <windows.h>
#include <wintrust.h>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#pragma comment(lib, "wintrust.lib")
#pragma comment(lib, "crypt32.lib")

namespace ExplorerLens {
namespace Engine {

enum class IntegrityStatus {
    Valid,         // Signature present and trusted
    InvalidSig,    // Signature present but untrusted/expired
    NoSignature,   // File not signed
    HashMismatch,  // SHA-256 does not match pinned value
    FileNotFound,
    VerifyError
};

struct IntegrityResult
{
    std::wstring filePath;
    IntegrityStatus status = IntegrityStatus::VerifyError;
    std::wstring signerCN;    // Certificate CN if signed
    std::wstring thumbprint;  // Certificate SHA-1 thumbprint
    bool revoked = false;
    bool ok() const
    {
        return status == IntegrityStatus::Valid;
    }
};

// Optional: known-good SHA-256 hashes for pinned verification
struct PinnedHash
{
    std::wstring filePath;
    std::string sha256Hex;  // lowercase hex
};

class CodeIntegrityChecker
{
  public:
    // Verify Authenticode signature using WinVerifyTrust
    IntegrityResult VerifySignature(const std::wstring& path) const
    {
        IntegrityResult res;
        res.filePath = path;

        if (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
            res.status = IntegrityStatus::FileNotFound;
            return res;
        }

        WINTRUST_FILE_INFO fileInfo = {};
        fileInfo.cbStruct = sizeof(fileInfo);
        fileInfo.pcwszFilePath = path.c_str();

        GUID action = WINTRUST_ACTION_GENERIC_VERIFY_V2;
        WINTRUST_DATA wtd = {};
        wtd.cbStruct = sizeof(wtd);
        wtd.dwUIChoice = WTD_UI_NONE;
        wtd.fdwRevocationChecks = WTD_REVOKE_WHOLECHAIN;
        wtd.dwUnionChoice = WTD_CHOICE_FILE;
        wtd.pFile = &fileInfo;
        wtd.dwProvFlags = WTD_CACHE_ONLY_URL_RETRIEVAL;
        wtd.dwStateAction = WTD_STATEACTION_VERIFY;

        LONG result = WinVerifyTrust(nullptr, &action, &wtd);
        wtd.dwStateAction = WTD_STATEACTION_CLOSE;
        WinVerifyTrust(nullptr, &action, &wtd);

        switch (result) {
            case ERROR_SUCCESS:
                res.status = IntegrityStatus::Valid;
                break;
            case TRUST_E_NOSIGNATURE:
            case CERT_E_UNTRUSTEDROOT:
                res.status = IntegrityStatus::NoSignature;
                break;
            case TRUST_E_EXPLICIT_DISTRUST:
            case CERT_E_EXPIRED:
                res.status = IntegrityStatus::InvalidSig;
                break;
            default:
                res.status = IntegrityStatus::VerifyError;
                break;
        }
        if (res.status == IntegrityStatus::Valid)
            ExtractSignerInfo(path, res);
        return res;
    }

    // Verify SHA-256 hash against a pinned value
    bool VerifyHash(const std::wstring& path, const std::string& expectedHex, std::string& actualHex) const
    {
        actualHex = ComputeSHA256(path);
        return !actualHex.empty() && actualHex == expectedHex;
    }

    // Verify all pinned hashes at once
    std::vector<IntegrityResult> VerifyAll(const std::vector<std::wstring>& paths,
                                           const std::vector<PinnedHash>& pins = {}) const
    {
        std::vector<IntegrityResult> results;
        for (const auto& p : paths) {
            IntegrityResult res = VerifySignature(p);
            // If pinned hash provided, also check hash
            for (const auto& pin : pins) {
                if (pin.filePath == p) {
                    std::string actual;
                    if (!VerifyHash(p, pin.sha256Hex, actual))
                        res.status = IntegrityStatus::HashMismatch;
                    break;
                }
            }
            results.push_back(res);
        }
        return results;
    }

    // Check the running DLL itself
    IntegrityResult VerifySelf() const
    {
        wchar_t path[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        return VerifySignature(path);
    }

  private:
    void ExtractSignerInfo(const std::wstring& path, IntegrityResult& res) const
    {
        HCERTSTORE hStore = nullptr;
        HCRYPTMSG hMsg = nullptr;
        DWORD encode = 0;
        DWORD content = 0;
        DWORD format = 0;

        if (!CryptQueryObject(CERT_QUERY_OBJECT_FILE, path.c_str(), CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
                              CERT_QUERY_FORMAT_FLAG_BINARY, 0, &encode, &content, &format, &hStore, &hMsg, nullptr))
            return;

        DWORD sz = 0;
        CryptMsgGetParam(hMsg, CMSG_SIGNER_CERT_INFO_PARAM, 0, nullptr, &sz);
        if (sz > 0 && sz < 65536) {
            std::vector<BYTE> buf(sz);
            if (CryptMsgGetParam(hMsg, CMSG_SIGNER_CERT_INFO_PARAM, 0, buf.data(), &sz)) {
                auto* ci = reinterpret_cast<CERT_INFO*>(buf.data());
                PCCERT_CONTEXT ctx = CertFindCertificateInStore(hStore, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0,
                                                                CERT_FIND_SUBJECT_CERT, ci, nullptr);
                if (ctx) {
                    wchar_t cn[256] = {};
                    CertGetNameStringW(ctx, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, nullptr, cn, 256);
                    res.signerCN = cn;
                    CertFreeCertificateContext(ctx);
                }
            }
        }
        if (hMsg)
            CryptMsgClose(hMsg);
        if (hStore)
            CertCloseStore(hStore, 0);
    }

    static std::string ComputeSHA256(const std::wstring& path)
    {
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                   FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
            return {};

        HCRYPTPROV hProv = 0;
        HCRYPTHASH hHash = 0;
        if (!CryptAcquireContextW(&hProv, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
            CloseHandle(hFile);
            return {};
        }
        if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
            CryptReleaseContext(hProv, 0);
            CloseHandle(hFile);
            return {};
        }
        uint8_t buf[65536];
        DWORD read = 0;
        while (ReadFile(hFile, buf, sizeof(buf), &read, nullptr) && read)
            CryptHashData(hHash, buf, read, 0);
        CloseHandle(hFile);

        std::string hex;
        DWORD sz = 32;
        uint8_t hash[32] = {};
        if (CryptGetHashParam(hHash, HP_HASHVAL, hash, &sz, 0)) {
            static const char* const hexChr = "0123456789abcdef";
            for (int i = 0; i < 32; ++i) {
                hex += hexChr[hash[i] >> 4];
                hex += hexChr[hash[i] & 0xF];
            }
        }
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return hex;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
