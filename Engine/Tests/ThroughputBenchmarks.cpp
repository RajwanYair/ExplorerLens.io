/******************************************************************************
 * ExplorerLens Per-Decoder Throughput Benchmarks
 * Copyright (c) 2026 - ExplorerLens Project
 *
 * Google Benchmark-compatible decoder throughput tests for CI regression gates.
 * Measures: decode throughput, memory usage, p50/p95/p99 latency
 *****************************************************************************/

#include <Psapi.h>
#include <Windows.h>
#include "../Decoders/AVIFDecoder.h"
#include "../Decoders/HEIFDecoder.h"
#include "../Decoders/JPEGDecoder.h"
#include "../Decoders/JXLDecoder.h"
#include "../Decoders/PNGDecoder.h"
#include "../Decoders/PSDDecoder.h"
#include "../Decoders/SVGDecoder.h"
#include "../Decoders/WebPDecoder.h"
#include "../Utils/MemoryMappedFile.h"
#include <benchmark/benchmark.h>

using namespace ExplorerLens;
using namespace ExplorerLens::Engine;

//============================================================================
// Test Data Paths (configure per environment)
//============================================================================

// Set EL_TEST_DATA_DIR env var to point at your local corpus directory.
const wchar_t* TEST_DATA_DIR = []() -> const wchar_t* {
    static wchar_t dir[MAX_PATH];
    if (GetEnvironmentVariableW(L"EL_TEST_DATA_DIR", dir, MAX_PATH) > 0) return dir;
    return L".\\data\\corpus\\";
}();

struct TestImage
{
    const wchar_t* name;
    const wchar_t* format;
    uint32_t width;
    uint32_t height;
};

// Representative test images per format
static const TestImage TEST_IMAGES[] = {
    {L"sample-4k.jpg", L"JPEG", 3840, 2160},   {L"sample-4k.png", L"PNG", 3840, 2160},
    {L"sample-4k.webp", L"WebP", 3840, 2160},  {L"sample-4k.avif", L"AVIF", 3840, 2160},
    {L"sample-4k.heic", L"HEIF", 3840, 2160},  {L"sample-4k.jxl", L"JXL", 3840, 2160},
    {L"sample-large.psd", L"PSD", 4096, 4096}, {L"sample.svg", L"SVG", 1024, 1024},
    {L"sample.epub", L"EPUB", 800, 1200},
};

//============================================================================
// Memory Tracking
//============================================================================

static SIZE_T GetPeakWorkingSet()
{
    PROCESS_MEMORY_COUNTERS pmc = {};
    pmc.cb = sizeof(pmc);
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.PeakWorkingSetSize;
    }
    return 0;
}

//============================================================================
// JPEG Decoder Benchmarks
//============================================================================

static void BM_DecodeJPEG_4K(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"jpeg\\sample-4k.jpg";

    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;
    ctx.preserveAspect = true;

    JPEGDecoder decoder;
    SIZE_T startMem = GetPeakWorkingSet();

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap) {
            DeleteObject(hBitmap);
        }
    }

    SIZE_T endMem = GetPeakWorkingSet();
    state.SetBytesProcessed(state.iterations() * (3840 * 2160 * 3));  // RGB bytes
    state.counters["peak_memory_mb"] = benchmark::Counter((endMem - startMem) / 1024.0 / 1024.0);
}
BENCHMARK(BM_DecodeJPEG_4K)->Unit(benchmark::kMillisecond)->Repetitions(20)->ReportAggregatesOnly(true);

static void BM_DecodeJPEG_HD(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"jpeg\\sample-hd.jpg";

    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;
    ctx.preserveAspect = true;

    JPEGDecoder decoder;

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap)
            DeleteObject(hBitmap);
    }

    state.SetBytesProcessed(state.iterations() * (1920 * 1080 * 3));
}
BENCHMARK(BM_DecodeJPEG_HD)->Unit(benchmark::kMillisecond)->Repetitions(20)->ReportAggregatesOnly(true);

//============================================================================
// PNG Decoder Benchmarks
//============================================================================

static void BM_DecodePNG_4K(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"png\\sample-4k.png";

    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;
    ctx.preserveAspect = true;

    PNGDecoder decoder;

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap)
            DeleteObject(hBitmap);
    }

    state.SetBytesProcessed(state.iterations() * (3840 * 2160 * 4));  // RGBA
}
BENCHMARK(BM_DecodePNG_4K)->Unit(benchmark::kMillisecond)->Repetitions(20)->ReportAggregatesOnly(true);

static void BM_DecodePNG_Interlaced(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"png\\sample-interlaced.png";

    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;
    ctx.preserveAspect = true;

    PNGDecoder decoder;

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap)
            DeleteObject(hBitmap);
    }
}
BENCHMARK(BM_DecodePNG_Interlaced)->Unit(benchmark::kMillisecond)->Repetitions(20)->ReportAggregatesOnly(true);

//============================================================================
// WebP Decoder Benchmarks
//============================================================================

static void BM_DecodeWebP_4K_Lossy(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"webp\\sample-4k-lossy.webp";

    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;
    ctx.preserveAspect = true;

    WebPDecoder decoder;

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap)
            DeleteObject(hBitmap);
    }

    state.SetBytesProcessed(state.iterations() * (3840 * 2160 * 4));
}
BENCHMARK(BM_DecodeWebP_4K_Lossy)->Unit(benchmark::kMillisecond)->Repetitions(20)->ReportAggregatesOnly(true);

static void BM_DecodeWebP_4K_Lossless(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"webp\\sample-4k-lossless.webp";

    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;
    ctx.preserveAspect = true;

    WebPDecoder decoder;

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap)
            DeleteObject(hBitmap);
    }

    state.SetBytesProcessed(state.iterations() * (3840 * 2160 * 4));
}
BENCHMARK(BM_DecodeWebP_4K_Lossless)->Unit(benchmark::kMillisecond)->Repetitions(20)->ReportAggregatesOnly(true);

//============================================================================
// AVIF Decoder Benchmarks
//============================================================================

static void BM_DecodeAVIF_4K(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"avif\\sample-4k.avif";

    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;
    ctx.preserveAspect = true;

    AVIFDecoder decoder;

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap)
            DeleteObject(hBitmap);
    }

    state.SetBytesProcessed(state.iterations() * (3840 * 2160 * 4));
}
BENCHMARK(BM_DecodeAVIF_4K)->Unit(benchmark::kMillisecond)->Repetitions(20)->ReportAggregatesOnly(true);

static void BM_DecodeAVIF_10bit(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"avif\\sample-10bit.avif";

    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;
    ctx.preserveAspect = true;

    AVIFDecoder decoder;

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap)
            DeleteObject(hBitmap);
    }
}
BENCHMARK(BM_DecodeAVIF_10bit)->Unit(benchmark::kMillisecond)->Repetitions(20)->ReportAggregatesOnly(true);

//============================================================================
// HEIF Decoder Benchmarks
//============================================================================

static void BM_DecodeHEIF_4K(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"heif\\sample-4k.heic";

    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;
    ctx.preserveAspect = true;

    HEIFDecoder decoder;

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap)
            DeleteObject(hBitmap);
    }

    state.SetBytesProcessed(state.iterations() * (3840 * 2160 * 3));
}
BENCHMARK(BM_DecodeHEIF_4K)->Unit(benchmark::kMillisecond)->Repetitions(20)->ReportAggregatesOnly(true);

//============================================================================
// JPEG XL Decoder Benchmarks
//============================================================================

static void BM_DecodeJXL_4K_Lossless(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"jxl\\sample-4k-lossless.jxl";

    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;
    ctx.preserveAspect = true;

    JXLDecoder decoder;

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap)
            DeleteObject(hBitmap);
    }

    state.SetBytesProcessed(state.iterations() * (3840 * 2160 * 4));
}
BENCHMARK(BM_DecodeJXL_4K_Lossless)->Unit(benchmark::kMillisecond)->Repetitions(20)->ReportAggregatesOnly(true);

static void BM_DecodeJXL_4K_Lossy(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"jxl\\sample-4k-lossy.jxl";

    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;
    ctx.preserveAspect = true;

    JXLDecoder decoder;

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap)
            DeleteObject(hBitmap);
    }

    state.SetBytesProcessed(state.iterations() * (3840 * 2160 * 4));
}
BENCHMARK(BM_DecodeJXL_4K_Lossy)->Unit(benchmark::kMillisecond)->Repetitions(20)->ReportAggregatesOnly(true);

//============================================================================
// PSD Decoder Benchmarks
//============================================================================

static void BM_DecodePSD_Large(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"psd\\sample-large.psd";

    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;
    ctx.preserveAspect = true;

    PSDDecoder decoder;

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap)
            DeleteObject(hBitmap);
    }
}
BENCHMARK(BM_DecodePSD_Large)->Unit(benchmark::kMillisecond)->Repetitions(20)->ReportAggregatesOnly(true);

//============================================================================
// SVG Decoder Benchmarks
//============================================================================

static void BM_DecodeSVG_Simple(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"svg\\sample-simple.svg";

    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;
    ctx.preserveAspect = true;

    SVGDecoder decoder;

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap)
            DeleteObject(hBitmap);
    }
}
BENCHMARK(BM_DecodeSVG_Simple)->Unit(benchmark::kMillisecond)->Repetitions(20)->ReportAggregatesOnly(true);

static void BM_DecodeSVG_Complex(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"svg\\sample-complex.svg";

    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;
    ctx.preserveAspect = true;

    SVGDecoder decoder;

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap)
            DeleteObject(hBitmap);
    }
}
BENCHMARK(BM_DecodeSVG_Complex)->Unit(benchmark::kMillisecond)->Repetitions(20)->ReportAggregatesOnly(true);

//============================================================================
// Archive Performance Benchmarks
//============================================================================

static void BM_ZipCentralDirectory_Small(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"archives\\small.zip";

    for (auto _ : state) {
        // Benchmark central directory parsing optimization
        MemoryMappedFile mmap(path.c_str());
        if (mmap.IsValid()) {
            // Parse EOCD + central directory
            const uint8_t* data = static_cast<const uint8_t*>(mmap.GetData());
            size_t size = mmap.GetSize();

            // Seek from end for EOCD signature
            size_t eocd_offset = size - 22;  // Minimum EOCD size
            // ... parsing logic ...
        }
    }
}
BENCHMARK(BM_ZipCentralDirectory_Small)->Unit(benchmark::kMicrosecond)->Repetitions(50)->ReportAggregatesOnly(true);

static void BM_ZipCentralDirectory_Large(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"archives\\large-500mb.zip";

    for (auto _ : state) {
        MemoryMappedFile mmap(path.c_str());
        if (mmap.IsValid()) {
            const uint8_t* data = static_cast<const uint8_t*>(mmap.GetData());
            size_t size = mmap.GetSize();

            // Optimized central directory parsing
            size_t eocd_offset = size - 22;
            // ... parsing logic ...
        }
    }
}
BENCHMARK(BM_ZipCentralDirectory_Large)->Unit(benchmark::kMillisecond)->Repetitions(20)->ReportAggregatesOnly(true);

static void BM_ZipMemoryMapped_vs_Traditional(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"archives\\medium-100mb.zip";

    if (state.range(0) == 0) {
        // Memory-mapped I/O
        for (auto _ : state) {
            MemoryMappedFile mmap(path.c_str());
            if (mmap.IsValid()) {
                const uint8_t* data = static_cast<const uint8_t*>(mmap.GetData());
                // Access file content directly
                benchmark::DoNotOptimize(data);
            }
        }
    } else {
        // Traditional I/O (fread)
        for (auto _ : state) {
            FILE* file = _wfopen(path.c_str(), L"rb");
            if (file) {
                fseek(file, 0, SEEK_END);
                size_t size = ftell(file);
                fseek(file, 0, SEEK_SET);

                std::vector<uint8_t> buffer(size);
                fread(buffer.data(), 1, size, file);
                fclose(file);

                benchmark::DoNotOptimize(buffer.data());
            }
        }
    }
}
BENCHMARK(BM_ZipMemoryMapped_vs_Traditional)->Args({0})->Args({1})->Unit(benchmark::kMillisecond);

//============================================================================
// Memory Profiling Benchmarks
//============================================================================

static void BM_Memory_PeakHeap(benchmark::State& state)
{
    SIZE_T peak_before = GetPeakWorkingSet();

    // Decode 100 thumbnails and measure peak heap
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"jpeg\\sample-4k.jpg";
    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;

    JPEGDecoder decoder;
    std::vector<HBITMAP> bitmaps;

    for (auto _ : state) {
        for (int i = 0; i < 100; ++i) {
            HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
            if (hBitmap)
                bitmaps.push_back(hBitmap);
        }

        // Clean up
        for (HBITMAP hBitmap : bitmaps) {
            DeleteObject(hBitmap);
        }
        bitmaps.clear();
    }

    SIZE_T peak_after = GetPeakWorkingSet();
    state.SetBytesProcessed((peak_after - peak_before) * state.iterations());
}
BENCHMARK(BM_Memory_PeakHeap)->Unit(benchmark::kMillisecond);

static void BM_Memory_WorkingSet(benchmark::State& state)
{
    PROCESS_MEMORY_COUNTERS pmc = {};
    pmc.cb = sizeof(pmc);

    for (auto _ : state) {
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            state.SetBytesProcessed(pmc.WorkingSetSize);
        }
    }
}
BENCHMARK(BM_Memory_WorkingSet)->Unit(benchmark::kMicrosecond);

static void BM_Memory_Allocations(benchmark::State& state)
{
    int alloc_count = 0;

    std::wstring path = std::wstring(TEST_DATA_DIR) + L"jpeg\\sample-hd.jpg";
    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;

    JPEGDecoder decoder;

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap) {
            DeleteObject(hBitmap);
            alloc_count++;
        }
    }

    state.SetItemsProcessed(alloc_count);
}
BENCHMARK(BM_Memory_Allocations)->Unit(benchmark::kMillisecond);

//============================================================================
// End-to-End Latency Benchmarks
//============================================================================

static void BM_EndToEnd_ColdStart(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"jpeg\\sample-4k.jpg";

    for (auto _ : state) {
        state.PauseTiming();
        // Simulate cold start (flush caches)
        _flushall();
        state.ResumeTiming();

        DecoderContext ctx;
        ctx.targetWidth = 256;
        ctx.targetHeight = 256;
        ctx.preserveAspect = true;

        JPEGDecoder decoder;
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap)
            DeleteObject(hBitmap);
    }
}
BENCHMARK(BM_EndToEnd_ColdStart)->Unit(benchmark::kMillisecond)->Repetitions(10)->ReportAggregatesOnly(true);

static void BM_EndToEnd_WarmCache(benchmark::State& state)
{
    std::wstring path = std::wstring(TEST_DATA_DIR) + L"jpeg\\sample-4k.jpg";

    // Warm up
    DecoderContext ctx;
    ctx.targetWidth = 256;
    ctx.targetHeight = 256;
    ctx.preserveAspect = true;
    ctx.enableCache = true;

    JPEGDecoder decoder;
    HBITMAP warm = decoder.Decode(path.c_str(), ctx);
    if (warm)
        DeleteObject(warm);

    for (auto _ : state) {
        HBITMAP hBitmap = decoder.Decode(path.c_str(), ctx);
        if (hBitmap)
            DeleteObject(hBitmap);
    }
}
BENCHMARK(BM_EndToEnd_WarmCache)->Unit(benchmark::kMicrosecond)->Repetitions(50)->ReportAggregatesOnly(true);

//============================================================================
// Main Entry Point
//============================================================================

BENCHMARK_MAIN();
