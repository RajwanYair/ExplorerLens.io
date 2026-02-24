#include "FileHashEngine.h"
#include <sstream>
#include <iomanip>

namespace ExplorerLens { namespace Engine {

uint32_t FileHashEngine::s_crc32Table[256] = {};
bool FileHashEngine::s_tableInit = false;

FileHashEngine::FileHashEngine() {
    if (!s_tableInit) InitCRC32Table();
}

void FileHashEngine::InitCRC32Table() {
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (int j = 0; j < 8; ++j) {
            crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320 : crc >> 1;
        }
        s_crc32Table[i] = crc;
    }
    s_tableInit = true;
}

const wchar_t* FileHashEngine::GetAlgorithmName(HashAlgorithm algo) {
    switch (algo) {
        case HashAlgorithm::CRC32:  return L"CRC32";
        case HashAlgorithm::MD5:    return L"MD5";
        case HashAlgorithm::SHA1:   return L"SHA-1";
        case HashAlgorithm::SHA256: return L"SHA-256";
        case HashAlgorithm::SHA512: return L"SHA-512";
        default:                    return L"Unknown";
    }
}

uint32_t FileHashEngine::ComputeCRC32(const uint8_t* data, size_t size) {
    if (!s_tableInit) InitCRC32Table();
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < size; ++i) {
        crc = s_crc32Table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

std::wstring FileHashEngine::ComputeHash(const uint8_t* data, size_t size, HashAlgorithm algo) {
    if (algo == HashAlgorithm::CRC32) {
        uint32_t crc = ComputeCRC32(data, size);
        std::wstringstream ss;
        ss << std::hex << std::setw(8) << std::setfill(L'0') << crc;
        return ss.str();
    }
    // For non-CRC32, return a placeholder (real impl would use WinCrypt/BCrypt)
    std::wstringstream ss;
    uint32_t hashLen = GetHashLength(algo);
    for (uint32_t i = 0; i < hashLen; ++i) ss << L"0";
    return ss.str();
}

bool FileHashEngine::VerifyHash(const std::wstring& expected, const std::wstring& actual) {
    return expected == actual;
}

uint32_t FileHashEngine::GetHashLength(HashAlgorithm algo) {
    switch (algo) {
        case HashAlgorithm::CRC32:  return 8;
        case HashAlgorithm::MD5:    return 32;
        case HashAlgorithm::SHA1:   return 40;
        case HashAlgorithm::SHA256: return 64;
        case HashAlgorithm::SHA512: return 128;
        default:                    return 0;
    }
}

}} // namespace ExplorerLens::Engine

