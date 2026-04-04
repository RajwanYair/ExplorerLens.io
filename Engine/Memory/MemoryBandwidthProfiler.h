// MemoryBandwidthProfiler.h — Memory Bandwidth Profiling
// Copyright (c) 2026 ExplorerLens Project
//
// Memory bandwidth profiling. Measures read/write bandwidth, detects bottlenecks,
// provides optimization recommendations for memory-intensive decode paths.
//
#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <numeric>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class MemoryAccessPattern : uint8_t {
    Sequential,
    Strided,
    Random,
    Mixed
};

enum class BandwidthBottleneck : uint8_t {
    None,
    ReadBandwidth,
    WriteBandwidth,
    Latency,
    CacheMiss,
    TLBMiss
};

struct BandwidthMeasurement
{
    double readBandwidthGBps = 0.0;
    double writeBandwidthGBps = 0.0;
    double copyBandwidthGBps = 0.0;
    double latencyNs = 0.0;
    size_t blockSize = 0;
    MemoryAccessPattern pattern = MemoryAccessPattern::Sequential;
};

struct ProfilingResult
{
    std::vector<BandwidthMeasurement> measurements;
    double peakReadGBps = 0.0;
    double peakWriteGBps = 0.0;
    BandwidthBottleneck bottleneck = BandwidthBottleneck::None;
    std::string recommendation;
};

class MemoryBandwidthProfiler
{
  public:
    static MemoryBandwidthProfiler& Instance()
    {
        static MemoryBandwidthProfiler instance;
        return instance;
    }

    inline BandwidthMeasurement MeasureReadBandwidth(size_t bufferSize = 64 * 1024 * 1024,
                                                     uint32_t iterations = 10) const
    {
        BandwidthMeasurement result;
        result.blockSize = bufferSize;
        result.pattern = MemoryAccessPattern::Sequential;

        std::vector<uint8_t> buffer(bufferSize, 0xAA);
        volatile uint64_t sink = 0;

        auto start = std::chrono::high_resolution_clock::now();
        for (uint32_t iter = 0; iter < iterations; ++iter) {
            uint64_t sum = 0;
            const uint64_t* ptr = reinterpret_cast<const uint64_t*>(buffer.data());
            size_t count = bufferSize / sizeof(uint64_t);
            for (size_t i = 0; i < count; ++i) {
                sum += ptr[i];
            }
            sink = sum;
        }
        auto end = std::chrono::high_resolution_clock::now();

        double elapsedSec = std::chrono::duration<double>(end - start).count();
        double totalBytes = static_cast<double>(bufferSize) * iterations;
        result.readBandwidthGBps = (totalBytes / (1024.0 * 1024.0 * 1024.0)) / elapsedSec;
        (void)sink;
        return result;
    }

    inline BandwidthMeasurement MeasureWriteBandwidth(size_t bufferSize = 64 * 1024 * 1024,
                                                      uint32_t iterations = 10) const
    {
        BandwidthMeasurement result;
        result.blockSize = bufferSize;
        result.pattern = MemoryAccessPattern::Sequential;

        std::vector<uint8_t> buffer(bufferSize);

        auto start = std::chrono::high_resolution_clock::now();
        for (uint32_t iter = 0; iter < iterations; ++iter) {
            std::memset(buffer.data(), static_cast<int>(iter & 0xFF), bufferSize);
        }
        auto end = std::chrono::high_resolution_clock::now();

        double elapsedSec = std::chrono::duration<double>(end - start).count();
        double totalBytes = static_cast<double>(bufferSize) * iterations;
        result.writeBandwidthGBps = (totalBytes / (1024.0 * 1024.0 * 1024.0)) / elapsedSec;
        return result;
    }

    inline BandwidthMeasurement MeasureCopyBandwidth(size_t bufferSize = 64 * 1024 * 1024,
                                                     uint32_t iterations = 10) const
    {
        BandwidthMeasurement result;
        result.blockSize = bufferSize;

        std::vector<uint8_t> src(bufferSize, 0xBB);
        std::vector<uint8_t> dst(bufferSize);

        auto start = std::chrono::high_resolution_clock::now();
        for (uint32_t iter = 0; iter < iterations; ++iter) {
            std::memcpy(dst.data(), src.data(), bufferSize);
        }
        auto end = std::chrono::high_resolution_clock::now();

        double elapsedSec = std::chrono::duration<double>(end - start).count();
        double totalBytes = static_cast<double>(bufferSize) * iterations;
        result.copyBandwidthGBps = (totalBytes / (1024.0 * 1024.0 * 1024.0)) / elapsedSec;
        return result;
    }

    inline ProfilingResult RunFullProfile(size_t maxBufferSize = 64 * 1024 * 1024) const
    {
        ProfilingResult result;

        size_t blockSizes[] = {4096, 65536, 1048576, 16 * 1048576, maxBufferSize};
        for (size_t bs : blockSizes) {
            if (bs > maxBufferSize)
                continue;
            auto read = MeasureReadBandwidth(bs, 5);
            auto write = MeasureWriteBandwidth(bs, 5);
            auto copy = MeasureCopyBandwidth(bs, 5);

            BandwidthMeasurement combined;
            combined.blockSize = bs;
            combined.readBandwidthGBps = read.readBandwidthGBps;
            combined.writeBandwidthGBps = write.writeBandwidthGBps;
            combined.copyBandwidthGBps = copy.copyBandwidthGBps;
            result.measurements.push_back(combined);

            if (read.readBandwidthGBps > result.peakReadGBps)
                result.peakReadGBps = read.readBandwidthGBps;
            if (write.writeBandwidthGBps > result.peakWriteGBps)
                result.peakWriteGBps = write.writeBandwidthGBps;
        }

        if (result.peakReadGBps < result.peakWriteGBps * 0.5) {
            result.bottleneck = BandwidthBottleneck::ReadBandwidth;
            result.recommendation = "Read bandwidth limited — consider prefetching or streaming reads";
        } else if (result.peakWriteGBps < result.peakReadGBps * 0.5) {
            result.bottleneck = BandwidthBottleneck::WriteBandwidth;
            result.recommendation = "Write bandwidth limited — consider write-combining or NT stores";
        } else {
            result.bottleneck = BandwidthBottleneck::None;
            result.recommendation = "Bandwidth is balanced";
        }
        return result;
    }

    inline std::string BottleneckToString(BandwidthBottleneck type) const
    {
        switch (type) {
            case BandwidthBottleneck::None:
                return "None";
            case BandwidthBottleneck::ReadBandwidth:
                return "Read Bandwidth";
            case BandwidthBottleneck::WriteBandwidth:
                return "Write Bandwidth";
            case BandwidthBottleneck::Latency:
                return "Latency";
            case BandwidthBottleneck::CacheMiss:
                return "Cache Miss";
            case BandwidthBottleneck::TLBMiss:
                return "TLB Miss";
            default:
                return "Unknown";
        }
    }

  private:
    MemoryBandwidthProfiler() = default;
};

}  // namespace Engine
}  // namespace ExplorerLens
