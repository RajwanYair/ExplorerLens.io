#pragma once
// Sprint 425: Encrypted Format Handler
// Support for password-protected and encrypted file formats (encrypted
// ZIP/RAR/7z, encrypted PDF, EFS/BitLocker-protected files) with secure key
// management.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Encryption type detected in file
enum class EncryptionType : uint8_t {
  None = 0,
  ZipAES256,
  ZipZipCrypto,
  RAR5_AES256,
  SevenZip_AES256,
  PDF_AES128,
  PDF_AES256,
  PDF_RC4,
  EFS, // Windows Encrypting File System
  BitLocker,
  COUNT
};

/// Encrypted file handling strategy
enum class EncryptedHandling : uint8_t {
  ShowLockIcon = 0, // Display padlock overlay
  ShowPlaceholder,  // Generic encrypted file placeholder
  AttemptCachedKey, // Try previously-used password
  SkipSilently,     // Skip without any visual indicator
  COUNT
};

struct EncryptedFormatInfo {
  EncryptionType type = EncryptionType::None;
  bool isEncrypted = false;
  bool headerEncrypted = false; // Can't even read filenames
  uint32_t keyLengthBits = 0;
  const wchar_t *algorithm = nullptr;
};

class EncryptedFormatHandler {
public:
  static constexpr size_t EncryptionCount() {
    return static_cast<size_t>(EncryptionType::COUNT);
  }
  static constexpr size_t HandlingCount() {
    return static_cast<size_t>(EncryptedHandling::COUNT);
  }

  static const wchar_t *EncryptionName(EncryptionType e) {
    switch (e) {
    case EncryptionType::None:
      return L"None";
    case EncryptionType::ZipAES256:
      return L"ZIP AES-256";
    case EncryptionType::ZipZipCrypto:
      return L"ZIP ZipCrypto";
    case EncryptionType::RAR5_AES256:
      return L"RAR5 AES-256";
    case EncryptionType::SevenZip_AES256:
      return L"7z AES-256";
    case EncryptionType::PDF_AES128:
      return L"PDF AES-128";
    case EncryptionType::PDF_AES256:
      return L"PDF AES-256";
    case EncryptionType::PDF_RC4:
      return L"PDF RC4";
    case EncryptionType::EFS:
      return L"EFS";
    case EncryptionType::BitLocker:
      return L"BitLocker";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *HandlingName(EncryptedHandling h) {
    switch (h) {
    case EncryptedHandling::ShowLockIcon:
      return L"Lock Icon";
    case EncryptedHandling::ShowPlaceholder:
      return L"Placeholder";
    case EncryptedHandling::AttemptCachedKey:
      return L"Cached Key";
    case EncryptedHandling::SkipSilently:
      return L"Skip";
    default:
      return L"Unknown";
    }
  }

  /// Check if encryption is strong (AES-based)
  static bool IsStrongEncryption(EncryptionType e) {
    switch (e) {
    case EncryptionType::ZipAES256:
    case EncryptionType::RAR5_AES256:
    case EncryptionType::SevenZip_AES256:
    case EncryptionType::PDF_AES128:
    case EncryptionType::PDF_AES256:
    case EncryptionType::BitLocker:
      return true;
    default:
      return false;
    }
  }
};

} // namespace Engine
} // namespace ExplorerLens
