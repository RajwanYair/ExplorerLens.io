/******************************************************************************
 * CrashIntelligence.h — Crash Intelligence & Symbol Pipeline
 * Copyright (c) 2026 — ExplorerLens Project
 *
 * Automated minidump capture, symbol pipeline, crash bucketing, and
 * diagnostic integration for fast crash triage across shell/engine/plugin.
 * Extends Engine/Plugin/CrashHandler.h with deep crash analysis.
 *
 *****************************************************************************/

#pragma once

#include <Windows.h>
#include <DbgHelp.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <functional>
#include <sstream>
#include <cstdint>

#pragma comment(lib, "dbghelp.lib")

namespace ExplorerLens {
namespace CrashIntel {

//============================================================================
// Stack Frame Information
//============================================================================

struct StackFrame {
    uint64_t address = 0;
    uint64_t offset = 0;
    std::wstring module_name;
    std::wstring function_name;
    std::wstring source_file;
    uint32_t line_number = 0;

    bool IsSymbolized() const {
        return !function_name.empty() && function_name != L"<unknown>";
    }

    std::wstring ToString() const {
        std::wostringstream ss;
        ss << module_name << L"!" << function_name;
        if (!source_file.empty()) {
            ss << L" [" << source_file << L":" << line_number << L"]";
        }
        ss << L" +0x" << std::hex << offset;
        return ss.str();
    }
};

//============================================================================
// Minidump Metadata (privacy-safe — no user paths, no PII)
//============================================================================

struct MinidumpMetadata {
    std::wstring dump_id; // GUID, e.g. "A1B2C3D4-..."
    std::wstring dump_path; // Local path to .dmp file
    std::wstring faulting_module;
    DWORD exception_code = 0;
    uint64_t faulting_address = 0;
    std::wstring os_version;
    std::wstring product_version;
    uint32_t process_uptime_ms = 0;
    std::chrono::system_clock::time_point timestamp;

    // Privacy scrubbing: strip user-specific paths
    static std::wstring SanitizePath(const std::wstring& path) {
        // Replace anything before the module filename with "<module>"
        auto pos = path.find_last_of(L"\\/");
        if (pos != std::wstring::npos) {
            return L"<path>\\" + path.substr(pos + 1);
        }
        return path;
    }
};

//============================================================================
// Crash Signature & Bucketing
//============================================================================

struct CrashSignature {
    std::wstring module; // Faulting module name
    DWORD exception_code = 0; // STATUS_ACCESS_VIOLATION, etc.
    std::vector<std::wstring> top_frames; // Top 5 function names (for bucketing)

    // Generate bucket key: "module|exception|frame1|frame2|...|frame5"
    std::wstring ToBucketKey() const {
        std::wostringstream ss;
        ss << module << L"|0x" << std::hex << exception_code;
        for (size_t i = 0; i < top_frames.size() && i < 5; ++i) {
            ss << L"|" << top_frames[i];
        }
        return ss.str();
    }

    bool operator==(const CrashSignature& other) const {
        return ToBucketKey() == other.ToBucketKey();
    }
};

struct CrashBucket {
    CrashSignature signature;
    uint32_t hit_count = 0;
    std::chrono::system_clock::time_point first_seen;
    std::chrono::system_clock::time_point last_seen;
    std::vector<std::wstring> dump_ids; // Associated dump GUIDs
    std::wstring severity; // "Critical", "High", "Medium", "Low"

    void RecordHit(const std::wstring& dump_id) {
        hit_count++;
        last_seen = std::chrono::system_clock::now();
        if (dump_ids.size() < 20) { // Keep max 20 dump references
            dump_ids.push_back(dump_id);
        }
    }

    // Severity: based on hit count and exception type
    std::wstring ComputeSeverity() const {
        if (signature.exception_code == STATUS_ACCESS_VIOLATION && hit_count >= 5)
            return L"Critical";
        if (hit_count >= 10) return L"Critical";
        if (hit_count >= 5) return L"High";
        if (hit_count >= 2) return L"Medium";
        return L"Low";
    }
};

//============================================================================
// Minidump Capturer
//============================================================================

class MinidumpCapturer {
public:
    struct Config {
        std::filesystem::path dump_directory; // e.g. %LOCALAPPDATA%/ExplorerLens/CrashDumps
        uint32_t max_dump_count = 50; // Auto-purge oldest beyond this
        uint64_t max_total_size_mb = 500; // Cap total dump storage
        bool include_heap = false; // MiniDumpWithFullMemory (large!)
        bool include_thread_info = true; // MiniDumpWithThreadInfo
        bool privacy_mode = true; // Strip PII from metadata
        std::wstring product_version = L"7.0.0";
    };

    explicit MinidumpCapturer(const Config& config = {})
        : config_(config) {
        if (config_.dump_directory.empty()) {
            wchar_t appdata[MAX_PATH] = {};
            if (GetEnvironmentVariableW(L"LOCALAPPDATA", appdata, MAX_PATH)) {
                config_.dump_directory = std::filesystem::path(appdata) / L"ExplorerLens" / L"CrashDumps";
            }
        }
    }

    // Capture minidump for current process (called from SEH filter)
    MinidumpMetadata CaptureForCurrentProcess(EXCEPTION_POINTERS* exception_ptrs) {
        MinidumpMetadata meta;
        meta.timestamp = std::chrono::system_clock::now();
        meta.dump_id = GenerateDumpId();
        meta.product_version = config_.product_version;
        meta.os_version = GetOSVersion();

        if (exception_ptrs && exception_ptrs->ExceptionRecord) {
            meta.exception_code = exception_ptrs->ExceptionRecord->ExceptionCode;
            meta.faulting_address = reinterpret_cast<uint64_t>(
                exception_ptrs->ExceptionRecord->ExceptionAddress);
        }

        // Ensure dump directory exists
        std::error_code ec;
        std::filesystem::create_directories(config_.dump_directory, ec);

        // Build dump filename: ExplorerLens_YYYYMMDD_HHMMSS_<id>.dmp
        auto t = std::chrono::system_clock::to_time_t(meta.timestamp);
        std::tm tm_buf = {};
        localtime_s(&tm_buf, &t);
        wchar_t time_str[64] = {};
        wcsftime(time_str, 64, L"%Y%m%d_%H%M%S", &tm_buf);

        std::wstring filename = L"ExplorerLens_" + std::wstring(time_str) +
            L"_" + meta.dump_id.substr(0, 8) + L".dmp";
        meta.dump_path = (config_.dump_directory / filename).wstring();

        // Write minidump via DbgHelp
        HANDLE hFile = CreateFileW(meta.dump_path.c_str(), GENERIC_WRITE, 0,
            nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != INVALID_HANDLE_VALUE) {
            MINIDUMP_TYPE dump_type = static_cast<MINIDUMP_TYPE>(
                MiniDumpNormal |
                (config_.include_thread_info ? MiniDumpWithThreadInfo : 0) |
                (config_.include_heap ? MiniDumpWithFullMemory : 0));

            MINIDUMP_EXCEPTION_INFORMATION mei = {};
            mei.ThreadId = GetCurrentThreadId();
            mei.ExceptionPointers = exception_ptrs;
            mei.ClientPointers = FALSE;

            MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                hFile, dump_type,
                exception_ptrs ? &mei : nullptr,
                nullptr, nullptr);
            CloseHandle(hFile);
        }

        // Privacy scrubbing
        if (config_.privacy_mode) {
            meta.faulting_module = MinidumpMetadata::SanitizePath(meta.faulting_module);
        }

        // Auto-purge old dumps
        PurgeOldDumps();
        return meta;
    }

    // Get current dump count
    uint32_t GetDumpCount() const {
        uint32_t count = 0;
        std::error_code ec;
        if (std::filesystem::exists(config_.dump_directory, ec)) {
            for (auto& entry : std::filesystem::directory_iterator(config_.dump_directory, ec)) {
                if (entry.path().extension() == L".dmp") count++;
            }
        }
        return count;
    }

    const Config& GetConfig() const { return config_; }

private:
    Config config_;

    std::wstring GenerateDumpId() const {
        // Simplified GUID-like ID
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        uint64_t hash = now ^ (static_cast<uint64_t>(GetCurrentProcessId()) << 32);
        wchar_t buf[64] = {};
        swprintf_s(buf, L"%08X-%04X-%04X-%04X-%012llX",
            static_cast<uint32_t>(hash >> 32),
            static_cast<uint16_t>(hash >> 16),
            static_cast<uint16_t>(hash),
            static_cast<uint16_t>(hash >> 48),
            hash & 0xFFFFFFFFFFFFULL);
        return buf;
    }

    std::wstring GetOSVersion() const {
        // Use RtlGetVersion for accurate version on Win10+
        OSVERSIONINFOW ovi = {};
        ovi.dwOSVersionInfoSize = sizeof(ovi);
        // Use RtlGetVersion (undeprecated) from ntdll — avoids C4996 on GetVersionExW
        using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        if (hNtdll) {
            auto fn = reinterpret_cast<RtlGetVersionFn>(GetProcAddress(hNtdll, "RtlGetVersion"));
            if (fn) fn(reinterpret_cast<PRTL_OSVERSIONINFOW>(&ovi));
        }
        wchar_t buf[64] = {};
        swprintf_s(buf, L"%u.%u.%u", ovi.dwMajorVersion, ovi.dwMinorVersion, ovi.dwBuildNumber);
        return buf;
    }

    void PurgeOldDumps() {
        std::error_code ec;
        if (!std::filesystem::exists(config_.dump_directory, ec)) return;

        struct DumpFile {
            std::filesystem::path path;
            std::filesystem::file_time_type mtime;
            uintmax_t size;
        };
        std::vector<DumpFile> dumps;
        uint64_t total_size = 0;

        for (auto& entry : std::filesystem::directory_iterator(config_.dump_directory, ec)) {
            if (entry.path().extension() == L".dmp") {
                auto sz = entry.file_size(ec);
                auto mt = entry.last_write_time(ec);
                dumps.push_back({ entry.path(), mt, sz });
                total_size += sz;
            }
        }

        // Sort oldest first
        std::sort(dumps.begin(), dumps.end(),
            [](const DumpFile& a, const DumpFile& b) {
                return a.mtime < b.mtime;
            });

        uint64_t max_bytes = config_.max_total_size_mb * 1024ULL * 1024ULL;

        // Remove oldest dumps if over count or size limits
        while (dumps.size() > config_.max_dump_count || total_size > max_bytes) {
            if (dumps.empty()) break;
            total_size -= dumps.front().size;
            std::filesystem::remove(dumps.front().path, ec);
            dumps.erase(dumps.begin());
        }
    }
};

//============================================================================
// Symbol Pipeline
//============================================================================

struct SymbolServerConfig {
    std::wstring server_url; // e.g. "https://symbols.explorerlens.dev"
    std::filesystem::path local_cache; // e.g. %TEMP%/ExplorerLens/SymbolCache
    std::wstring product_version;
    bool auto_publish = false; // CI mode: push .pdb on build
    uint32_t timeout_ms = 30000;

    // SRV*-style symbol path for debugger
    std::wstring ToSymbolPath() const {
        std::wostringstream ss;
        ss << L"SRV*" << local_cache.wstring() << L"*" << server_url;
        return ss.str();
    }
};

struct PdbInfo {
    std::wstring module_name; // e.g. "LENSShell.dll"
    std::wstring pdb_name; // e.g. "LENSShell.pdb"
    std::wstring guid; // PDB signature GUID
    uint32_t age = 0; // PDB age
    std::wstring version; // Product version at build time
    std::filesystem::path pdb_path;

    // Validate PDB matches module by GUID+age
    bool Matches(const std::wstring& module_guid, uint32_t module_age) const {
        return guid == module_guid && age == module_age;
    }
};

// Version mapping manifest: maps product version <-> PDB set
struct VersionManifest {
    std::wstring product_version;
    std::wstring build_id;
    std::chrono::system_clock::time_point build_time;
    std::vector<PdbInfo> symbols;

    // Find PDB for a given module
    const PdbInfo* FindPdb(const std::wstring& module_name) const {
        for (auto& pdb : symbols) {
            if (_wcsicmp(pdb.module_name.c_str(), module_name.c_str()) == 0) {
                return &pdb;
            }
        }
        return nullptr;
    }
};

class SymbolPipeline {
public:
    explicit SymbolPipeline(const SymbolServerConfig& config = {})
        : config_(config) {
        if (config_.local_cache.empty()) {
            wchar_t tmp[MAX_PATH] = {};
            if (GetTempPathW(MAX_PATH, tmp)) {
                config_.local_cache = std::filesystem::path(tmp) / L"ExplorerLens" / L"SymbolCache";
            }
        }
    }

    // Register PDB from a build artifact
    void RegisterPdb(const PdbInfo& info) {
        std::lock_guard lock(mutex_);
        pdbs_[info.module_name] = info;
    }

    // Symbolize a raw address in a module
    StackFrame Symbolize(const std::wstring& module_name, uint64_t address) const {
        StackFrame frame;
        frame.address = address;
        frame.module_name = module_name;

        // Use DbgHelp SymFromAddr if symbol context loaded
        // (Simplified: in production, SymInitialize + SymLoadModuleEx first)
        HANDLE hProcess = GetCurrentProcess();
        char symbol_buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)] = {};
        PSYMBOL_INFO symbol = reinterpret_cast<PSYMBOL_INFO>(symbol_buffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 displacement = 0;
        if (SymFromAddr(hProcess, address, &displacement, symbol)) {
            frame.function_name.assign(symbol->Name, symbol->Name + symbol->NameLen);
            frame.offset = displacement;
        }
        else {
            frame.function_name = L"<unknown>";
            frame.offset = address;
        }

        // Line info
        IMAGEHLP_LINE64 line = {};
        line.SizeOfStruct = sizeof(line);
        DWORD line_disp = 0;
        if (SymGetLineFromAddr64(hProcess, address, &line_disp, &line)) {
            if (line.FileName) {
                std::string fn(line.FileName);
                frame.source_file.assign(fn.begin(), fn.end());
            }
            frame.line_number = line.LineNumber;
        }

        return frame;
    }

    // Symbolize an entire stack trace
    std::vector<StackFrame> SymbolizeStack(const std::vector<std::pair<std::wstring, uint64_t>>& raw_frames) const {
        std::vector<StackFrame> result;
        result.reserve(raw_frames.size());
        for (auto& [module, addr] : raw_frames) {
            result.push_back(Symbolize(module, addr));
        }
        return result;
    }

    // Check if we have symbols for all produced binaries (CI validation)
    struct SymbolVerification {
        uint32_t total_binaries = 0;
        uint32_t matched_pdbs = 0;
        uint32_t missing_pdbs = 0;
        std::vector<std::wstring> missing_modules;

        bool AllSymbolsPresent() const { return missing_pdbs == 0; }
        float CoveragePercent() const {
            return total_binaries > 0 ? (matched_pdbs * 100.0f / total_binaries) : 0.0f;
        }
    };

    SymbolVerification VerifySymbolCoverage(const std::vector<std::wstring>& binary_names) const {
        SymbolVerification result;
        result.total_binaries = static_cast<uint32_t>(binary_names.size());

        std::lock_guard lock(mutex_);
        for (auto& bin : binary_names) {
            if (pdbs_.count(bin)) {
                result.matched_pdbs++;
            }
            else {
                result.missing_pdbs++;
                result.missing_modules.push_back(bin);
            }
        }
        return result;
    }

    const SymbolServerConfig& GetConfig() const { return config_; }

private:
    SymbolServerConfig config_;
    std::unordered_map<std::wstring, PdbInfo> pdbs_;
    mutable std::mutex mutex_;
};

//============================================================================
// Crash Bucketing Engine
//============================================================================

class CrashBucketingEngine {
public:
    // Build crash signature from a symbolized stack + exception info
    CrashSignature BuildSignature(const std::wstring& module,
        DWORD exception_code,
        const std::vector<StackFrame>& stack) const {
        CrashSignature sig;
        sig.module = module;
        sig.exception_code = exception_code;

        // Take top 5 symbolized frames for bucketing
        for (size_t i = 0; i < stack.size() && sig.top_frames.size() < 5; ++i) {
            if (stack[i].IsSymbolized()) {
                sig.top_frames.push_back(stack[i].function_name);
            }
        }
        return sig;
    }

    // File a crash into a bucket (creates new bucket if new signature)
    CrashBucket& FileCrash(const CrashSignature& signature, const std::wstring& dump_id) {
        std::lock_guard lock(mutex_);
        auto key = signature.ToBucketKey();
        auto it = buckets_.find(key);
        if (it == buckets_.end()) {
            CrashBucket bucket;
            bucket.signature = signature;
            bucket.first_seen = std::chrono::system_clock::now();
            bucket.last_seen = bucket.first_seen;
            bucket.hit_count = 0;
            auto [insert_it, _] = buckets_.emplace(key, std::move(bucket));
            it = insert_it;
        }
        it->second.RecordHit(dump_id);
        it->second.severity = it->second.ComputeSeverity();
        return it->second;
    }

    // Check if crash is a known duplicate
    bool IsDuplicate(const CrashSignature& signature) const {
        std::lock_guard lock(mutex_);
        return buckets_.count(signature.ToBucketKey()) > 0;
    }

    // Get all buckets sorted by severity/hit count
    std::vector<CrashBucket> GetBucketsByPriority() const {
        std::lock_guard lock(mutex_);
        std::vector<CrashBucket> result;
        result.reserve(buckets_.size());
        for (auto& [_, bucket] : buckets_) {
            result.push_back(bucket);
        }
        // Sort: Critical > High > Medium > Low, then by hit count desc
        std::sort(result.begin(), result.end(),
            [](const CrashBucket& a, const CrashBucket& b) {
                auto severityRank = [](const std::wstring& s) -> int {
                    if (s == L"Critical") return 4;
                    if (s == L"High") return 3;
                    if (s == L"Medium") return 2;
                    if (s == L"Low") return 1;
                    return 0;
                    };
                int ra = severityRank(a.severity);
                int rb = severityRank(b.severity);
                if (ra != rb) return ra > rb;
                return a.hit_count > b.hit_count;
            });
        return result;
    }

    // Get bucket count
    uint32_t GetBucketCount() const {
        std::lock_guard lock(mutex_);
        return static_cast<uint32_t>(buckets_.size());
    }

    // Get total crash count across all buckets
    uint32_t GetTotalCrashes() const {
        std::lock_guard lock(mutex_);
        uint32_t total = 0;
        for (auto& [_, b] : buckets_) total += b.hit_count;
        return total;
    }

private:
    std::unordered_map<std::wstring, CrashBucket> buckets_;
    mutable std::mutex mutex_;
};

//============================================================================
// Diagnostics Integration (for LENSManager)
//============================================================================

struct DiagnosticsCrashSummary {
    uint32_t total_crashes = 0;
    uint32_t unique_signatures = 0;
    uint32_t critical_buckets = 0;
    uint32_t dumps_on_disk = 0;
    uint64_t dump_storage_bytes = 0;
    std::vector<CrashBucket> top_issues; // Top 5 buckets by severity/count
    std::wstring latest_dump_id;
    std::chrono::system_clock::time_point last_crash_time;

    bool HasRecentCrashes(std::chrono::hours window = std::chrono::hours(24)) const {
        auto now = std::chrono::system_clock::now();
        return (now - last_crash_time) < window;
    }
};

class CrashDiagnostics {
public:
    CrashDiagnostics(CrashBucketingEngine& buckets, MinidumpCapturer& capturer)
        : buckets_(buckets), capturer_(capturer) {
    }

    DiagnosticsCrashSummary GenerateSummary() const {
        DiagnosticsCrashSummary summary;
        summary.total_crashes = buckets_.GetTotalCrashes();
        summary.unique_signatures = buckets_.GetBucketCount();
        summary.dumps_on_disk = capturer_.GetDumpCount();

        auto prioritized = buckets_.GetBucketsByPriority();
        for (auto& b : prioritized) {
            if (b.severity == L"Critical") summary.critical_buckets++;
        }

        // Top 5 issues
        size_t top_n = std::min<size_t>(prioritized.size(), 5);
        summary.top_issues.assign(prioritized.begin(), prioritized.begin() + top_n);

        if (!prioritized.empty()) {
            summary.last_crash_time = prioritized.front().last_seen;
            if (!prioritized.front().dump_ids.empty()) {
                summary.latest_dump_id = prioritized.front().dump_ids.back();
            }
        }

        return summary;
    }

    // Format summary for LENSManager display
    std::wstring FormatForDisplay() const {
        auto summary = GenerateSummary();
        std::wostringstream ss;
        ss << L"=== Crash Intelligence Summary ===" << std::endl;
        ss << L"Total crashes: " << summary.total_crashes << std::endl;
        ss << L"Unique signatures: " << summary.unique_signatures << std::endl;
        ss << L"Critical issues: " << summary.critical_buckets << std::endl;
        ss << L"Dumps on disk: " << summary.dumps_on_disk << std::endl;

        if (!summary.top_issues.empty()) {
            ss << std::endl << L"--- Top Issues ---" << std::endl;
            for (size_t i = 0; i < summary.top_issues.size(); ++i) {
                auto& b = summary.top_issues[i];
                ss << (i + 1) << L". [" << b.severity << L"] "
                    << b.signature.module << L" (0x"
                    << std::hex << b.signature.exception_code << std::dec
                    << L") — " << b.hit_count << L" hits" << std::endl;
            }
        }
        return ss.str();
    }

private:
    CrashBucketingEngine& buckets_;
    MinidumpCapturer& capturer_;
};

//============================================================================
// Top-Level Exception Filter (install once at startup)
//============================================================================

class CrashIntelligenceSystem {
public:
    static CrashIntelligenceSystem& Instance() {
        static CrashIntelligenceSystem instance;
        return instance;
    }

    CrashIntelligenceSystem(const CrashIntelligenceSystem&) = delete;
    CrashIntelligenceSystem& operator=(const CrashIntelligenceSystem&) = delete;

    // Initialize the system (call once at DLL/process startup)
    void Initialize(const MinidumpCapturer::Config& dump_config = {},
        const SymbolServerConfig& sym_config = {}) {
        capturer_ = std::make_unique<MinidumpCapturer>(dump_config);
        symbols_ = std::make_unique<SymbolPipeline>(sym_config);
        buckets_ = std::make_unique<CrashBucketingEngine>();
        initialized_ = true;
    }

    bool IsInitialized() const { return initialized_; }

    MinidumpCapturer& Capturer() { return *capturer_; }
    SymbolPipeline& Symbols() { return *symbols_; }
    CrashBucketingEngine& Buckets() { return *buckets_; }

    // Convenience: generate diagnostics summary
    DiagnosticsCrashSummary GetDiagnosticsSummary() const {
        if (!initialized_) return {};
        CrashDiagnostics diag(*buckets_, *capturer_);
        return diag.GenerateSummary();
    }

    // Process a crash end-to-end: capture dump → symbolize → bucket
    CrashBucket ProcessCrash(EXCEPTION_POINTERS* exception_ptrs,
        const std::vector<std::pair<std::wstring, uint64_t>>& raw_stack) {
        if (!initialized_) return {};

        // 1. Capture minidump
        auto meta = capturer_->CaptureForCurrentProcess(exception_ptrs);

        // 2. Symbolize stack
        auto stack = symbols_->SymbolizeStack(raw_stack);

        // 3. Build signature and file into bucket
        auto sig = buckets_->BuildSignature(meta.faulting_module,
            meta.exception_code,
            stack);

        // Note: BuildSignature doesn't take dump_id, use FileCrash
        auto signature = CrashSignature{};
        signature.module = meta.faulting_module;
        signature.exception_code = meta.exception_code;
        for (size_t i = 0; i < stack.size() && signature.top_frames.size() < 5; ++i) {
            if (stack[i].IsSymbolized())
                signature.top_frames.push_back(stack[i].function_name);
        }

        return buckets_->FileCrash(signature, meta.dump_id);
    }

private:
    CrashIntelligenceSystem() = default;

    std::unique_ptr<MinidumpCapturer> capturer_;
    std::unique_ptr<SymbolPipeline> symbols_;
    std::unique_ptr<CrashBucketingEngine> buckets_;
    bool initialized_ = false;
};

} // namespace CrashIntel
} // namespace ExplorerLens
