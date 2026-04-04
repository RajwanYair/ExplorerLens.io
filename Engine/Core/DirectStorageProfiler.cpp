// DirectStorageProfiler.cpp — Latency profiler for DirectStorage vs CPU decode paths
// Copyright (c) 2026 ExplorerLens Project

#include "DirectStorageProfiler.h"

#include <algorithm>
#include <numeric>

namespace ExplorerLens { namespace Engine {

DirectStorageProfiler& DirectStorageProfiler::Instance()
{
    static DirectStorageProfiler s_instance;
    return s_instance;
}

void DirectStorageProfiler::AddSample(const DSProfileSample& s) noexcept
{
    if (m_count < MAX_SAMPLES)
    {
        m_samples[m_count++] = s;
    }
    else
    {
        // Ring-wrap: overwrite oldest sample
        static uint32_t g_writeHead = 0;
        m_samples[g_writeHead % MAX_SAMPLES] = s;
        ++g_writeHead;
    }
}

DSProfileStats DirectStorageProfiler::ComputeStats() const noexcept
{
    DSProfileStats st{};
    if (m_count == 0) return st;

    // Collect total latencies into a small local array for percentile calculation
    float totals[MAX_SAMPLES]{};
    for (uint32_t i = 0; i < m_count; ++i)
    {
        totals[i] = m_samples[i].totalMs;
        if (m_samples[i].path == DSDecodePath::DIRECT_STORAGE) ++st.dsPathCount;
        else ++st.cpuPathCount;
    }

    std::sort(totals, totals + m_count);
    st.sampleCount = m_count;
    st.p50TotalMs  = totals[static_cast<uint32_t>(m_count * 0.50f)];
    st.p95TotalMs  = totals[static_cast<uint32_t>(m_count * 0.95f)];
    st.p99TotalMs  = totals[static_cast<uint32_t>(m_count * 0.99f)];
    return st;
}

void DirectStorageProfiler::Reset() noexcept
{
    m_count = 0;
}

DSDecodePath DirectStorageProfiler::RecommendedPath(uint32_t fileSizeBytes) const noexcept
{
    // DirectStorage preferred for files ≥ 4 MB where I/O bandwidth dominates
    constexpr uint32_t DS_THRESHOLD_BYTES = 4u * 1024u * 1024u;
    return (fileSizeBytes >= DS_THRESHOLD_BYTES)
        ? DSDecodePath::DIRECT_STORAGE
        : DSDecodePath::CPU_DECODE;
}

std::string_view DirectStorageProfiler::PathName(DSDecodePath p) noexcept
{
    switch (p)
    {
        case DSDecodePath::DIRECT_STORAGE: return "DirectStorage";
        default:                           return "CPUDecode";
    }
}

}} // namespace ExplorerLens::Engine
