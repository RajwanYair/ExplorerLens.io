#pragma once
// File Hash Engine — SHA256/MD5/CRC32 file hashing for integrity verification
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// Hash algorithm type
enum class HashAlgorithm : uint32_t {
    CRC32   = 0,
    MD5     = 1,
    SHA1    = 2,
    SHA256  = 3,
    SHA512  = 4,
    COUNT   = 5
};

/// Result of a hash operation
struct HashResult {
    HashAlgorithm algorithm = HashAlgorithm::SHA256;
    std::wstring  hashValue;
    std::wstring  filePath;
    uint64_t      fileSize = 0;
    double        elapsedMs = 0.0;
    bool          success  = false;
};

/// File hashing engine for integrity verification
class FileHashEngine {
public:
    FileHashEngine();

    static const wchar_t* GetAlgorithmName(HashAlgorithm algo);
    static uint32_t GetAlgorithmCount() { return static_cast<uint32_t>(HashAlgorithm::COUNT); }

    /// Compute hash of data buffer
    static std::wstring ComputeHash(const uint8_t* data, size_t size, HashAlgorithm algo);
    /// Compute simple CRC32
    static uint32_t ComputeCRC32(const uint8_t* data, size_t size);
    /// Verify a hash matches expected value
    static bool VerifyHash(const std::wstring& expected, const std::wstring& actual);
    /// Get hash output length in hex characters
    static uint32_t GetHashLength(HashAlgorithm algo);

private:
    static uint32_t s_crc32Table[256];
    static bool s_tableInit;
    static void InitCRC32Table();
};

}} // namespace ExplorerLens::Engine

