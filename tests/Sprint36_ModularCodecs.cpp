//==============================================================================
// ExplorerLens — Sprint 36 Tests: Modular Codec DLLs & Memory Optimization
// Tests the per-format DLL architecture, lazy loading, memory budgeting,
// bitmap pools, decode buffer recycling, and directory-aware preloading.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_set>

// Headers under test
#include "../Engine/Codec/ICodecModule.h"
#include "../Engine/Codec/CodecLoader.h"
#include "../Engine/Codec/CodecModuleSpecs.h"
#include "../Engine/Codec/LazyCodecManager.h"
#include "../Engine/Memory/MemoryOptimizationEngine.h"

using namespace ExplorerLens::Engine::Codec;
using namespace ExplorerLens::Engine::Memory;

//==============================================================================
// ICodecModule ABI Tests
//==============================================================================

TEST(CodecABI, VersionConstants)
{
    EXPECT_EQ(DTCODEC_ABI_VERSION_MAJOR, 1);
    EXPECT_EQ(DTCODEC_ABI_VERSION_MINOR, 0);
}

TEST(CodecABI, PixelFormatValues)
{
    EXPECT_EQ(static_cast<int>(DT_PIXEL_BGRA32), 0);
    EXPECT_EQ(static_cast<int>(DT_PIXEL_BGR24), 1);
    EXPECT_EQ(static_cast<int>(DT_PIXEL_RGBA32), 2);
}

TEST(CodecABI, CapabilityFlags)
{
    uint32_t caps = DT_CAP_GPU_ACCEL | DT_CAP_ANIMATION | DT_CAP_THREAD_SAFE;
    EXPECT_TRUE(caps & DT_CAP_GPU_ACCEL);
    EXPECT_TRUE(caps & DT_CAP_ANIMATION);
    EXPECT_TRUE(caps & DT_CAP_THREAD_SAFE);
    EXPECT_FALSE(caps & DT_CAP_ARCHIVE);
    EXPECT_FALSE(caps & DT_CAP_HDR);
}

TEST(CodecABI, StructSizeVersioning)
{
    DtCodecModuleInfo info{};
    info.structSize = sizeof(DtCodecModuleInfo);
    EXPECT_GT(info.structSize, 0u);

    DtDecodeRequest request{};
    request.structSize = sizeof(DtDecodeRequest);
    EXPECT_GT(request.structSize, 0u);

    DtDecodeResult result{};
    result.structSize = sizeof(DtDecodeResult);
    EXPECT_GT(result.structSize, 0u);
}

TEST(CodecABI, FunctionNameConstants)
{
    EXPECT_STREQ(DTCODEC_FN_INITIALIZE, "DtCodec_Initialize");
    EXPECT_STREQ(DTCODEC_FN_GETMODULEINFO, "DtCodec_GetModuleInfo");
    EXPECT_STREQ(DTCODEC_FN_DECODETHUMBNAIL, "DtCodec_DecodeThumbnail");
    EXPECT_STREQ(DTCODEC_FN_FREERESULT, "DtCodec_FreeResult");
    EXPECT_STREQ(DTCODEC_FN_SHUTDOWN, "DtCodec_Shutdown");
    EXPECT_STREQ(DTCODEC_FN_GETHEALTH, "DtCodec_GetHealth");
}

TEST(CodecABI, DecodeRequestDefaults)
{
    DtDecodeRequest req{};
    req.structSize = sizeof(DtDecodeRequest);
    req.filePath = L"test.webp";
    req.maxWidth = 256;
    req.maxHeight = 256;
    req.flags = DT_DECODE_DEFAULT;

    EXPECT_EQ(req.maxWidth, 256u);
    EXPECT_EQ(req.maxHeight, 256u);
    EXPECT_EQ(req.flags, static_cast<uint32_t>(DT_DECODE_DEFAULT));
    EXPECT_EQ(req.reserved[0], 0u);
}

TEST(CodecABI, DecodeFlagsCombination)
{
    uint32_t flags = DT_DECODE_FAST_MODE | DT_DECODE_PRESERVE_ASPECT;
    EXPECT_TRUE(flags & DT_DECODE_FAST_MODE);
    EXPECT_TRUE(flags & DT_DECODE_PRESERVE_ASPECT);
    EXPECT_FALSE(flags & DT_DECODE_HIGH_QUALITY);
    EXPECT_FALSE(flags & DT_DECODE_FORCE_SDR);
}

//==============================================================================
// CodecModuleSpecs Tests
//==============================================================================

TEST(CodecSpecs, AllCodecsRegistered)
{
    auto specs = GetAllCodecSpecs();
    EXPECT_EQ(specs.size(), 17u);  // 17 codec DLLs defined
}

TEST(CodecSpecs, UniqueCodecIds)
{
    auto specs = GetAllCodecSpecs();
    std::unordered_set<std::string> ids;
    for (auto& s : specs) {
        EXPECT_TRUE(ids.insert(s.codecId).second)
            << "Duplicate codec ID: " << s.codecId;
    }
}

TEST(CodecSpecs, UniqueDllNames)
{
    auto specs = GetAllCodecSpecs();
    std::unordered_set<std::wstring> dlls;
    for (auto& s : specs) {
        EXPECT_TRUE(dlls.insert(s.dllName).second)
            << "Duplicate DLL name for codec: " << s.codecId;
    }
}

TEST(CodecSpecs, NoOverlappingExtensions)
{
    auto specs = GetAllCodecSpecs();
    std::unordered_map<std::wstring, std::string> extMap;
    for (auto& s : specs) {
        for (auto& ext : s.extensions) {
            auto it = extMap.find(ext);
            if (it != extMap.end()) {
                FAIL() << "Extension " << std::string(ext.begin(), ext.end())
                       << " claimed by both " << it->second
                       << " and " << s.codecId;
            }
            extMap[ext] = s.codecId;
        }
    }
}

TEST(CodecSpecs, WebPCodecDetails)
{
    auto specs = GetAllCodecSpecs();
    auto it = std::find_if(specs.begin(), specs.end(),
        [](const CodecModuleSpec& s) { return s.codecId == std::string("explorerlens.codec.webp"); });
    ASSERT_NE(it, specs.end());

    EXPECT_STREQ(it->dllName, L"ExplorerLens_Codec_WebP.dll");
    EXPECT_EQ(it->estimatedMemoryMB, 4u);
    EXPECT_TRUE(it->capabilities & DT_CAP_ANIMATION);
    EXPECT_TRUE(it->capabilities & DT_CAP_THREAD_SAFE);
    EXPECT_EQ(it->extensions.size(), 1u);
    EXPECT_EQ(it->extensions[0], L".webp");
}

TEST(CodecSpecs, RAWCodecExtensionCount)
{
    auto specs = GetAllCodecSpecs();
    auto it = std::find_if(specs.begin(), specs.end(),
        [](const CodecModuleSpec& s) { return s.codecId == std::string("explorerlens.codec.raw"); });
    ASSERT_NE(it, specs.end());

    // RAW codec should cover 23+ camera format extensions
    EXPECT_GE(it->extensions.size(), 20u);
    EXPECT_EQ(it->estimatedMemoryMB, 18u);
    EXPECT_TRUE(it->requiresExternalLib);
}

TEST(CodecSpecs, ArchiveCodecCapabilities)
{
    auto specs = GetAllCodecSpecs();
    auto it = std::find_if(specs.begin(), specs.end(),
        [](const CodecModuleSpec& s) { return s.codecId == std::string("explorerlens.codec.archive"); });
    ASSERT_NE(it, specs.end());

    EXPECT_TRUE(it->capabilities & DT_CAP_ARCHIVE);
    EXPECT_TRUE(it->capabilities & DT_CAP_MULTI_PAGE);
    EXPECT_GE(it->extensions.size(), 10u);
}

TEST(CodecSpecs, TotalExtensionCoverage)
{
    auto specs = GetAllCodecSpecs();
    uint32_t totalExts = 0;
    for (auto& s : specs) {
        totalExts += static_cast<uint32_t>(s.extensions.size());
    }
    // Should cover 80+ file extensions across all codecs
    EXPECT_GE(totalExts, 80u);
}

TEST(CodecSpecs, ManifestGeneration)
{
    std::string manifest = GenerateCodecManifest();

    // Valid JSON structure checks
    EXPECT_FALSE(manifest.empty());
    EXPECT_NE(manifest.find("\"version\": 1"), std::string::npos);
    EXPECT_NE(manifest.find("\"codecs\""), std::string::npos);
    EXPECT_NE(manifest.find("explorerlens.codec.webp"), std::string::npos);
    EXPECT_NE(manifest.find("explorerlens.codec.heif"), std::string::npos);
    EXPECT_NE(manifest.find("explorerlens.codec.jxl"), std::string::npos);
    EXPECT_NE(manifest.find("explorerlens.codec.raw"), std::string::npos);
    EXPECT_NE(manifest.find(".webp"), std::string::npos);
    EXPECT_NE(manifest.find("ExplorerLens_Codec_WebP.dll"), std::string::npos);
}

//==============================================================================
// Memory Impact Analysis Tests
//==============================================================================

TEST(MemoryImpact, JpegOnlyDirectory)
{
    // A folder of only JPEGs needs no codec DLLs (WIC handles it)
    auto report = AnalyzeMemoryImpact({L".jpg"});

    EXPECT_EQ(report.codecsLoaded, 0u);
    EXPECT_GE(report.codecsAvoided, 15u);
    EXPECT_GT(report.savingsPercent, 70.0);
    EXPECT_EQ(report.modularMemoryMB, 12u);  // WIC base only
}

TEST(MemoryImpact, MixedJpegWebPDirectory)
{
    auto report = AnalyzeMemoryImpact({L".jpg", L".webp"});

    EXPECT_EQ(report.codecsLoaded, 1u);   // Only WebP codec loaded
    EXPECT_GT(report.savingsPercent, 60.0);
    EXPECT_EQ(report.modularMemoryMB, 16u);  // WIC (12) + WebP (4)
}

TEST(MemoryImpact, RAWOnlyDirectory)
{
    auto report = AnalyzeMemoryImpact({L".cr2", L".nef"});

    // Both extensions map to the same RAW codec
    EXPECT_EQ(report.codecsLoaded, 1u);
    EXPECT_GT(report.savingsPercent, 50.0);
}

TEST(MemoryImpact, AllFormatsDirectory)
{
    // Worst case: a directory with every possible format
    auto specs = GetAllCodecSpecs();
    std::vector<std::wstring> allExts;
    for (auto& s : specs) {
        for (auto& e : s.extensions) allExts.push_back(e);
    }

    auto report = AnalyzeMemoryImpact(allExts);
    EXPECT_EQ(report.codecsLoaded, 17u);
    EXPECT_EQ(report.codecsAvoided, 0u);
    EXPECT_DOUBLE_EQ(report.savingsPercent, 0.0);
}

//==============================================================================
// CodecLoader Tests
//==============================================================================

TEST(CodecLoader, DefaultConfig)
{
    CodecLoaderConfig config;
    EXPECT_EQ(config.memoryBudgetBytes, 128ULL * 1024 * 1024);
    EXPECT_EQ(config.idleTimeoutMs, 5u * 60 * 1000);
    EXPECT_TRUE(config.autoEvict);
    EXPECT_TRUE(config.preloadWIC);
}

TEST(CodecLoader, InitializeWithNoManifest)
{
    CodecLoaderConfig config;
    config.codecDirectory = L"C:\\nonexistent\\path";
    config.manifestPath = L"C:\\nonexistent\\codec-manifest.json";

    CodecLoader loader(config);
    uint32_t err = loader.Initialize();
    EXPECT_EQ(err, 0u);  // Init succeeds even without manifest (no codecs registered)

    auto codecs = loader.GetRegisteredCodecs();
    EXPECT_TRUE(codecs.empty());

    auto stats = loader.GetStats();
    EXPECT_EQ(stats.totalLoads, 0u);
    EXPECT_EQ(stats.currentLoadedCodecs, 0u);
}

TEST(CodecLoader, ExtensionLookupMissing)
{
    CodecLoader loader;
    loader.Initialize();

    // No manifest loaded, so no codecs registered
    auto codecId = loader.FindCodecForExtension(L".zzz");
    EXPECT_TRUE(codecId.empty());
}

TEST(CodecLoader, StatsInitialState)
{
    CodecLoader loader;
    loader.Initialize();

    auto stats = loader.GetStats();
    EXPECT_EQ(stats.totalLoads, 0u);
    EXPECT_EQ(stats.totalUnloads, 0u);
    EXPECT_EQ(stats.totalDecodes, 0u);
    EXPECT_EQ(stats.totalErrors, 0u);
    EXPECT_EQ(stats.loadFailures, 0u);
    EXPECT_EQ(stats.evictions, 0u);
    EXPECT_EQ(stats.currentLoadedCodecs, 0u);
    EXPECT_EQ(stats.currentMemoryBytes, 0u);
}

TEST(CodecLoader, CodecStateEnum)
{
    EXPECT_NE(CodecState::Discovered, CodecState::Loading);
    EXPECT_NE(CodecState::Ready, CodecState::Error);
    EXPECT_NE(CodecState::Unloaded, CodecState::Ready);
}

//==============================================================================
// CodecHandle Tests
//==============================================================================

TEST(CodecHandle, InitialState)
{
    CodecHandle handle;
    EXPECT_EQ(handle.hModule, nullptr);
    EXPECT_EQ(handle.state, CodecState::Discovered);
    EXPECT_EQ(handle.decodeCount.load(), 0u);
    EXPECT_EQ(handle.errorCount.load(), 0u);
    EXPECT_FALSE(handle.IsLoaded());
}

TEST(CodecHandle, TimestampTracking)
{
    CodecHandle handle;
    EXPECT_EQ(handle.lastUseTimestamp.load(), 0u);

    handle.TouchTimestamp();
    uint64_t ts1 = handle.lastUseTimestamp.load();
    EXPECT_GT(ts1, 0u);

    // Second touch should be >= first
    handle.TouchTimestamp();
    uint64_t ts2 = handle.lastUseTimestamp.load();
    EXPECT_GE(ts2, ts1);
}

TEST(CodecHandle, IdleTimeCalculation)
{
    CodecHandle handle;
    // Never used → idle time should be MAX
    EXPECT_EQ(handle.IdleMs(), UINT64_MAX);

    handle.TouchTimestamp();
    // Just touched → idle time should be very small
    uint64_t idle = handle.IdleMs();
    EXPECT_LT(idle, 1000u);  // Less than 1 second
}

//==============================================================================
// LazyCodecManager Tests
//==============================================================================

TEST(LazyCodecManager, DefaultConfig)
{
    LazyCodecManagerConfig config;
    EXPECT_EQ(config.preloadStrategy, PreloadStrategy::SingleFormat);
    EXPECT_EQ(config.topNPreload, 3u);
    EXPECT_EQ(config.maxCensusScanFiles, 5000u);
    EXPECT_TRUE(config.enableMemoryPressureMonitor);
    EXPECT_TRUE(config.autoConvertToHBitmap);
}

TEST(LazyCodecManager, PreloadStrategyEnum)
{
    EXPECT_NE(PreloadStrategy::None, PreloadStrategy::SingleFormat);
    EXPECT_NE(PreloadStrategy::TopN, PreloadStrategy::All);
}

//==============================================================================
// DirectoryFormatCensus Tests
//==============================================================================

TEST(DirectoryFormatCensus, DominantExtension)
{
    DirectoryFormatCensus census;
    census.extensionCounts[L".jpg"] = 100;
    census.extensionCounts[L".png"] = 5;
    census.extensionCounts[L".webp"] = 2;
    census.totalFiles = 107;

    EXPECT_EQ(census.GetDominantExtension(), L".jpg");
}

TEST(DirectoryFormatCensus, IsSingleFormat)
{
    DirectoryFormatCensus census;
    census.extensionCounts[L".jpg"] = 98;
    census.extensionCounts[L".png"] = 2;
    census.totalFiles = 100;

    EXPECT_TRUE(census.IsSingleFormat(0.95));
    EXPECT_TRUE(census.IsSingleFormat(0.98));
    EXPECT_FALSE(census.IsSingleFormat(0.99));
}

TEST(DirectoryFormatCensus, UniqueExtensionList)
{
    DirectoryFormatCensus census;
    census.extensionCounts[L".cr2"] = 10;
    census.extensionCounts[L".nef"] = 5;
    census.extensionCounts[L".arw"] = 3;
    census.totalFiles = 18;

    auto exts = census.GetUniqueExtensions();
    EXPECT_EQ(exts.size(), 3u);
}

TEST(DirectoryFormatCensus, EmptyDirectory)
{
    DirectoryFormatCensus census;
    census.totalFiles = 0;

    EXPECT_TRUE(census.IsSingleFormat());
    EXPECT_TRUE(census.GetDominantExtension().empty());
    EXPECT_EQ(census.GetUniqueExtensions().size(), 0u);
    EXPECT_EQ(census.GetDistinctCodecCount(), 0u);
}

TEST(DirectoryFormatCensus, DistinctCodecCount)
{
    DirectoryFormatCensus census;
    // .cr2 and .nef both map to the RAW codec
    census.extensionCounts[L".cr2"] = 10;
    census.extensionCounts[L".nef"] = 5;
    census.extensionCounts[L".webp"] = 20;
    census.totalFiles = 35;

    auto count = census.GetDistinctCodecCount();
    EXPECT_EQ(count, 2u);  // RAW + WebP
}

//==============================================================================
// MemoryOptimizationEngine Tests
//==============================================================================

TEST(MemoryEngine, DefaultBudgetConfig)
{
    MemoryBudgetConfig config;
    EXPECT_EQ(config.maxWorkingSetBytes, 64ULL * 1024 * 1024);
    EXPECT_EQ(config.softLimitBytes, 51ULL * 1024 * 1024);
    EXPECT_EQ(config.bitmapPoolSize, 32u);
    EXPECT_EQ(config.decodeBufferPoolSize, 8u);
    EXPECT_EQ(config.decodeBufferSizeBytes, 256u * 256 * 4);
    EXPECT_TRUE(config.enforceWorkingSetLimits);
    EXPECT_TRUE(config.useMemoryMappedIO);
    EXPECT_TRUE(config.enableZeroCopy);
}

TEST(MemoryEngine, SubsystemNames)
{
    EXPECT_STREQ(SubsystemName(MemorySubsystem::Core), "Core");
    EXPECT_STREQ(SubsystemName(MemorySubsystem::DecoderWIC), "DecoderWIC");
    EXPECT_STREQ(SubsystemName(MemorySubsystem::DecoderCodec), "DecoderCodec");
    EXPECT_STREQ(SubsystemName(MemorySubsystem::ThumbnailCache), "ThumbnailCache");
    EXPECT_STREQ(SubsystemName(MemorySubsystem::BitmapPool), "BitmapPool");
    EXPECT_STREQ(SubsystemName(MemorySubsystem::DecodeBuffers), "DecodeBuffers");
    EXPECT_STREQ(SubsystemName(MemorySubsystem::GPU), "GPU");
}

TEST(MemoryEngine, SubsystemTracking)
{
    SubsystemMemory sm;
    EXPECT_EQ(sm.currentBytes.load(), 0);
    EXPECT_EQ(sm.peakBytes.load(), 0);
    EXPECT_EQ(sm.allocCount.load(), 0u);

    sm.TrackAlloc(1024);
    EXPECT_EQ(sm.currentBytes.load(), 1024);
    EXPECT_EQ(sm.peakBytes.load(), 1024);
    EXPECT_EQ(sm.allocCount.load(), 1u);

    sm.TrackAlloc(2048);
    EXPECT_EQ(sm.currentBytes.load(), 3072);
    EXPECT_EQ(sm.peakBytes.load(), 3072);

    sm.TrackFree(1024);
    EXPECT_EQ(sm.currentBytes.load(), 2048);
    EXPECT_EQ(sm.peakBytes.load(), 3072);  // Peak unchanged
    EXPECT_EQ(sm.freeCount.load(), 1u);
}

TEST(MemoryEngine, InitializeAndReport)
{
    MemoryBudgetConfig config;
    config.bitmapPoolSize = 4;
    config.decodeBufferPoolSize = 2;

    MemoryOptimizationEngine engine(config);
    engine.Initialize();

    std::string report = engine.GetMemoryReport();
    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("ExplorerLens Memory Report"), std::string::npos);
    EXPECT_NE(report.find("BitmapPool"), std::string::npos);
    EXPECT_NE(report.find("Trims"), std::string::npos);

    engine.Shutdown();
}

TEST(MemoryEngine, DecodeBufferPool)
{
    DecodeBufferPool pool(1024, 4);
    pool.Initialize();

    // Acquire buffers
    uint8_t* buf1 = pool.Acquire(1024);
    EXPECT_NE(buf1, nullptr);
    EXPECT_EQ(pool.GetActiveCount(), 1u);
    EXPECT_EQ(pool.GetAvailableCount(), 3u);

    uint8_t* buf2 = pool.Acquire(1024);
    EXPECT_NE(buf2, nullptr);
    EXPECT_NE(buf1, buf2);
    EXPECT_EQ(pool.GetActiveCount(), 2u);

    // Release
    pool.Release(buf1);
    EXPECT_EQ(pool.GetActiveCount(), 1u);
    EXPECT_EQ(pool.GetAvailableCount(), 3u);

    // Oversized request returns nullptr
    uint8_t* oversized = pool.Acquire(2048);
    EXPECT_EQ(oversized, nullptr);

    pool.Clear();
}

TEST(MemoryEngine, BitmapPoolAcquireRelease)
{
    BitmapPool pool(64, 64, 4);
    pool.Initialize();

    EXPECT_EQ(pool.GetPoolSize(), 4u);
    EXPECT_EQ(pool.GetActiveCount(), 0u);

    auto bmp = pool.Acquire(64, 64);
    EXPECT_NE(bmp.hBitmap, nullptr);
    EXPECT_NE(bmp.bits, nullptr);
    EXPECT_EQ(bmp.width, 64u);
    EXPECT_EQ(bmp.height, 64u);
    EXPECT_EQ(pool.GetActiveCount(), 1u);

    pool.Release(bmp);
    EXPECT_EQ(pool.GetActiveCount(), 0u);

    pool.Clear();
}

TEST(MemoryEngine, WorkingSetSnapshot)
{
    auto snap = WorkingSetMonitor::GetCurrentSnapshot();

    // Process should have some working set
    EXPECT_GT(snap.workingSetBytes, 0u);
    EXPECT_GT(snap.workingSetMB, 0.0);
    EXPECT_GT(snap.peakWorkingSetBytes, 0u);
}

TEST(MemoryEngine, MemoryMappedFileNonexistent)
{
    MemoryMappedFile mmf;
    bool ok = mmf.Open(L"C:\\nonexistent\\file.jpg");
    EXPECT_FALSE(ok);
    EXPECT_FALSE(mmf.IsOpen());
    EXPECT_EQ(mmf.Data(), nullptr);
    EXPECT_EQ(mmf.Size(), 0u);
}

TEST(MemoryEngine, BudgetCheckIntegration)
{
    MemoryBudgetConfig config;
    config.maxWorkingSetBytes = 1024ULL * 1024 * 1024;  // 1 GB — unlikely to exceed
    config.enforceWorkingSetLimits = false;

    MemoryOptimizationEngine engine(config);
    engine.Initialize();

    EXPECT_TRUE(engine.IsWithinBudget());
    EXPECT_FALSE(engine.IsNearSoftLimit());

    engine.Shutdown();
}

//==============================================================================
// Integration: Codec System + Memory Engine
//==============================================================================

TEST(Integration, CodecSpecsMemoryEstimates)
{
    auto specs = GetAllCodecSpecs();

    // Total monolithic memory should be significant
    uint64_t totalMB = 0;
    for (auto& s : specs) {
        totalMB += s.estimatedMemoryMB;
    }
    EXPECT_GE(totalMB, 50u);  // At least 50 MB total across all codecs

    // Modular system with only JPEG should use fraction
    auto report = AnalyzeMemoryImpact({L".jpg"});
    EXPECT_LT(report.modularMemoryMB, report.monolithicMemoryMB);
}

TEST(Integration, AllCodecsHaveThreadSafety)
{
    auto specs = GetAllCodecSpecs();
    for (auto& s : specs) {
        EXPECT_TRUE(s.capabilities & DT_CAP_THREAD_SAFE)
            << s.codecId << " is not marked thread-safe";
    }
}

TEST(Integration, ExternalLibCodecsHaveDependencies)
{
    auto specs = GetAllCodecSpecs();
    for (auto& s : specs) {
        if (s.requiresExternalLib) {
            EXPECT_FALSE(s.dependencies.empty())
                << s.codecId << " requires external lib but lists no dependencies";
        }
    }
}

TEST(Integration, PriorityOrdering)
{
    auto specs = GetAllCodecSpecs();

    // Archive codec should have highest priority (lowest number)
    auto archiveIt = std::find_if(specs.begin(), specs.end(),
        [](const CodecModuleSpec& s) { return s.codecId == std::string("explorerlens.codec.archive"); });
    ASSERT_NE(archiveIt, specs.end());
    EXPECT_LE(archiveIt->priority, 10);

    // Document/Font codecs should have lower priority (higher number)
    auto docIt = std::find_if(specs.begin(), specs.end(),
        [](const CodecModuleSpec& s) { return s.codecId == std::string("explorerlens.codec.document"); });
    ASSERT_NE(docIt, specs.end());
    EXPECT_GE(docIt->priority, 50);
}

//==============================================================================
// Main
//==============================================================================
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

