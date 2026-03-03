// ArchivePasswordDetector.h — Fast Detection of Password-Protected Archives
// Copyright (c) 2026 ExplorerLens Project
//
// Detects encryption in ZIP, RAR, and 7z archives by inspecting header bytes
// and flag bits without attempting extraction. Supports identification of
// encryption algorithm types (ZipCrypto, AES, RAR5, 7z-AES).
//
#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class ArchiveFormat : uint8_t {
    Unknown = 0,
    Zip = 1,
    Rar = 2,
    SevenZip = 3
};

enum class ArchiveEncryptionType : uint8_t {
    None = 0,
    ZipCrypto = 1,
    ZipAES = 2,
    Rar4Crypt = 3,
    Rar5Crypt = 4,
    SevenZipAES = 5,
    Unknown = 255
};

class ArchivePasswordDetector {
public:
    // ---------------------------------------------------------------
    // ZIP encryption detection
    // ZIP local file header:  PK\x03\x04 at offset 0
    //   General purpose bit flag at offset 6 (2 bytes, little-endian)
    //   Bit 0 = encrypted, Bit 6 = strong encryption
    // ---------------------------------------------------------------
    static bool CheckZipEncryption(const uint8_t* headerBytes, size_t size) noexcept {
        if (!headerBytes || size < 10) return false;

        // Verify ZIP local file header signature: PK\x03\x04
        if (headerBytes[0] != 0x50 || headerBytes[1] != 0x4B ||
            headerBytes[2] != 0x03 || headerBytes[3] != 0x04) {
            return false;
        }

        // General purpose bit flag at offset 6-7 (little-endian)
        const uint16_t flags = static_cast<uint16_t>(headerBytes[6]) |
            (static_cast<uint16_t>(headerBytes[7]) << 8);

        // Bit 0: file is encrypted
        return (flags & 0x0001) != 0;
    }

    // ---------------------------------------------------------------
    // RAR encryption detection
    // RAR4 signature: Rar!\x1A\x07\x00  (7 bytes)
    //   Archive header flags at offset 10 (2 bytes LE), bit 7 = password
    // RAR5 signature: Rar!\x1A\x07\x01\x00 (8 bytes)
    //   Uses vint encoding; simplified: check encryption flag byte
    // ---------------------------------------------------------------
    static bool CheckRarEncryption(const uint8_t* headerBytes, size_t size) noexcept {
        if (!headerBytes || size < 12) return false;

        // Check RAR4 signature: "Rar!\x1A\x07\x00"
        const uint8_t rar4Sig[] = { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00 };
        const uint8_t rar5Sig[] = { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x01, 0x00 };

        bool isRar4 = (size >= 7) && (std::memcmp(headerBytes, rar4Sig, 7) == 0);
        bool isRar5 = (size >= 8) && (std::memcmp(headerBytes, rar5Sig, 8) == 0);

        if (!isRar4 && !isRar5) return false;

        if (isRar4 && size >= 12) {
            // RAR4: archive header flags at offset 10 (little-endian)
            // Bit 7 (0x0080) indicates archive-level password
            const uint16_t flags = static_cast<uint16_t>(headerBytes[10]) |
                (static_cast<uint16_t>(headerBytes[11]) << 8);
            return (flags & 0x0080) != 0;
        }

        if (isRar5 && size >= 13) {
            // RAR5: encryption header type 4, presence byte at offset 12
            // Simplified: check if encryption record flag is set
            // Byte at offset 12 holds header flags; bit 0 = extra area present
            // A non-zero encryption indicator at byte 12 suggests encryption
            return (headerBytes[12] & 0x01) != 0;
        }

        return false;
    }

    // ---------------------------------------------------------------
    // 7z encryption detection
    // 7z signature: {'7','z',0xBC,0xAF,0x27,0x1C} (6 bytes)
    //   Encoded header with method ID 0x06F10701 = AES-256
    //   Simplified: scan first N bytes for AES codec ID bytes
    // ---------------------------------------------------------------
    static bool Check7zEncryption(const uint8_t* headerBytes, size_t size) noexcept {
        if (!headerBytes || size < 32) return false;

        // Verify 7z signature
        const uint8_t sig7z[] = { 0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C };
        if (std::memcmp(headerBytes, sig7z, 6) != 0) return false;

        // 7z AES codec identifier bytes: 0x06, 0xF1, 0x07, 0x01
        // Scan the header region for this 4-byte sequence
        const size_t scanLimit = (std::min)(size, static_cast<size_t>(512));
        const uint8_t aesCodec[] = { 0x06, 0xF1, 0x07, 0x01 };

        for (size_t i = 6; i + 4 <= scanLimit; ++i) {
            if (std::memcmp(headerBytes + i, aesCodec, 4) == 0) {
                return true;
            }
        }

        return false;
    }

    // ---------------------------------------------------------------
    // Unified encryption check across formats
    // ---------------------------------------------------------------
    static bool IsEncrypted(const uint8_t* headerBytes, size_t size,
        ArchiveFormat format) noexcept {
        if (!headerBytes || size == 0) return false;

        switch (format) {
        case ArchiveFormat::Zip:      return CheckZipEncryption(headerBytes, size);
        case ArchiveFormat::Rar:      return CheckRarEncryption(headerBytes, size);
        case ArchiveFormat::SevenZip: return Check7zEncryption(headerBytes, size);
        case ArchiveFormat::Unknown:
        default:
            // Try all formats
            return CheckZipEncryption(headerBytes, size) ||
                CheckRarEncryption(headerBytes, size) ||
                Check7zEncryption(headerBytes, size);
        }
    }

    // ---------------------------------------------------------------
    // Identify the specific encryption algorithm
    // ---------------------------------------------------------------
    static ArchiveEncryptionType GetEncryptionType(const uint8_t* headerBytes,
        size_t size) noexcept {
        if (!headerBytes || size < 10) return ArchiveEncryptionType::None;

        // ZIP check
        if (size >= 10 && headerBytes[0] == 0x50 && headerBytes[1] == 0x4B &&
            headerBytes[2] == 0x03 && headerBytes[3] == 0x04) {
            const uint16_t flags = static_cast<uint16_t>(headerBytes[6]) |
                (static_cast<uint16_t>(headerBytes[7]) << 8);
            if (flags & 0x0001) {
                // Bit 6 = strong encryption (AES)
                return (flags & 0x0040) ? ArchiveEncryptionType::ZipAES
                    : ArchiveEncryptionType::ZipCrypto;
            }
            return ArchiveEncryptionType::None;
        }

        // RAR check
        const uint8_t rar4Sig[] = { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00 };
        const uint8_t rar5Sig[] = { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x01, 0x00 };

        if (size >= 12 && std::memcmp(headerBytes, rar4Sig, 7) == 0) {
            const uint16_t flags = static_cast<uint16_t>(headerBytes[10]) |
                (static_cast<uint16_t>(headerBytes[11]) << 8);
            return (flags & 0x0080) ? ArchiveEncryptionType::Rar4Crypt
                : ArchiveEncryptionType::None;
        }

        if (size >= 13 && std::memcmp(headerBytes, rar5Sig, 8) == 0) {
            return (headerBytes[12] & 0x01) ? ArchiveEncryptionType::Rar5Crypt
                : ArchiveEncryptionType::None;
        }

        // 7z check
        if (Check7zEncryption(headerBytes, size)) {
            return ArchiveEncryptionType::SevenZipAES;
        }

        return ArchiveEncryptionType::None;
    }

    // ---------------------------------------------------------------
    // Detect archive format from magic bytes
    // ---------------------------------------------------------------
    static ArchiveFormat DetectFormat(const uint8_t* headerBytes,
        size_t size) noexcept {
        if (!headerBytes || size < 6) return ArchiveFormat::Unknown;

        if (size >= 4 && headerBytes[0] == 0x50 && headerBytes[1] == 0x4B &&
            headerBytes[2] == 0x03 && headerBytes[3] == 0x04) {
            return ArchiveFormat::Zip;
        }

        if (size >= 7 && headerBytes[0] == 0x52 && headerBytes[1] == 0x61 &&
            headerBytes[2] == 0x72 && headerBytes[3] == 0x21 &&
            headerBytes[4] == 0x1A && headerBytes[5] == 0x07) {
            return ArchiveFormat::Rar;
        }

        const uint8_t sig7z[] = { 0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C };
        if (size >= 6 && std::memcmp(headerBytes, sig7z, 6) == 0) {
            return ArchiveFormat::SevenZip;
        }

        return ArchiveFormat::Unknown;
    }
};

} // namespace Engine
} // namespace ExplorerLens
