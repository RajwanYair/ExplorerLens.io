// PluginCompatibilityKitV2.h — Canonical Plugin Compatibility Kit (V1 + V2 consolidated)
// Copyright (c) 2026 ExplorerLens Project
//
// Consolidated: V2 ABI/conformance kit (ExplorerLens::Plugin) + V1 PE validator (ExplorerLens::Engine).
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens::Plugin {

// ─── ABI version ──────────────────────────────────────────────────────────────

struct ABIVersion
{
    uint32_t major{1};
    uint32_t minor{0};
    uint32_t patch{0};

    bool IsCompatible(const ABIVersion& target) const
    {
        return major == target.major;  // major-version compatibility only
    }

    std::string ToString() const
    {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }

    static ABIVersion V1()
    {
        return {1, 0, 0};
    }
    static ABIVersion V1_1()
    {
        return {1, 1, 0};
    }
    static ABIVersion V2()
    {
        return {2, 0, 0};
    }

    bool operator==(const ABIVersion& o) const
    {
        return major == o.major && minor == o.minor && patch == o.patch;
    }
    bool operator!=(const ABIVersion& o) const
    {
        return !(*this == o);
    }
};

// ─── Exported symbol set ───────────────────────────────────────────────────────

struct ExportedSymbol
{
    std::string name;
    std::string signature;  // simplified type signature
    bool isRequired{true};
};

struct ABIStableSurface
{
    ABIVersion version;
    std::vector<ExportedSymbol> symbols;

    static ABIStableSurface V1Baseline()
    {
        ABIStableSurface s;
        s.version = ABIVersion::V1();
        s.symbols = {
            {"DT_PluginGetVersion", "uint32_t()", true},
            {"DT_PluginInit", "bool(const char*)", true},
            {"DT_PluginDecode", "bool(const uint8_t*,size_t,void**)", true},
            {"DT_PluginGetCapabilities", "uint32_t()", true},
            {"DT_PluginShutdown", "void()", true},
        };
        return s;
    }
};

// ─── Conformance checks ────────────────────────────────────────────────────────

enum class ConformanceCheckId : uint32_t {
    ABIVersionMatch = 1,
    RequiredExports = 2,
    PerfGate = 3,
    MemoryGate = 4,
    CrashIsolation = 5,
    ABIDriftDetection = 6,
};

inline std::string ToString(ConformanceCheckId id)
{
    switch (id) {
        case ConformanceCheckId::ABIVersionMatch:
            return "ABIVersionMatch";
        case ConformanceCheckId::RequiredExports:
            return "RequiredExports";
        case ConformanceCheckId::PerfGate:
            return "PerfGate";
        case ConformanceCheckId::MemoryGate:
            return "MemoryGate";
        case ConformanceCheckId::CrashIsolation:
            return "CrashIsolation";
        case ConformanceCheckId::ABIDriftDetection:
            return "ABIDriftDetection";
        default:
            return "Unknown";
    }
}

struct ConformanceCheckResult
{
    ConformanceCheckId id;
    bool passed{false};
    std::string message;
    std::string details;

    static ConformanceCheckResult Pass(ConformanceCheckId id, std::string msg = "")
    {
        return {id, true, std::move(msg), ""};
    }
    static ConformanceCheckResult Fail(ConformanceCheckId id, std::string msg, std::string det = "")
    {
        return {id, false, std::move(msg), std::move(det)};
    }
};

// ─── Performance gate ─────────────────────────────────────────────────────────

struct PluginPerfGate
{
    double maxDecodeMs{100.0};    // p95 decode must be under 100ms
    double maxInitMs{500.0};      // init must complete under 500ms
    double maxShutdownMs{200.0};  // shutdown must complete under 200ms

    ConformanceCheckResult Evaluate(double decodeMs, double initMs, double shutdownMs) const
    {
        if (decodeMs > maxDecodeMs)
            return ConformanceCheckResult::Fail(
                ConformanceCheckId::PerfGate, "decode too slow",
                "got " + std::to_string(decodeMs) + "ms, limit " + std::to_string(maxDecodeMs) + "ms");
        if (initMs > maxInitMs)
            return ConformanceCheckResult::Fail(ConformanceCheckId::PerfGate, "init too slow",
                                                "got " + std::to_string(initMs) + "ms");
        if (shutdownMs > maxShutdownMs)
            return ConformanceCheckResult::Fail(ConformanceCheckId::PerfGate, "shutdown too slow",
                                                "got " + std::to_string(shutdownMs) + "ms");
        return ConformanceCheckResult::Pass(ConformanceCheckId::PerfGate, "all timing gates pass");
    }
};

// ─── Memory gate ─────────────────────────────────────────────────────────────

struct PluginMemoryGate
{
    uint64_t maxPeakBytes{50ULL * 1024 * 1024};  // 50 MB peak per decode

    ConformanceCheckResult Evaluate(uint64_t peakBytes) const
    {
        if (peakBytes > maxPeakBytes)
            return ConformanceCheckResult::Fail(ConformanceCheckId::MemoryGate, "peak memory exceeded",
                                                "got " + std::to_string(peakBytes / (1024 * 1024)) + "MB, limit 50MB");
        return ConformanceCheckResult::Pass(ConformanceCheckId::MemoryGate, "memory within budget");
    }
};

// ─── Conformance report ───────────────────────────────────────────────────────

struct PluginConformanceReport
{
    std::string pluginId;
    ABIVersion reportedVersion;
    std::vector<ConformanceCheckResult> checks;
    uint32_t passCount{0};
    uint32_t failCount{0};

    bool AllPassed() const
    {
        return failCount == 0;
    }

    std::string ToJSON() const
    {
        std::string json = "{\n \"pluginId\": \"" + pluginId + "\",\n";
        json += " \"abiVersion\": \"" + reportedVersion.ToString() + "\",\n";
        json += " \"pass\": " + std::to_string(passCount) + ",\n";
        json += " \"fail\": " + std::to_string(failCount) + ",\n";
        json += " \"verdict\": \"" + std::string(AllPassed() ? "PASS" : "FAIL") + "\"\n}";
        return json;
    }

    std::string Summary() const
    {
        return "Conformance " + pluginId + ": " + std::to_string(passCount) + "P/" + std::to_string(failCount) + "F — "
               + (AllPassed() ? "PASS" : "FAIL");
    }
};

// ─── ABI drift detector ───────────────────────────────────────────────────────

class ABIDriftDetector
{
  public:
    ABIDriftDetector(const ABIStableSurface& baseline, const ABIStableSurface& current)
        : m_baseline(baseline)
        , m_current(current)
    {}

    ConformanceCheckResult Detect() const
    {
        if (!m_baseline.version.IsCompatible(m_current.version)) {
            return ConformanceCheckResult::Fail(
                ConformanceCheckId::ABIDriftDetection, "ABI version mismatch",
                "baseline=" + m_baseline.version.ToString() + " current=" + m_current.version.ToString());
        }

        std::vector<std::string> missing;
        for (const auto& sym : m_baseline.symbols) {
            if (!sym.isRequired)
                continue;
            bool found = false;
            for (const auto& cur : m_current.symbols) {
                if (cur.name == sym.name) {
                    found = true;
                    break;
                }
            }
            if (!found)
                missing.push_back(sym.name);
        }

        if (!missing.empty()) {
            std::string detail = "missing exports:";
            for (const auto& m : missing)
                detail += " " + m;
            return ConformanceCheckResult::Fail(ConformanceCheckId::ABIDriftDetection, "required exports missing",
                                                detail);
        }

        return ConformanceCheckResult::Pass(ConformanceCheckId::ABIDriftDetection, "no ABI drift detected");
    }

  private:
    ABIStableSurface m_baseline;
    ABIStableSurface m_current;
};

}  // namespace ExplorerLens::Plugin

// ── PluginCompatibilityKit V1 (consolidated from PluginCompatibilityKit.h) ─

namespace ExplorerLens {
namespace Engine {

struct CompatResult
{
    bool compatible = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    std::string dllVersion;
    uint32_t abiVersion = 0;
    bool is64bit = false;
    std::string compiler;
};

struct CompatStats
{
    uint64_t pluginsChecked = 0;
    uint64_t pluginsPassed = 0;
    uint64_t pluginsFailed = 0;
    std::unordered_map<std::string, uint64_t> failureReasons;
};

class PluginCompatibilityKit
{
  public:
    static constexpr uint32_t CURRENT_ABI_VERSION = 15;

    PluginCompatibilityKit()
    {
        InitializeSRWLock(&m_lock);
        m_requiredExports = {"PluginInit", "PluginShutdown", "GetPluginInfo", "GetABIVersion"};
    }

    ~PluginCompatibilityKit() = default;

    PluginCompatibilityKit(const PluginCompatibilityKit&) = delete;
    PluginCompatibilityKit& operator=(const PluginCompatibilityKit&) = delete;

    inline void SetRequiredExports(const std::vector<std::string>& exports)
    {
        AcquireSRWLockExclusive(&m_lock);
        m_requiredExports = exports;
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline CompatResult ValidatePlugin(const std::wstring& dllPath)
    {
        CompatResult result;

        AcquireSRWLockExclusive(&m_lock);
        m_stats.pluginsChecked++;
        ReleaseSRWLockExclusive(&m_lock);

        std::vector<uint8_t> peData = ReadFileBytes(dllPath);
        if (peData.empty()) {
            result.errors.push_back("Failed to read PE file");
            RecordFailure("file_read_error");
            return result;
        }

        if (peData.size() < sizeof(IMAGE_DOS_HEADER)) {
            result.errors.push_back("File too small for DOS header");
            RecordFailure("invalid_dos_header");
            return result;
        }

        auto* dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(peData.data());
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            result.errors.push_back("Invalid DOS signature (expected MZ)");
            RecordFailure("invalid_dos_signature");
            return result;
        }

        DWORD ntOffset = static_cast<DWORD>(dosHeader->e_lfanew);
        if (ntOffset + sizeof(IMAGE_NT_HEADERS64) > peData.size()) {
            result.errors.push_back("NT header offset out of bounds");
            RecordFailure("invalid_nt_offset");
            return result;
        }

        auto* ntHeaders = reinterpret_cast<const IMAGE_NT_HEADERS64*>(peData.data() + ntOffset);
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
            result.errors.push_back("Invalid NT signature (expected PE\\0\\0)");
            RecordFailure("invalid_nt_signature");
            return result;
        }

        result.is64bit = (ntHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64);
        if (!result.is64bit) {
            result.errors.push_back("Not a 64-bit DLL (machine type: 0x" + ToHex(ntHeaders->FileHeader.Machine) + ")");
            RecordFailure("not_64bit");
        }

        if (!(ntHeaders->FileHeader.Characteristics & IMAGE_FILE_DLL)) {
            result.errors.push_back("PE file is not marked as DLL");
            RecordFailure("not_dll");
        }

        auto exports = ParseExportTable(peData, ntHeaders, ntOffset);
        AcquireSRWLockShared(&m_lock);
        auto requiredExports = m_requiredExports;
        ReleaseSRWLockShared(&m_lock);

        for (const auto& req : requiredExports) {
            bool found = false;
            for (const auto& exp : exports) {
                if (exp == req) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                result.errors.push_back("Missing required export: " + req);
                RecordFailure("missing_export_" + req);
            }
        }

        auto imports = ParseImportTable(peData, ntHeaders, ntOffset);
        for (const auto& imp : imports) {
            std::string lower = imp;
            std::transform(lower.begin(), lower.end(), lower.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (lower.find("vcruntime") != std::string::npos || lower.find("ucrtbase") != std::string::npos
                || lower.find("msvcr") != std::string::npos) {
                result.compiler = imp;
            }

            if (!CheckDependencyExists(imp)) {
                result.warnings.push_back("Dependency not found on system: " + imp);
            }
        }

        HMODULE hMod = LoadLibraryExW(dllPath.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
        if (hMod) {
            using GetABIVersionFn = uint32_t (*)();
            auto fnABI = reinterpret_cast<GetABIVersionFn>(GetProcAddress(hMod, "GetABIVersion"));
            if (fnABI) {
                result.abiVersion = fnABI();
                if (result.abiVersion > CURRENT_ABI_VERSION) {
                    std::ostringstream oss;
                    oss << "ABI version " << result.abiVersion << " exceeds current (" << CURRENT_ABI_VERSION << ")";
                    result.errors.push_back(oss.str());
                    RecordFailure("abi_version_mismatch");
                }
            } else {
                result.warnings.push_back("GetABIVersion export not callable (loaded without resolving)");
            }

            using GetPluginInfoFn = const char* (*)();
            auto fnInfo = reinterpret_cast<GetPluginInfoFn>(GetProcAddress(hMod, "GetPluginInfo"));
            if (fnInfo) {
                const char* info = fnInfo();
                if (info)
                    result.dllVersion = info;
            }

            FreeLibrary(hMod);
        } else {
            result.warnings.push_back("Could not load DLL for ABI check (LoadLibraryExW failed)");
        }

        result.compatible = result.errors.empty();

        AcquireSRWLockExclusive(&m_lock);
        if (result.compatible) {
            m_stats.pluginsPassed++;
        } else {
            m_stats.pluginsFailed++;
        }
        ReleaseSRWLockExclusive(&m_lock);

        return result;
    }

    inline bool QuickCheck(const std::wstring& dllPath)
    {
        std::vector<uint8_t> peData = ReadFileBytes(dllPath);
        if (peData.size() < sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS64)) {
            return false;
        }

        auto* dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(peData.data());
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
            return false;

        DWORD ntOffset = static_cast<DWORD>(dosHeader->e_lfanew);
        if (ntOffset + sizeof(IMAGE_NT_HEADERS64) > peData.size())
            return false;

        auto* ntHeaders = reinterpret_cast<const IMAGE_NT_HEADERS64*>(peData.data() + ntOffset);
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
            return false;
        if (ntHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64)
            return false;
        if (!(ntHeaders->FileHeader.Characteristics & IMAGE_FILE_DLL))
            return false;

        auto exports = ParseExportTable(peData, ntHeaders, ntOffset);
        AcquireSRWLockShared(&m_lock);
        auto requiredExports = m_requiredExports;
        ReleaseSRWLockShared(&m_lock);

        for (const auto& req : requiredExports) {
            bool found = false;
            for (const auto& exp : exports) {
                if (exp == req) {
                    found = true;
                    break;
                }
            }
            if (!found)
                return false;
        }

        return true;
    }

    inline CompatStats GetStats() const
    {
        AcquireSRWLockShared(&m_lock);
        CompatStats s = m_stats;
        ReleaseSRWLockShared(&m_lock);
        return s;
    }

  private:
    inline std::vector<uint8_t> ReadFileBytes(const std::wstring& path)
    {
        std::vector<uint8_t> data;
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
            return data;

        LARGE_INTEGER size{};
        if (!GetFileSizeEx(hFile, &size) || size.QuadPart > 256 * 1024 * 1024) {
            CloseHandle(hFile);
            return data;
        }

        data.resize(static_cast<size_t>(size.QuadPart));
        DWORD bytesRead = 0;
        DWORD totalRead = 0;
        while (totalRead < static_cast<DWORD>(size.QuadPart)) {
            DWORD toRead = (std::min)(static_cast<DWORD>(size.QuadPart) - totalRead, static_cast<DWORD>(65536));
            if (!ReadFile(hFile, data.data() + totalRead, toRead, &bytesRead, nullptr) || bytesRead == 0) {
                break;
            }
            totalRead += bytesRead;
        }
        CloseHandle(hFile);

        if (totalRead != static_cast<DWORD>(size.QuadPart)) {
            data.clear();
        }
        return data;
    }

    inline std::vector<std::string> ParseExportTable(const std::vector<uint8_t>& pe, const IMAGE_NT_HEADERS64* nth,
                                                     DWORD ntOffset)
    {
        std::vector<std::string> exportNames;
        (void)ntOffset;

        if (nth->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_EXPORT) {
            return exportNames;
        }

        DWORD exportRVA = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        DWORD exportSize = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
        if (exportRVA == 0 || exportSize == 0)
            return exportNames;

        DWORD exportOffset = RvaToFileOffset(pe, nth, exportRVA);
        if (exportOffset == 0 || exportOffset + sizeof(IMAGE_EXPORT_DIRECTORY) > pe.size()) {
            return exportNames;
        }

        auto* expDir = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(pe.data() + exportOffset);
        DWORD nameCount = expDir->NumberOfNames;
        DWORD namesRVA = expDir->AddressOfNames;
        DWORD namesOffset = RvaToFileOffset(pe, nth, namesRVA);
        if (namesOffset == 0)
            return exportNames;

        for (DWORD i = 0; i < nameCount; ++i) {
            if (namesOffset + (i + 1) * 4 > pe.size())
                break;
            DWORD nameRVA = *reinterpret_cast<const DWORD*>(pe.data() + namesOffset + i * 4);
            DWORD nameOff = RvaToFileOffset(pe, nth, nameRVA);
            if (nameOff == 0 || nameOff >= pe.size())
                continue;

            const char* name = reinterpret_cast<const char*>(pe.data() + nameOff);
            size_t maxLen = pe.size() - nameOff;
            size_t len = strnlen(name, maxLen);
            exportNames.emplace_back(name, len);
        }

        return exportNames;
    }

    inline std::vector<std::string> ParseImportTable(const std::vector<uint8_t>& pe, const IMAGE_NT_HEADERS64* nth,
                                                     DWORD ntOffset)
    {
        std::vector<std::string> importDlls;
        (void)ntOffset;

        if (nth->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_IMPORT) {
            return importDlls;
        }

        DWORD importRVA = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        if (importRVA == 0)
            return importDlls;

        DWORD importOffset = RvaToFileOffset(pe, nth, importRVA);
        if (importOffset == 0)
            return importDlls;

        for (size_t idx = 0;; ++idx) {
            size_t descOffset = importOffset + idx * sizeof(IMAGE_IMPORT_DESCRIPTOR);
            if (descOffset + sizeof(IMAGE_IMPORT_DESCRIPTOR) > pe.size())
                break;

            auto* desc = reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(pe.data() + descOffset);
            if (desc->Name == 0 && desc->FirstThunk == 0)
                break;

            DWORD nameOff = RvaToFileOffset(pe, nth, desc->Name);
            if (nameOff == 0 || nameOff >= pe.size())
                continue;

            const char* name = reinterpret_cast<const char*>(pe.data() + nameOff);
            size_t maxLen = pe.size() - nameOff;
            size_t len = strnlen(name, maxLen);
            importDlls.emplace_back(name, len);
        }

        return importDlls;
    }

    static inline DWORD RvaToFileOffset(const std::vector<uint8_t>& pe, const IMAGE_NT_HEADERS64* nth, DWORD rva)
    {
        const auto* section = IMAGE_FIRST_SECTION(nth);
        for (WORD i = 0; i < nth->FileHeader.NumberOfSections; ++i) {
            DWORD secStart = section[i].VirtualAddress;
            DWORD secEnd = secStart + section[i].Misc.VirtualSize;
            if (rva >= secStart && rva < secEnd) {
                DWORD delta = rva - secStart;
                DWORD fileOff = section[i].PointerToRawData + delta;
                if (fileOff < pe.size())
                    return fileOff;
            }
        }
        return 0;
    }

    inline bool CheckDependencyExists(const std::string& dllName)
    {
        int len = MultiByteToWideChar(CP_ACP, 0, dllName.c_str(), -1, nullptr, 0);
        if (len <= 0)
            return false;
        std::wstring wide(static_cast<size_t>(len - 1), L'\0');
        MultiByteToWideChar(CP_ACP, 0, dllName.c_str(), -1, wide.data(), len);

        wchar_t foundPath[MAX_PATH]{};
        DWORD result = SearchPathW(nullptr, wide.c_str(), nullptr, MAX_PATH, foundPath, nullptr);
        return result > 0;
    }

    inline void RecordFailure(const std::string& reason)
    {
        AcquireSRWLockExclusive(&m_lock);
        m_stats.failureReasons[reason]++;
        ReleaseSRWLockExclusive(&m_lock);
    }

    static inline std::string ToHex(uint32_t val)
    {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0') << std::setw(4) << val;
        return oss.str();
    }

    mutable SRWLOCK m_lock{};
    std::vector<std::string> m_requiredExports;
    CompatStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
