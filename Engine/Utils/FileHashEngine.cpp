#include "FileHashEngine.h"
#include <iomanip>
#include <sstream>

#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0A00
#endif
#include <windows.h>

// bcrypt.h requires NTSTATUS which may be missing under WIN32_LEAN_AND_MEAN
#ifndef _NTDEF_
typedef LONG NTSTATUS;
#endif
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")

namespace ExplorerLens {
namespace Engine {

uint32_t FileHashEngine::s_crc32Table[256] = {};
bool FileHashEngine::s_tableInit = false;

FileHashEngine::FileHashEngine()
{
    if (!s_tableInit)
        InitCRC32Table();
}

void FileHashEngine::InitCRC32Table()
{
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (int j = 0; j < 8; ++j) {
            crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320 : crc >> 1;
        }
        s_crc32Table[i] = crc;
    }
    s_tableInit = true;
}

const wchar_t* FileHashEngine::GetAlgorithmName(HashAlgorithm algo)
{
    switch (algo) {
        case HashAlgorithm::CRC32:
            return L"CRC32";
        case HashAlgorithm::MD5:
            return L"MD5";
        case HashAlgorithm::SHA1:
            return L"SHA-1";
        case HashAlgorithm::SHA256:
            return L"SHA-256";
        case HashAlgorithm::SHA512:
            return L"SHA-512";
        default:
            return L"Unknown";
    }
}

uint32_t FileHashEngine::ComputeCRC32(const uint8_t* data, size_t size)
{
    if (!s_tableInit)
        InitCRC32Table();
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < size; ++i) {
        crc = s_crc32Table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

std::wstring FileHashEngine::ComputeHash(const uint8_t* data, size_t size, HashAlgorithm algo)
{
    if (algo == HashAlgorithm::CRC32) {
        uint32_t crc = ComputeCRC32(data, size);
        std::wstringstream ss;
        ss << std::hex << std::setw(8) << std::setfill(L'0') << crc;
        return ss.str();
    }

    // Map algorithm enum to BCrypt algorithm identifier
    LPCWSTR bcryptAlgo = nullptr;
    switch (algo) {
        case HashAlgorithm::MD5:
            bcryptAlgo = BCRYPT_MD5_ALGORITHM;
            break;
        case HashAlgorithm::SHA1:
            bcryptAlgo = BCRYPT_SHA1_ALGORITHM;
            break;
        case HashAlgorithm::SHA256:
            bcryptAlgo = BCRYPT_SHA256_ALGORITHM;
            break;
        case HashAlgorithm::SHA512:
            bcryptAlgo = BCRYPT_SHA512_ALGORITHM;
            break;
        default:
            return std::wstring();
    }

    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_HASH_HANDLE hHash = nullptr;
    NTSTATUS status = 0;
    std::wstring result;

    status = BCryptOpenAlgorithmProvider(&hAlg, bcryptAlgo, nullptr, 0);
    if (!BCRYPT_SUCCESS(status))
        return std::wstring();

    // Query hash length
    DWORD hashLength = 0;
    DWORD cbResult = 0;
    status = BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, reinterpret_cast<PBYTE>(&hashLength), sizeof(hashLength),
                               &cbResult, 0);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return std::wstring();
    }

    std::vector<uint8_t> hashBuffer(hashLength);

    status = BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return std::wstring();
    }

    status = BCryptHashData(hHash, const_cast<PUCHAR>(data), static_cast<ULONG>(size), 0);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return std::wstring();
    }

    status = BCryptFinishHash(hHash, hashBuffer.data(), hashLength, 0);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return std::wstring();
    }

    // Convert to hex string
    std::wstringstream ss;
    ss << std::hex << std::setfill(L'0');
    for (DWORD i = 0; i < hashLength; ++i) {
        ss << std::setw(2) << static_cast<unsigned>(hashBuffer[i]);
    }
    result = ss.str();

    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    return result;
}

bool FileHashEngine::VerifyHash(const std::wstring& expected, const std::wstring& actual)
{
    return expected == actual;
}

uint32_t FileHashEngine::GetHashLength(HashAlgorithm algo)
{
    switch (algo) {
        case HashAlgorithm::CRC32:
            return 8;
        case HashAlgorithm::MD5:
            return 32;
        case HashAlgorithm::SHA1:
            return 40;
        case HashAlgorithm::SHA256:
            return 64;
        case HashAlgorithm::SHA512:
            return 128;
        default:
            return 0;
    }
}

}  // namespace Engine
}  // namespace ExplorerLens
