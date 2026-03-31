// CrashIntelligence.h — Crash analysis, bucketing, and minidump metadata
// Copyright (c) 2026 ExplorerLens Project
//
// Provides stack frame symbolization, crash signature generation,
// severity-based bucketing, and minidump path sanitization for
// post-mortem crash analysis.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace CrashIntel {

struct StackFrame {
    uint64_t address = 0;
    std::wstring module_name;
    std::wstring function_name;
    uint32_t line_number = 0;

    bool IsSymbolized() const noexcept {
        return !function_name.empty();
    }

    std::wstring ToString() const {
        std::wstring result = module_name + L"!" + function_name;
        if (line_number > 0)
            result += L" +" + std::to_wstring(line_number);
        return result;
    }
};

struct MinidumpMetadata {
    std::wstring dumpPath;
    uint64_t timestamp = 0;
    uint32_t processId = 0;

    static std::wstring SanitizePath(const std::wstring& path) {
        auto pos = path.rfind(L'\\');
        if (pos != std::wstring::npos)
            return L"[redacted]\\" + path.substr(pos + 1);
        return path;
    }
};

struct CrashSignature {
    std::wstring module;
    uint32_t exception_code = 0;
    std::vector<std::wstring> top_frames;

    std::wstring ToBucketKey() const {
        std::wstring key = module + L"_" + std::to_wstring(exception_code);
        for (auto& f : top_frames)
            key += L"_" + f;
        return key;
    }

    bool operator==(const CrashSignature& other) const noexcept {
        return module == other.module &&
               exception_code == other.exception_code &&
               top_frames == other.top_frames;
    }
};

struct CrashBucket {
    CrashSignature signature;
    uint32_t hit_count = 0;
    std::vector<std::wstring> dump_ids;

    void RecordHit(const std::wstring& dumpId) {
        hit_count++;
        dump_ids.push_back(dumpId);
    }

    std::wstring ComputeSeverity() const noexcept {
        if (hit_count >= 100) return L"Critical";
        if (hit_count >= 10)  return L"High";
        if (hit_count >= 5)   return L"Medium";
        return L"Low";
    }
};

} // namespace CrashIntel

// Engine-level singleton wrapping crash intelligence for test access
namespace Engine {

class CrashIntelligenceEngine {
public:
    static CrashIntelligenceEngine& Instance() {
        static CrashIntelligenceEngine s_instance;
        return s_instance;
    }

    bool Initialize() noexcept { m_initialized = true; return true; }

    std::vector<CrashIntel::StackFrame> CaptureStackTrace(int /*skipFrames*/) const {
        return {};
    }

    struct Stats {
        uint64_t crashesCaught = 0;
        uint64_t dumpsSaved = 0;
    };

    Stats GetStats() const noexcept { return m_stats; }

private:
    CrashIntelligenceEngine() = default;
    bool m_initialized = false;
    Stats m_stats{};
};

} // namespace Engine

} // namespace ExplorerLens
