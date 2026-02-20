#pragma once
// Sprint 238: Log Rotation Engine — log file rotation, compression, and cleanup
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// Log rotation policy
enum class RotationPolicy : uint32_t {
    SizeBased  = 0,   ///< Rotate when file exceeds max size
    TimeBased  = 1,   ///< Rotate daily/hourly
    CountBased = 2,   ///< Keep N most recent files
    Hybrid     = 3,   ///< Size + time
    COUNT      = 4
};

/// Compression for rotated logs
enum class LogCompression : uint32_t {
    None    = 0,
    GZip    = 1,
    Zstd    = 2,
    LZ4     = 3,
    COUNT   = 4
};

/// Log rotation configuration
struct RotationConfig {
    RotationPolicy policy       = RotationPolicy::SizeBased;
    LogCompression compression  = LogCompression::None;
    uint64_t       maxSizeBytes = 10 * 1024 * 1024;  // 10 MB
    uint32_t       maxFiles     = 5;
    uint32_t       maxAgeDays   = 30;
    std::wstring   logDirectory;
    std::wstring   filePattern  = L"darkthumbs_{n}.log";
};

/// Info about a rotated log file
struct RotatedLogFile {
    std::wstring path;
    uint64_t     sizeBytes = 0;
    uint64_t     timestamp = 0;
    bool         compressed = false;
};

/// Manages log file rotation and cleanup
class LogRotationEngine {
public:
    LogRotationEngine();

    static const wchar_t* GetPolicyName(RotationPolicy policy);
    static const wchar_t* GetCompressionName(LogCompression comp);
    static uint32_t GetPolicyCount() { return static_cast<uint32_t>(RotationPolicy::COUNT); }

    /// Set rotation configuration
    void SetConfig(const RotationConfig& config) { m_config = config; }
    const RotationConfig& GetConfig() const { return m_config; }

    /// Check if rotation is needed based on current size
    bool NeedsRotation(uint64_t currentSizeBytes) const;
    /// Register a rotated file
    void AddRotatedFile(const std::wstring& path, uint64_t size);
    /// Get rotated files list
    const std::vector<RotatedLogFile>& GetRotatedFiles() const { return m_rotatedFiles; }
    /// Get files that should be cleaned up
    std::vector<std::wstring> GetFilesToCleanup() const;

private:
    RotationConfig m_config;
    std::vector<RotatedLogFile> m_rotatedFiles;
};

}} // namespace DarkThumbs::Engine
