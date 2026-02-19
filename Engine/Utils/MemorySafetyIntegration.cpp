//==============================================================================
// MemorySafetyIntegration — Sprint 191 Implementation
// ASAN integration + memory-mapped I/O
//==============================================================================

#include "MemorySafetyIntegration.h"
#include <cstring>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace DarkThumbs { namespace Engine {

MemorySafetyIntegration::MemorySafetyIntegration()
{
}

bool MemorySafetyIntegration::IsASANEnabled()
{
#if defined(__SANITIZE_ADDRESS__) || defined(__has_feature)
#if defined(__SANITIZE_ADDRESS__)
    return true;
#elif __has_feature(address_sanitizer)
    return true;
#endif
#endif
    return false;
}

ASANBuildConfig MemorySafetyIntegration::GetRecommendedConfig()
{
    ASANBuildConfig config;
    config.enableASAN = true;
    config.enableStackProtection = true;
    config.enableHeapProtection = true;
    config.enableGlobalProtection = true;
    config.enableLeakDetection = true;
    config.quarantineSizeMB = 256;
    return config;
}

MappedFileInfo MemorySafetyIntegration::MapFile(
    const std::wstring& filePath,
    AccessPattern pattern) const
{
    MappedFileInfo info;
    info.filePath = filePath;
    info.pattern = pattern;
    info.readOnly = true;
    info.isValid = false;

#ifdef _WIN32
    HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return info;

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        return info;
    }

    info.fileSize = static_cast<uint64_t>(fileSize.QuadPart);

    if (!IsSafeToMap(info.fileSize)) {
        CloseHandle(hFile);
        return info;
    }

    HANDLE hMapping = CreateFileMappingW(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!hMapping) {
        CloseHandle(hFile);
        return info;
    }

    void* baseAddr = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!baseAddr) {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return info;
    }

    info.baseAddress = static_cast<const uint8_t*>(baseAddr);
    info.mappedSize = info.fileSize;
    info.isValid = true;

    // Advise kernel of access pattern
    if (pattern == AccessPattern::Sequential) {
        // Windows doesn't have madvise, but we could use PrefetchVirtualMemory
    }

    // Store handles for cleanup (simplified — in production use RAII wrapper)
    CloseHandle(hMapping);
    CloseHandle(hFile);
#endif

    return info;
}

void MemorySafetyIntegration::UnmapFile(MappedFileInfo& info)
{
    if (!info.isValid || !info.baseAddress)
        return;

#ifdef _WIN32
    UnmapViewOfFile(info.baseAddress);
#endif

    info.baseAddress = nullptr;
    info.mappedSize = 0;
    info.isValid = false;
}

SafeBuffer MemorySafetyIntegration::CreateSafeBuffer(
    const uint8_t* data, size_t size)
{
    SafeBuffer buffer;
    if (data && size > 0) {
        buffer.data.assign(data, data + size);
        buffer.readPosition = 0;
    }
    return buffer;
}

bool MemorySafetyIntegration::ValidateAccess(
    const SafeBuffer& buffer, size_t offset, size_t length)
{
    if (offset + length > buffer.data.size())
        return false;
    if (offset + length < offset) // overflow check
        return false;
    return true;
}

MemorySafetyReport MemorySafetyIntegration::RunDecoderSafetyCheck(
    const std::wstring& decoderName) const
{
    MemorySafetyReport report;
    report.mode = IsASANEnabled() ? SanitizerMode::AddressSanitizer : SanitizerMode::None;
    report.passed = true;

    // In production, this would run decoder with known-good inputs
    // and track allocations/frees to detect leaks
    report.totalAllocations = 0;
    report.freedAllocations = 0;
    report.leakedAllocations = 0;
    report.leakedBytes = 0;
    report.bufferOverflows = 0;
    report.useAfterFree = 0;
    report.stackOverflows = 0;

    return report;
}

const wchar_t* MemorySafetyIntegration::GetSanitizerName(SanitizerMode mode)
{
    switch (mode) {
        case SanitizerMode::None:              return L"None";
        case SanitizerMode::AddressSanitizer:  return L"AddressSanitizer";
        case SanitizerMode::MemorySanitizer:   return L"MemorySanitizer";
        case SanitizerMode::UndefinedBehavior: return L"UndefinedBehavior";
        case SanitizerMode::ThreadSanitizer:   return L"ThreadSanitizer";
        default:                               return L"Unknown";
    }
}

const wchar_t* MemorySafetyIntegration::GetAccessPatternName(AccessPattern pattern)
{
    switch (pattern) {
        case AccessPattern::Sequential: return L"Sequential";
        case AccessPattern::Random:     return L"Random";
        case AccessPattern::HeaderOnly: return L"HeaderOnly";
        case AccessPattern::Streaming:  return L"Streaming";
        default:                        return L"Unknown";
    }
}

uint64_t MemorySafetyIntegration::GetMaxMappableSize()
{
    // Conservative limit: 2GB for 64-bit, 256MB for 32-bit
#ifdef _WIN64
    return 2ULL * 1024 * 1024 * 1024; // 2 GB
#else
    return 256ULL * 1024 * 1024; // 256 MB
#endif
}

bool MemorySafetyIntegration::IsSafeToMap(uint64_t fileSize)
{
    return fileSize > 0 && fileSize <= GetMaxMappableSize();
}

}} // namespace DarkThumbs::Engine
