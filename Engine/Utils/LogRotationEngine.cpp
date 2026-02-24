#include "LogRotationEngine.h"
#include <chrono>

namespace ExplorerLens { namespace Engine {

LogRotationEngine::LogRotationEngine() = default;

const wchar_t* LogRotationEngine::GetPolicyName(RotationPolicy policy) {
    switch (policy) {
        case RotationPolicy::SizeBased:  return L"Size Based";
        case RotationPolicy::TimeBased:  return L"Time Based";
        case RotationPolicy::CountBased: return L"Count Based";
        case RotationPolicy::Hybrid:     return L"Hybrid";
        default:                         return L"Unknown";
    }
}

const wchar_t* LogRotationEngine::GetCompressionName(LogCompression comp) {
    switch (comp) {
        case LogCompression::None: return L"None";
        case LogCompression::GZip: return L"GZip";
        case LogCompression::Zstd: return L"Zstd";
        case LogCompression::LZ4:  return L"LZ4";
        default:                   return L"Unknown";
    }
}

bool LogRotationEngine::NeedsRotation(uint64_t currentSizeBytes) const {
    if (m_config.policy == RotationPolicy::SizeBased || m_config.policy == RotationPolicy::Hybrid) {
        return currentSizeBytes >= m_config.maxSizeBytes;
    }
    return false;
}

void LogRotationEngine::AddRotatedFile(const std::wstring& path, uint64_t size) {
    RotatedLogFile file;
    file.path = path;
    file.sizeBytes = size;
    auto now = std::chrono::system_clock::now();
    file.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    file.compressed = (m_config.compression != LogCompression::None);
    m_rotatedFiles.push_back(std::move(file));
}

std::vector<std::wstring> LogRotationEngine::GetFilesToCleanup() const {
    std::vector<std::wstring> cleanup;
    if (m_rotatedFiles.size() > m_config.maxFiles) {
        size_t excess = m_rotatedFiles.size() - m_config.maxFiles;
        for (size_t i = 0; i < excess; ++i) {
            cleanup.push_back(m_rotatedFiles[i].path);
        }
    }
    return cleanup;
}

}} // namespace ExplorerLens::Engine

