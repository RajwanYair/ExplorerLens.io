// PSOPrecompiler.h — Pipeline State Object Precompilation
// Copyright (c) 2026 ExplorerLens Project
//
// Pre-compiles pipeline state objects (PSOs) at startup or idle to avoid
// runtime stalls during rendering. Supports serialization to disk cache.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

enum class PSOPrecompileStatus : uint32_t {
    Pending = 0,
    Compiling = 1,
    Succeeded = 2,
    Failed = 3,
    Cached = 4,
    Evicted = 5
};

struct PSOPrecompileJob {
    uint64_t            jobId = 0;
    std::string         shaderName;
    uint32_t            vertexFormat = 0;
    uint32_t            blendState = 0;
    PSOPrecompileStatus status = PSOPrecompileStatus::Pending;
    uint64_t            compileTimeUs = 0;
    uint64_t            blobSizeBytes = 0;
    std::vector<uint8_t> cachedBlob;
};

class PSOPrecompiler {
public:
    static PSOPrecompiler& Instance() {
        static PSOPrecompiler s;
        return s;
    }

    uint64_t SubmitJob(const std::string& shaderName, uint32_t vertexFmt, uint32_t blendSt) {
        std::lock_guard<std::mutex> lock(m_mutex);
        PSOPrecompileJob job;
        job.jobId = m_nextJobId++;
        job.shaderName = shaderName;
        job.vertexFormat = vertexFmt;
        job.blendState = blendSt;
        job.status = PSOPrecompileStatus::Pending;
        m_jobs.push_back(job);
        return job.jobId;
    }

    bool Precompile(uint64_t jobId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = FindJob(jobId);
        if (it == m_jobs.end()) return false;

        it->status = PSOPrecompileStatus::Compiling;
        auto start = std::chrono::high_resolution_clock::now();

        // Simulate PSO compilation: hash shader params to create blob
        uint32_t hash = HashParams(it->shaderName, it->vertexFormat, it->blendState);
        it->cachedBlob.resize(64);
        for (size_t i = 0; i < 64; ++i)
            it->cachedBlob[i] = static_cast<uint8_t>((hash >> (i % 4 * 8)) & 0xFF);

        auto end = std::chrono::high_resolution_clock::now();
        it->compileTimeUs = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        it->blobSizeBytes = it->cachedBlob.size();
        it->status = PSOPrecompileStatus::Succeeded;
        m_totalCompiled++;
        return true;
    }

    double GetProgress() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_jobs.empty()) return 1.0;
        size_t done = 0;
        for (const auto& j : m_jobs) {
            if (j.status == PSOPrecompileStatus::Succeeded ||
                j.status == PSOPrecompileStatus::Cached)
                done++;
        }
        return static_cast<double>(done) / m_jobs.size();
    }

    bool SerializeToDisk(const std::wstring& path) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        uint32_t count = static_cast<uint32_t>(m_jobs.size());
        DWORD written = 0;
        WriteFile(hFile, &count, sizeof(count), &written, nullptr);

        for (const auto& j : m_jobs) {
            if (j.status == PSOPrecompileStatus::Succeeded && !j.cachedBlob.empty()) {
                uint32_t blobSize = static_cast<uint32_t>(j.cachedBlob.size());
                WriteFile(hFile, &blobSize, sizeof(blobSize), &written, nullptr);
                WriteFile(hFile, j.cachedBlob.data(), blobSize, &written, nullptr);
            }
        }
        CloseHandle(hFile);
        return true;
    }

    PSOPrecompileStatus GetJobStatus(uint64_t jobId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = FindJobConst(jobId);
        return it != m_jobs.end() ? it->status : PSOPrecompileStatus::Failed;
    }

    size_t GetTotalCompiled() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_totalCompiled;
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_jobs.clear();
        m_totalCompiled = 0;
        m_nextJobId = 1;
    }

    bool Validate() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& j : m_jobs) {
            if (j.status == PSOPrecompileStatus::Succeeded && j.cachedBlob.empty())
                return false;
            if (j.jobId == 0) return false;
        }
        double progress = m_jobs.empty() ? 1.0 :
            static_cast<double>(m_totalCompiled) / m_jobs.size();
        if (progress < 0.0 || progress > 1.0) return false;
        return true;
    }

private:
    PSOPrecompiler() = default;
    ~PSOPrecompiler() = default;
    PSOPrecompiler(const PSOPrecompiler&) = delete;
    PSOPrecompiler& operator=(const PSOPrecompiler&) = delete;

    std::vector<PSOPrecompileJob>::iterator FindJob(uint64_t id) {
        return std::find_if(m_jobs.begin(), m_jobs.end(),
            [id](const PSOPrecompileJob& j) { return j.jobId == id; });
    }

    std::vector<PSOPrecompileJob>::const_iterator FindJobConst(uint64_t id) const {
        return std::find_if(m_jobs.begin(), m_jobs.end(),
            [id](const PSOPrecompileJob& j) { return j.jobId == id; });
    }

    uint32_t HashParams(const std::string& name, uint32_t vf, uint32_t bs) const {
        uint32_t h = 2166136261u;
        for (char c : name) { h ^= static_cast<uint32_t>(c); h *= 16777619u; }
        h ^= vf; h *= 16777619u;
        h ^= bs; h *= 16777619u;
        return h;
    }

    mutable std::mutex m_mutex;
    std::vector<PSOPrecompileJob> m_jobs;
    uint64_t m_nextJobId = 1;
    size_t   m_totalCompiled = 0;
};

}
} // namespace ExplorerLens::Engine
