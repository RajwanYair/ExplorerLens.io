//==============================================================================
// ExplorerLens Engine - Fast Cache Key Generator
// Copyright (c) 2026 - ExplorerLens Project
// Task B8: Optimized cache key generation
//==============================================================================

#pragma once

#include <windows.h>
#include <string>
#include <sstream>
#include <iomanip>

namespace ExplorerLens {
namespace Engine {
namespace Cache {

 /// <summary>
 /// Fast cache key generator using FNV-1a hash
 /// </summary>
 class CacheKeyGenerator {
 public:
 /// <summary>
 /// Generate cache key from file path and dimensions
 /// Format: {hash}_{width}x{height}
 /// </summary>
 static std::wstring Generate(const wchar_t* filePath, uint32_t width, uint32_t height) {
 if (!filePath) return L"";

 // Use FNV-1a hash for fast, collision-resistant hashing
 uint64_t hash = HashFNV1a(filePath);

 // Format: hash_widthxheight (e.g., "1234567890ABCDEF_256x256")
 wchar_t key[64];
 swprintf_s(key, L"%016llX_%ux%u", hash, width, height);
 return key;
 }

 /// <summary>
 /// Generate cache key with modification time
 /// Format: {hash}_{width}x{height}_{timestamp}
 /// </summary>
 static std::wstring GenerateWithTime(const wchar_t* filePath, uint32_t width, 
 uint32_t height, FILETIME modTime) {
 if (!filePath) return L"";

 uint64_t hash = HashFNV1a(filePath);
 uint64_t timeStamp = (static_cast<uint64_t>(modTime.dwHighDateTime) << 32) | 
 modTime.dwLowDateTime;

 wchar_t key[96];
 swprintf_s(key, L"%016llX_%ux%u_%016llX", hash, width, height, timeStamp);
 return key;
 }

 /// <summary>
 /// FNV-1a hash algorithm (fast, good distribution)
 /// </summary>
 static uint64_t HashFNV1a(const wchar_t* str) {
 constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;
 constexpr uint64_t FNV_PRIME = 1099511628211ULL;

 uint64_t hash = FNV_OFFSET;
 
 for (const wchar_t* p = str; *p; ++p) {
 // Hash both bytes of wchar_t for better distribution
 hash ^= static_cast<uint64_t>((*p) & 0xFF);
 hash *= FNV_PRIME;
 hash ^= static_cast<uint64_t>((*p >> 8) & 0xFF);
 hash *= FNV_PRIME;
 }
 
 return hash;
 }

 /// <summary>
 /// Validate cache key format
 /// </summary>
 static bool IsValidKey(const wchar_t* key) {
 if (!key || wcslen(key) < 20) return false;
 
 // Check format: 16 hex digits + _ + dimensions
 for (int i = 0; i < 16; ++i) {
 if (!iswxdigit(key[i])) return false;
 }
 
 if (key[16] != L'_') return false;
 
 // Find second underscore or end
 const wchar_t* p = wcschr(key + 17, L'x');
 if (!p) return false;
 
 return true;
 }

 private:
 CacheKeyGenerator() = delete;
 };

 /// <summary>
 /// Cache key with metadata
 /// </summary>
 struct CacheKeyInfo {
 std::wstring key;
 uint64_t filePathHash;
 uint32_t width;
 uint32_t height;
 uint64_t timestamp;
 
 static CacheKeyInfo Parse(const wchar_t* key) {
 CacheKeyInfo info;
 info.key = key ? key : L"";
 
 if (!CacheKeyGenerator::IsValidKey(key)) {
 return info;
 }
 
 // Parse hash (16 hex chars)
 wchar_t hashStr[17];
 wcsncpy_s(hashStr, key, 16);
 info.filePathHash = wcstoull(hashStr, nullptr, 16);
 
 // Parse dimensions
 const wchar_t* dim = wcschr(key + 17, L'_');
 if (dim) ++dim;
 else dim = key + 17;
 
 swscanf_s(dim, L"%ux%u", &info.width, &info.height);
 
 // Parse timestamp if present
 const wchar_t* time = wcschr(dim, L'_');
 if (time) {
 time++;
 info.timestamp = wcstoull(time, nullptr, 16);
 }
 
 return info;
 }
 };

} // namespace Cache
} // namespace Engine
} // namespace ExplorerLens

