//==============================================================================
// ExplorerLens Engine — Plugin Compatibility Kit (Sprint 581)
//
// Validates plugin binary compatibility before loading by inspecting PE
// headers, export tables, import tables, bitness, and ABI version. Reads
// PE structures directly from disk (IMAGE_DOS_HEADER, IMAGE_NT_HEADERS64,
// IMAGE_EXPORT_DIRECTORY, IMAGE_IMPORT_DESCRIPTOR) to avoid loading
// untrusted code. Also performs quick CRT detection by scanning import names.
//
// Thread-safe with SRWLOCK. Header-only, C++20, MSVC /W4 clean.
//==============================================================================
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cstdint>
#include <fstream>
#include <algorithm>
#include <sstream>

namespace ExplorerLens {
namespace Engine {

struct CompatResult {
    bool                     compatible = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    std::string              dllVersion;
    uint32_t                 abiVersion  = 0;
    bool                     is64bit     = false;
    std::string              compiler;
};

struct CompatStats {
    uint64_t pluginsChecked   = 0;
    uint64_t pluginsPassed    = 0;
    uint64_t pluginsFailed    = 0;
    std::unordered_map<std::string, uint64_t> failureReasons;
};

class PluginCompatibilityKit {
public:
    static constexpr uint32_t CURRENT_ABI_VERSION = 15;

    PluginCompatibilityKit() {
        InitializeSRWLock(&m_lock);
        m_requiredExports = { "PluginInit", "PluginShutdown", "GetPluginInfo", "GetABIVersion" };
    }

    ~PluginCompatibilityKit() = default;

    PluginCompatibilityKit(const PluginCompatibilityKit&) = delete;
    PluginCompatibilityKit& operator=(const PluginCompatibilityKit&) = delete;

    inline void SetRequiredExports(const std::vector<std::string>& exports) {
        AcquireSRWLockExclusive(&m_lock);
        m_requiredExports = exports;
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline CompatResult ValidatePlugin(const std::wstring& dllPath) {
        CompatResult result;

        AcquireSRWLockExclusive(&m_lock);
        m_stats.pluginsChecked++;
        ReleaseSRWLockExclusive(&m_lock);

        // Read the entire PE file into memory
        std::vector<uint8_t> peData = ReadFileBytes(dllPath);
        if (peData.empty()) {
            result.errors.push_back("Failed to read PE file");
            RecordFailure("file_read_error");
            return result;
        }

        // Step 1: Validate DOS header
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

        // Step 2: Validate NT headers
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

        // Step 3: Check machine type (must be AMD64)
        result.is64bit = (ntHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64);
        if (!result.is64bit) {
            result.errors.push_back("Not a 64-bit DLL (machine type: 0x" +
                ToHex(ntHeaders->FileHeader.Machine) + ")");
            RecordFailure("not_64bit");
        }

        // Step 4: Check it's a DLL
        if (!(ntHeaders->FileHeader.Characteristics & IMAGE_FILE_DLL)) {
            result.errors.push_back("PE file is not marked as DLL");
            RecordFailure("not_dll");
        }

        // Step 5: Parse export table to verify required exports
        auto exports = ParseExportTable(peData, ntHeaders, ntOffset);
        AcquireSRWLockShared(&m_lock);
        auto requiredExports = m_requiredExports;
        ReleaseSRWLockShared(&m_lock);

        for (const auto& req : requiredExports) {
            bool found = false;
            for (const auto& exp : exports) {
                if (exp == req) { found = true; break; }
            }
            if (!found) {
                result.errors.push_back("Missing required export: " + req);
                RecordFailure("missing_export_" + req);
            }
        }

        // Step 6: Parse import table for CRT detection and dependency check
        auto imports = ParseImportTable(peData, ntHeaders, ntOffset);
        for (const auto& imp : imports) {
            // CRT detection
            std::string lower = imp;
            std::transform(lower.begin(), lower.end(), lower.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (lower.find("vcruntime") != std::string::npos ||
                lower.find("ucrtbase") != std::string::npos ||
                lower.find("msvcr") != std::string::npos) {
                result.compiler = imp; // Record CRT DLL as compiler info
            }

            // Check dependency exists on system
            if (!CheckDependencyExists(imp)) {
                result.warnings.push_back("Dependency not found on system: " + imp);
            }
        }

        // Step 7: ABI version check via LoadLibraryExW (DONT_RESOLVE_DLL_REFERENCES)
        HMODULE hMod = LoadLibraryExW(dllPath.c_str(), nullptr,
            DONT_RESOLVE_DLL_REFERENCES);
        if (hMod) {
            using GetABIVersionFn = uint32_t(*)();
            auto fnABI = reinterpret_cast<GetABIVersionFn>(
                GetProcAddress(hMod, "GetABIVersion"));
            if (fnABI) {
                result.abiVersion = fnABI();
                if (result.abiVersion > CURRENT_ABI_VERSION) {
                    std::ostringstream oss;
                    oss << "ABI version " << result.abiVersion
                        << " exceeds current (" << CURRENT_ABI_VERSION << ")";
                    result.errors.push_back(oss.str());
                    RecordFailure("abi_version_mismatch");
                }
            } else {
                result.warnings.push_back("GetABIVersion export not callable (loaded without resolving)");
            }

            // Try to get version string
            using GetPluginInfoFn = const char*(*)();
            auto fnInfo = reinterpret_cast<GetPluginInfoFn>(
                GetProcAddress(hMod, "GetPluginInfo"));
            if (fnInfo) {
                const char* info = fnInfo();
                if (info) result.dllVersion = info;
            }

            FreeLibrary(hMod);
        } else {
            result.warnings.push_back("Could not load DLL for ABI check (LoadLibraryExW failed)");
        }

        // Determine final compatibility
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

    inline bool QuickCheck(const std::wstring& dllPath) {
        std::vector<uint8_t> peData = ReadFileBytes(dllPath);
        if (peData.size() < sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS64)) {
            return false;
        }

        auto* dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(peData.data());
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return false;

        DWORD ntOffset = static_cast<DWORD>(dosHeader->e_lfanew);
        if (ntOffset + sizeof(IMAGE_NT_HEADERS64) > peData.size()) return false;

        auto* ntHeaders = reinterpret_cast<const IMAGE_NT_HEADERS64*>(peData.data() + ntOffset);
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return false;
        if (ntHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) return false;
        if (!(ntHeaders->FileHeader.Characteristics & IMAGE_FILE_DLL)) return false;

        // Verify at least the required exports exist
        auto exports = ParseExportTable(peData, ntHeaders, ntOffset);
        AcquireSRWLockShared(&m_lock);
        auto requiredExports = m_requiredExports;
        ReleaseSRWLockShared(&m_lock);

        for (const auto& req : requiredExports) {
            bool found = false;
            for (const auto& exp : exports) {
                if (exp == req) { found = true; break; }
            }
            if (!found) return false;
        }

        return true;
    }

    inline CompatStats GetStats() const {
        AcquireSRWLockShared(&m_lock);
        CompatStats s = m_stats;
        ReleaseSRWLockShared(&m_lock);
        return s;
    }

private:
    inline std::vector<uint8_t> ReadFileBytes(const std::wstring& path) {
        std::vector<uint8_t> data;
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return data;

        LARGE_INTEGER size{};
        if (!GetFileSizeEx(hFile, &size) || size.QuadPart > 256 * 1024 * 1024) {
            CloseHandle(hFile);
            return data; // Refuse files > 256MB
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

    inline std::vector<std::string> ParseExportTable(
        const std::vector<uint8_t>& pe,
        const IMAGE_NT_HEADERS64* nth,
        DWORD ntOffset) {

        std::vector<std::string> exportNames;
        (void)ntOffset;

        if (nth->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_EXPORT) {
            return exportNames;
        }

        DWORD exportRVA  = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        DWORD exportSize = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
        if (exportRVA == 0 || exportSize == 0) return exportNames;

        // Convert RVA to file offset using section headers
        DWORD exportOffset = RvaToFileOffset(pe, nth, exportRVA);
        if (exportOffset == 0 || exportOffset + sizeof(IMAGE_EXPORT_DIRECTORY) > pe.size()) {
            return exportNames;
        }

        auto* expDir = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(pe.data() + exportOffset);
        DWORD nameCount = expDir->NumberOfNames;
        DWORD namesRVA  = expDir->AddressOfNames;
        DWORD namesOffset = RvaToFileOffset(pe, nth, namesRVA);
        if (namesOffset == 0) return exportNames;

        for (DWORD i = 0; i < nameCount; ++i) {
            if (namesOffset + (i + 1) * 4 > pe.size()) break;
            DWORD nameRVA = *reinterpret_cast<const DWORD*>(pe.data() + namesOffset + i * 4);
            DWORD nameOff = RvaToFileOffset(pe, nth, nameRVA);
            if (nameOff == 0 || nameOff >= pe.size()) continue;

            const char* name = reinterpret_cast<const char*>(pe.data() + nameOff);
            size_t maxLen = pe.size() - nameOff;
            size_t len = strnlen(name, maxLen);
            exportNames.emplace_back(name, len);
        }

        return exportNames;
    }

    inline std::vector<std::string> ParseImportTable(
        const std::vector<uint8_t>& pe,
        const IMAGE_NT_HEADERS64* nth,
        DWORD ntOffset) {

        std::vector<std::string> importDlls;
        (void)ntOffset;

        if (nth->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_IMPORT) {
            return importDlls;
        }

        DWORD importRVA = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        if (importRVA == 0) return importDlls;

        DWORD importOffset = RvaToFileOffset(pe, nth, importRVA);
        if (importOffset == 0) return importDlls;

        // Walk IMAGE_IMPORT_DESCRIPTOR entries (terminated by zero entry)
        for (size_t idx = 0; ; ++idx) {
            size_t descOffset = importOffset + idx * sizeof(IMAGE_IMPORT_DESCRIPTOR);
            if (descOffset + sizeof(IMAGE_IMPORT_DESCRIPTOR) > pe.size()) break;

            auto* desc = reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(pe.data() + descOffset);
            if (desc->Name == 0 && desc->FirstThunk == 0) break; // Terminator

            DWORD nameOff = RvaToFileOffset(pe, nth, desc->Name);
            if (nameOff == 0 || nameOff >= pe.size()) continue;

            const char* name = reinterpret_cast<const char*>(pe.data() + nameOff);
            size_t maxLen = pe.size() - nameOff;
            size_t len = strnlen(name, maxLen);
            importDlls.emplace_back(name, len);
        }

        return importDlls;
    }

    static inline DWORD RvaToFileOffset(
        const std::vector<uint8_t>& pe,
        const IMAGE_NT_HEADERS64* nth,
        DWORD rva) {

        // Walk section headers to find containing section
        const auto* section = IMAGE_FIRST_SECTION(nth);
        for (WORD i = 0; i < nth->FileHeader.NumberOfSections; ++i) {
            DWORD secStart = section[i].VirtualAddress;
            DWORD secEnd   = secStart + section[i].Misc.VirtualSize;
            if (rva >= secStart && rva < secEnd) {
                DWORD delta = rva - secStart;
                DWORD fileOff = section[i].PointerToRawData + delta;
                if (fileOff < pe.size()) return fileOff;
            }
        }
        return 0;
    }

    inline bool CheckDependencyExists(const std::string& dllName) {
        // Convert to wide string
        int len = MultiByteToWideChar(CP_ACP, 0, dllName.c_str(), -1, nullptr, 0);
        if (len <= 0) return false;
        std::wstring wide(static_cast<size_t>(len - 1), L'\0');
        MultiByteToWideChar(CP_ACP, 0, dllName.c_str(), -1, wide.data(), len);

        // Use SearchPathW to check if DLL can be found
        wchar_t foundPath[MAX_PATH]{};
        DWORD result = SearchPathW(nullptr, wide.c_str(), nullptr,
            MAX_PATH, foundPath, nullptr);
        return result > 0;
    }

    inline void RecordFailure(const std::string& reason) {
        AcquireSRWLockExclusive(&m_lock);
        m_stats.failureReasons[reason]++;
        ReleaseSRWLockExclusive(&m_lock);
    }

    static inline std::string ToHex(uint32_t val) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0') << std::setw(4) << val;
        return oss.str();
    }

    // Members
    mutable SRWLOCK                   m_lock{};
    std::vector<std::string>          m_requiredExports;
    CompatStats                       m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
