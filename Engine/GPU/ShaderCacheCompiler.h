// ============================================================================
// ShaderCacheCompiler.h — Compile & Cache HLSL Compute Shaders to Disk
// Copyright (c) 2026 ExplorerLens Project
//
// PURPOSE
//   Compiles HLSL compute shaders to DXBC bytecode using D3DCompile at
//   runtime, and persists the compiled blobs to a disk cache so that
//   subsequent launches avoid the compile step entirely.  Cache keys are
//   SHA-256 hashes computed via the Windows CNG API (BCryptCreateHash /
//   BCryptHashData with BCRYPT_SHA256_ALGORITHM), guaranteeing correct
//   invalidation when shader source or compiler target changes.
//
// CLASSES
//   ShaderCacheCompiler — main entry point
//
// KEY API
//   CompileShader(hlslSource, entryPoint, target, outBytecode)
//       Compile HLSL to DXBC via D3DCompile.
//
//   SaveToCache(key, bytecode)  / LoadFromCache(key, outBytecode)
//   IsCached(key)
//       Disk read / write of compiled blob under
//       %LOCALAPPDATA%\ExplorerLens\ShaderCache\<sha256>.cso
//
//   SetCacheDirectory(dir)  — override default location
//   PurgeCache()            — delete all cached shaders
//   PurgeExpired(maxAgeDays)— delete shaders older than N days
//   GetStats()              — CacheStats (count, size, hits, misses)
//
// THREAD SAFETY
//   All public methods are guarded by SRWLOCK.
//
// DEPENDENCIES
//   Windows API only.  bcrypt.dll and d3dcompiler_47.dll dynamically loaded.
// ============================================================================
#pragma once

#include <windows.h>
#include <bcrypt.h>
#include <d3dcompiler.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cstdint>
#include <chrono>
#include <fstream>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// -----------------------------------------------------------------------
// CacheStats
// -----------------------------------------------------------------------
struct ShaderCacheStats {
    uint32_t cachedShaders = 0;
    uint64_t totalSizeBytes = 0;
    uint64_t cacheHits = 0;
    uint64_t cacheMisses = 0;
    double   compileTimeSavedMs = 0.0;
};

// -----------------------------------------------------------------------
// ShaderCacheCompiler
// -----------------------------------------------------------------------
class ShaderCacheCompiler {
public:
    ShaderCacheCompiler() = default;
    ~ShaderCacheCompiler() { Cleanup(); }

    ShaderCacheCompiler(const ShaderCacheCompiler&) = delete;
    ShaderCacheCompiler& operator=(const ShaderCacheCompiler&) = delete;

    // ================================================================
    // CompileShader
    // ================================================================
    inline bool CompileShader(const std::string& hlslSource,
        const std::string& entryPoint,
        const std::string& target,
        std::vector<uint8_t>& outBytecode) {
        AcquireExclusive();

        EnsureModules();
        if (!m_pfnCompile) { ReleaseExclusive(); return false; }

        auto t0 = std::chrono::steady_clock::now();

        ID3DBlob* blob = nullptr;
        ID3DBlob* err = nullptr;
        HRESULT hr = m_pfnCompile(
            hlslSource.data(), hlslSource.size(), nullptr, nullptr, nullptr,
            entryPoint.c_str(), target.c_str(),
            D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &blob, &err);
        if (err) err->Release();

        bool ok = SUCCEEDED(hr) && blob;
        if (ok) {
            const uint8_t* p = static_cast<const uint8_t*>(blob->GetBufferPointer());
            outBytecode.assign(p, p + blob->GetBufferSize());
            blob->Release();
        }

        auto t1 = std::chrono::steady_clock::now();
        m_lastCompileMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

        ReleaseExclusive();
        return ok;
    }

    // ================================================================
    // SHA-256 key generation via Windows CNG
    // ================================================================
    inline std::string MakeCacheKey(const std::string& source,
        const std::string& entryPoint,
        const std::string& target) {
        AcquireShared();
        EnsureModules();

        std::string combined = source + "|" + entryPoint + "|" + target;
        std::string hashHex = SHA256(combined);

        ReleaseShared();
        return hashHex;
    }

    // ================================================================
    // SaveToCache
    // ================================================================
    inline bool SaveToCache(const std::string& key,
        const std::vector<uint8_t>& bytecode) {
        AcquireExclusive();
        EnsureCacheDir();

        std::wstring path = m_cacheDir + L"\\" + ToWide(key) + L".cso";
        std::ofstream f(path, std::ios::binary);
        bool ok = false;
        if (f.is_open()) {
            f.write(reinterpret_cast<const char*>(bytecode.data()),
                static_cast<std::streamsize>(bytecode.size()));
            ok = f.good();
            f.close();
        }

        if (ok) {
            m_memCache[key] = bytecode;
            RefreshStats();
        }
        ReleaseExclusive();
        return ok;
    }

    // ================================================================
    // LoadFromCache
    // ================================================================
    inline bool LoadFromCache(const std::string& key,
        std::vector<uint8_t>& outBytecode) {
        AcquireShared();

        // in-memory first
        auto it = m_memCache.find(key);
        if (it != m_memCache.end()) {
            outBytecode = it->second;
            m_stats.cacheHits++;
            ReleaseShared();
            return true;
        }

        // disk
        std::wstring path = m_cacheDir + L"\\" + ToWide(key) + L".cso";
        std::ifstream f(path, std::ios::binary | std::ios::ate);
        if (!f.is_open()) {
            m_stats.cacheMisses++;
            ReleaseShared();
            return false;
        }
        auto sz = f.tellg();
        if (sz <= 0) { f.close(); m_stats.cacheMisses++; ReleaseShared(); return false; }
        outBytecode.resize(static_cast<size_t>(sz));
        f.seekg(0);
        f.read(reinterpret_cast<char*>(outBytecode.data()), sz);
        f.close();

        m_stats.cacheHits++;
        ReleaseShared();
        return true;
    }

    // ================================================================
    // IsCached
    // ================================================================
    inline bool IsCached(const std::string& key) {
        AcquireShared();
        bool found = m_memCache.count(key) > 0;
        if (!found) {
            std::wstring path = m_cacheDir + L"\\" + ToWide(key) + L".cso";
            DWORD attr = ::GetFileAttributesW(path.c_str());
            found = (attr != INVALID_FILE_ATTRIBUTES);
        }
        ReleaseShared();
        return found;
    }

    // ================================================================
    // SetCacheDirectory
    // ================================================================
    inline void SetCacheDirectory(const std::wstring& dir) {
        AcquireExclusive();
        m_cacheDir = dir;
        m_cacheDirReady = false;
        ReleaseExclusive();
    }

    // ================================================================
    // PurgeCache — delete everything
    // ================================================================
    inline void PurgeCache() {
        AcquireExclusive();
        m_memCache.clear();
        EnsureCacheDir();

        WIN32_FIND_DATAW fd{};
        std::wstring pattern = m_cacheDir + L"\\*.cso";
        HANDLE hFind = ::FindFirstFileW(pattern.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                std::wstring full = m_cacheDir + L"\\" + fd.cFileName;
                ::DeleteFileW(full.c_str());
            } while (::FindNextFileW(hFind, &fd));
            ::FindClose(hFind);
        }
        RefreshStats();
        ReleaseExclusive();
    }

    // ================================================================
    // PurgeExpired — delete shaders older than maxAgeDays
    // ================================================================
    inline void PurgeExpired(uint32_t maxAgeDays = 30) {
        AcquireExclusive();
        EnsureCacheDir();

        FILETIME now{};
        ::GetSystemTimeAsFileTime(&now);
        ULARGE_INTEGER nowU;
        nowU.LowPart = now.dwLowDateTime;
        nowU.HighPart = now.dwHighDateTime;
        uint64_t cutoff = nowU.QuadPart -
            static_cast<uint64_t>(maxAgeDays) * 24ULL * 3600ULL * 10000000ULL;

        WIN32_FIND_DATAW fd{};
        std::wstring pattern = m_cacheDir + L"\\*.cso";
        HANDLE hFind = ::FindFirstFileW(pattern.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                ULARGE_INTEGER ft;
                ft.LowPart = fd.ftLastWriteTime.dwLowDateTime;
                ft.HighPart = fd.ftLastWriteTime.dwHighDateTime;
                if (ft.QuadPart < cutoff) {
                    std::wstring full = m_cacheDir + L"\\" + fd.cFileName;
                    ::DeleteFileW(full.c_str());
                }
            } while (::FindNextFileW(hFind, &fd));
            ::FindClose(hFind);
        }
        RefreshStats();
        ReleaseExclusive();
    }

    // ================================================================
    // GetStats
    // ================================================================
    inline ShaderCacheStats GetStats() {
        AcquireShared();
        RefreshStats();
        ShaderCacheStats copy = m_stats;
        ReleaseShared();
        return copy;
    }

private:
    // ---- SRWLOCK wrappers ----
    SRWLOCK m_srw = SRWLOCK_INIT;
    inline void AcquireExclusive() { ::AcquireSRWLockExclusive(&m_srw); }
    inline void ReleaseExclusive() { ::ReleaseSRWLockExclusive(&m_srw); }
    inline void AcquireShared() { ::AcquireSRWLockShared(&m_srw); }
    inline void ReleaseShared() { ::ReleaseSRWLockShared(&m_srw); }

    // ---- dynamic modules ----
    HMODULE m_hBcrypt = nullptr;
    HMODULE m_hCompiler = nullptr;

    using PFN_D3DCompile = HRESULT(WINAPI*)(
        LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*,
        ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT,
        ID3DBlob**, ID3DBlob**);
    PFN_D3DCompile m_pfnCompile = nullptr;

    using PFN_BCryptOpenAlgorithmProvider = NTSTATUS(WINAPI*)(
        BCRYPT_ALG_HANDLE*, LPCWSTR, LPCWSTR, ULONG);
    using PFN_BCryptCreateHash = NTSTATUS(WINAPI*)(
        BCRYPT_ALG_HANDLE, BCRYPT_HASH_HANDLE*, PUCHAR, ULONG,
        PUCHAR, ULONG, ULONG);
    using PFN_BCryptHashData = NTSTATUS(WINAPI*)(
        BCRYPT_HASH_HANDLE, PUCHAR, ULONG, ULONG);
    using PFN_BCryptFinishHash = NTSTATUS(WINAPI*)(
        BCRYPT_HASH_HANDLE, PUCHAR, ULONG, ULONG);
    using PFN_BCryptDestroyHash = NTSTATUS(WINAPI*)(BCRYPT_HASH_HANDLE);
    using PFN_BCryptCloseAlgorithmProvider = NTSTATUS(WINAPI*)(
        BCRYPT_ALG_HANDLE, ULONG);

    PFN_BCryptOpenAlgorithmProvider m_pfnBCryptOpen = nullptr;
    PFN_BCryptCreateHash            m_pfnBCryptCreate = nullptr;
    PFN_BCryptHashData              m_pfnBCryptHashData = nullptr;
    PFN_BCryptFinishHash            m_pfnBCryptFinish = nullptr;
    PFN_BCryptDestroyHash           m_pfnBCryptDestroy = nullptr;
    PFN_BCryptCloseAlgorithmProvider m_pfnBCryptClose = nullptr;

    // ---- cache state ----
    std::wstring m_cacheDir;
    bool         m_cacheDirReady = false;
    std::unordered_map<std::string, std::vector<uint8_t>> m_memCache;
    ShaderCacheStats   m_stats{};
    double       m_lastCompileMs = 0.0;

    // ---- helpers ----
    inline void EnsureModules() {
        if (!m_hCompiler) {
            m_hCompiler = ::LoadLibraryW(L"d3dcompiler_47.dll");
            if (!m_hCompiler) m_hCompiler = ::LoadLibraryW(L"d3dcompiler_46.dll");
        }
        if (m_hCompiler && !m_pfnCompile) {
            m_pfnCompile = reinterpret_cast<PFN_D3DCompile>(
                ::GetProcAddress(m_hCompiler, "D3DCompile"));
        }
        if (!m_hBcrypt) {
            m_hBcrypt = ::LoadLibraryW(L"bcrypt.dll");
        }
        if (m_hBcrypt && !m_pfnBCryptOpen) {
            m_pfnBCryptOpen = reinterpret_cast<PFN_BCryptOpenAlgorithmProvider>(::GetProcAddress(m_hBcrypt, "BCryptOpenAlgorithmProvider"));
            m_pfnBCryptCreate = reinterpret_cast<PFN_BCryptCreateHash>(::GetProcAddress(m_hBcrypt, "BCryptCreateHash"));
            m_pfnBCryptHashData = reinterpret_cast<PFN_BCryptHashData>(::GetProcAddress(m_hBcrypt, "BCryptHashData"));
            m_pfnBCryptFinish = reinterpret_cast<PFN_BCryptFinishHash>(::GetProcAddress(m_hBcrypt, "BCryptFinishHash"));
            m_pfnBCryptDestroy = reinterpret_cast<PFN_BCryptDestroyHash>(::GetProcAddress(m_hBcrypt, "BCryptDestroyHash"));
            m_pfnBCryptClose = reinterpret_cast<PFN_BCryptCloseAlgorithmProvider>(::GetProcAddress(m_hBcrypt, "BCryptCloseAlgorithmProvider"));
        }
    }

    inline void EnsureCacheDir() {
        if (m_cacheDirReady && !m_cacheDir.empty()) return;

        if (m_cacheDir.empty()) {
            wchar_t buf[MAX_PATH]{};
            if (::GetEnvironmentVariableW(L"LOCALAPPDATA", buf, MAX_PATH) > 0) {
                m_cacheDir = std::wstring(buf) + L"\\ExplorerLens\\ShaderCache";
            }
            else {
                m_cacheDir = L"C:\\Temp\\ExplorerLens\\ShaderCache";
            }
        }

        // Create directory chain
        CreateDirectoryChain(m_cacheDir);
        m_cacheDirReady = true;
    }

    static inline void CreateDirectoryChain(const std::wstring& path) {
        for (size_t i = 0; i < path.size(); ++i) {
            if (path[i] == L'\\' || path[i] == L'/') {
                std::wstring sub = path.substr(0, i);
                if (!sub.empty() && sub.back() != L':')
                    ::CreateDirectoryW(sub.c_str(), nullptr);
            }
        }
        ::CreateDirectoryW(path.c_str(), nullptr);
    }

    inline std::string SHA256(const std::string& data) {
        EnsureModules();
        if (!m_pfnBCryptOpen || !m_pfnBCryptCreate || !m_pfnBCryptHashData ||
            !m_pfnBCryptFinish || !m_pfnBCryptDestroy || !m_pfnBCryptClose)
            return FallbackHash(data);

        BCRYPT_ALG_HANDLE hAlg = nullptr;
        NTSTATUS st = m_pfnBCryptOpen(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
        if (st != 0 || !hAlg) return FallbackHash(data);

        BCRYPT_HASH_HANDLE hHash = nullptr;
        st = m_pfnBCryptCreate(hAlg, &hHash, nullptr, 0, nullptr, 0, 0);
        if (st != 0 || !hHash) { m_pfnBCryptClose(hAlg, 0); return FallbackHash(data); }

        m_pfnBCryptHashData(hHash,
            reinterpret_cast<PUCHAR>(const_cast<char*>(data.data())),
            static_cast<ULONG>(data.size()), 0);

        uint8_t digest[32]{};
        m_pfnBCryptFinish(hHash, digest, 32, 0);
        m_pfnBCryptDestroy(hHash);
        m_pfnBCryptClose(hAlg, 0);

        return BytesToHex(digest, 32);
    }

    static inline std::string FallbackHash(const std::string& data) {
        // FNV-1a 64 fallback if BCrypt unavailable
        uint64_t h = 0xCBF29CE484222325ULL;
        for (char c : data) { h ^= static_cast<uint8_t>(c); h *= 0x100000001B3ULL; }
        char buf[17]{};
        snprintf(buf, sizeof(buf), "%016llx", static_cast<unsigned long long>(h));
        return std::string(buf);
    }

    static inline std::string BytesToHex(const uint8_t* data, size_t len) {
        static const char hex[] = "0123456789abcdef";
        std::string out;
        out.reserve(len * 2);
        for (size_t i = 0; i < len; ++i) {
            out.push_back(hex[data[i] >> 4]);
            out.push_back(hex[data[i] & 0x0F]);
        }
        return out;
    }

    static inline std::wstring ToWide(const std::string& s) {
        std::wstring w;
        w.reserve(s.size());
        for (char c : s) w.push_back(static_cast<wchar_t>(static_cast<unsigned char>(c)));
        return w;
    }

    inline void RefreshStats() {
        // Count disk files
        EnsureCacheDir();
        uint32_t count = 0;
        uint64_t totalSize = 0;

        WIN32_FIND_DATAW fd{};
        std::wstring pattern = m_cacheDir + L"\\*.cso";
        HANDLE hFind = ::FindFirstFileW(pattern.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                ULARGE_INTEGER sz;
                sz.LowPart = fd.nFileSizeLow;
                sz.HighPart = fd.nFileSizeHigh;
                totalSize += sz.QuadPart;
                count++;
            } while (::FindNextFileW(hFind, &fd));
            ::FindClose(hFind);
        }
        m_stats.cachedShaders = count;
        m_stats.totalSizeBytes = totalSize;
    }

    inline void Cleanup() {
        if (m_hCompiler) { ::FreeLibrary(m_hCompiler); m_hCompiler = nullptr; }
        if (m_hBcrypt) { ::FreeLibrary(m_hBcrypt);   m_hBcrypt = nullptr; }
        m_pfnCompile = nullptr;
        m_pfnBCryptOpen = nullptr;
    }
};

} // namespace Engine
} // namespace ExplorerLens
