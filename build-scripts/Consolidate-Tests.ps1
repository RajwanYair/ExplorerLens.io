# Consolidate-Tests.ps1 — Replaces boilerplate Sprint 397-399 tests with data-driven bulk tests
# Reduces ~135 trivial Init/GetName tests into 3 bulk tests, keeping 15 domain tests

$file = "$PSScriptRoot\..\Engine\Tests\EngineTests.cpp"
$lines = Get-Content $file

Write-Host "Original line count: $($lines.Count)"

# ============================================================================
# PART 1: Replace TEST() function blocks (lines 25385-26491 → consolidated)
# ============================================================================

# Line numbers are 1-based; array is 0-based
$beforeTests = $lines[0..25383]  # Lines 1-25384 (before Sprint 397 comment)

$s399DomainTestsEnd = 26491      # Last line of RegressionTestRunner test closing }
$afterTests = $lines[$s399DomainTestsEnd..($lines.Count - 1)]  # Lines 26492+

# Build replacement content for TEST() section
$testReplacement = @'
// ============================================================================
// Sprint 397-399: Consolidated Initialization Tests
// ============================================================================
// Bulk Init/GetName validation — replaces 135 individual boilerplate tests
// with 3 data-driven tests. Domain-specific tests kept individually below.

template<typename T>
void AssertInitPattern(const std::string& name) {
    T v;
    ASSERT(v.Initialize());
    ASSERT(v.IsInitialized());
    ASSERT(v.GetName() == name);
}

// Sprint 397: 50 modules — all use identical Initialize/IsInitialized/GetName
TEST(Test_S397_BulkInitialization) {
    // Utils
    AssertInitPattern<VersionMatrixValidator>("VersionMatrixValidator");
    AssertInitPattern<OrphanedFileScanner>("OrphanedFileScanner");
    AssertInitPattern<LinkageAuditEngine>("LinkageAuditEngine");
    AssertInitPattern<DependencyGraphValidator>("DependencyGraphValidator");
    AssertInitPattern<CompilerWarningTracker>("CompilerWarningTracker");
    AssertInitPattern<LibraryHashVerifier>("LibraryHashVerifier");
    AssertInitPattern<LibraryCompatMatrix>("LibraryCompatMatrix");
    AssertInitPattern<TestCorpusIndexer>("TestCorpusIndexer");
    AssertInitPattern<DocCoverageAnalyzer>("DocCoverageAnalyzer");
    AssertInitPattern<LatencyBreakdownReporter>("LatencyBreakdownReporter");
    // Core
    AssertInitPattern<ArchiveTypeExtractor>("ArchiveTypeExtractor");
    AssertInitPattern<ArchiveStreamFactory>("ArchiveStreamFactory");
    AssertInitPattern<PropertyStoreCache>("PropertyStoreCache");
    AssertInitPattern<RegistryWriteCoalescer>("RegistryWriteCoalescer");
    AssertInitPattern<CollapsibleSectionModel>("CollapsibleSectionModel");
    AssertInitPattern<FormatHealthDashboard>("FormatHealthDashboard");
    AssertInitPattern<SettingsProfileManager>("SettingsProfileManager");
    AssertInitPattern<PerformanceMetricAggregator>("PerformanceMetricAggregator");
    AssertInitPattern<OwnerDrawButtonRenderer>("OwnerDrawButtonRenderer");
    AssertInitPattern<SystemTrayContextProvider>("SystemTrayContextProvider");
    AssertInitPattern<DPILayoutTransformer>("DPILayoutTransformer");
    AssertInitPattern<FormatIconAtlas>("FormatIconAtlas");
    AssertInitPattern<WorkerPoolElasticScaler>("WorkerPoolElasticScaler");
    AssertInitPattern<SIMDInstructionAuditor>("SIMDInstructionAuditor");
    // Pipeline
    AssertInitPattern<NamedPipeMessageBus>("NamedPipeMessageBus");
    AssertInitPattern<ZeroCopyBufferChain>("ZeroCopyBufferChain");
    AssertInitPattern<ParallelDirectoryScanner>("ParallelDirectoryScanner");
    AssertInitPattern<BatchDemuxOptimizer>("BatchDemuxOptimizer");
    AssertInitPattern<IOReadaheadScheduler>("IOReadaheadScheduler");
    AssertInitPattern<AccessPatternRecorder>("AccessPatternRecorder");
    // GPU
    AssertInitPattern<ShaderWarmupEngine>("ShaderWarmupEngine");
    AssertInitPattern<LanczosKernelOptimizer>("LanczosKernelOptimizer");
    AssertInitPattern<HDRMetadataParser>("HDRMetadataParser");
    AssertInitPattern<GPUCommandBundler>("GPUCommandBundler");
    AssertInitPattern<GPUTimestampProfiler>("GPUTimestampProfiler");
    // Cache
    AssertInitPattern<PSOBlobSerializer>("PSOBlobSerializer");
    AssertInitPattern<CacheWarmingScheduler>("CacheWarmingScheduler");
    AssertInitPattern<CachePartitionRebalancer>("CachePartitionRebalancer");
    AssertInitPattern<CacheTTLManager>("CacheTTLManager");
    AssertInitPattern<CacheCompressionSelector>("CacheCompressionSelector");
    // Memory
    AssertInitPattern<BitmapPoolPartitioner>("BitmapPoolPartitioner");
    AssertInitPattern<MemoryTagAllocator>("MemoryTagAllocator");
    AssertInitPattern<AddressSpaceReserver>("AddressSpaceReserver");
    AssertInitPattern<CommitChargeTracker>("CommitChargeTracker");
    // Decoders
    AssertInitPattern<FreeTypeLayoutEngine>("FreeTypeLayoutEngine");
    AssertInitPattern<FFmpegDemuxerBridge>("FFmpegDemuxerBridge");
    AssertInitPattern<OpenJPEGStreamDecoder>("OpenJPEGStreamDecoder");
    AssertInitPattern<MemoryMappedBitmapReader>("MemoryMappedBitmapReader");
    // Plugin
    AssertInitPattern<PluginProcessWatchdog>("PluginProcessWatchdog");
    AssertInitPattern<PluginHealthReporter>("PluginHealthReporter");
}

// Sprint 398: 50 modules — all use identical Initialize/IsInitialized/GetName
TEST(Test_S398_BulkInitialization) {
    // Core
    AssertInitPattern<ThumbnailRotationCorrector>("ThumbnailRotationCorrector");
    AssertInitPattern<ShellOverlayDispatcher>("ShellOverlayDispatcher");
    AssertInitPattern<FileVersionTracker>("FileVersionTracker");
    AssertInitPattern<ContextMenuFormatter>("ContextMenuFormatter");
    AssertInitPattern<ProgressReportAggregator>("ProgressReportAggregator");
    AssertInitPattern<OutputColorSpaceMapper>("OutputColorSpaceMapper");
    AssertInitPattern<BatchAbortController>("BatchAbortController");
    AssertInitPattern<ExtensionConflictDetector>("ExtensionConflictDetector");
    AssertInitPattern<DecoderFallbackChain>("DecoderFallbackChain");
    AssertInitPattern<AsyncRegistrySnapshot>("AsyncRegistrySnapshot");
    // Pipeline
    AssertInitPattern<AdaptiveChunkSizer>("AdaptiveChunkSizer");
    AssertInitPattern<StreamingMipGenerator>("StreamingMipGenerator");
    AssertInitPattern<PipelineBackpressureValve>("PipelineBackpressureValve");
    AssertInitPattern<ExtractorPoolBalancer>("ExtractorPoolBalancer");
    AssertInitPattern<DecodeQueueInspector>("DecodeQueueInspector");
    AssertInitPattern<IOCompletionPortBridge>("IOCompletionPortBridge");
    AssertInitPattern<PipelineTelemetryEmitter>("PipelineTelemetryEmitter");
    AssertInitPattern<DecodeWatermarkStamper>("DecodeWatermarkStamper");
    // GPU
    AssertInitPattern<ShaderVariantSelector>("ShaderVariantSelector");
    AssertInitPattern<GPUMemoryDefragmenter>("GPUMemoryDefragmenter");
    AssertInitPattern<TextureSamplerCache>("TextureSamplerCache");
    AssertInitPattern<GPUBarrierOptimizer>("GPUBarrierOptimizer");
    AssertInitPattern<ShaderCompileListener>("ShaderCompileListener");
    AssertInitPattern<GPUVendorQuirksTable>("GPUVendorQuirksTable");
    // Cache
    AssertInitPattern<CachePrefetchOracle>("CachePrefetchOracle");
    AssertInitPattern<CacheSerializationCodec>("CacheSerializationCodec");
    AssertInitPattern<CacheSizeEstimator>("CacheSizeEstimator");
    AssertInitPattern<CacheInvalidationBroadcaster>("CacheInvalidationBroadcaster");
    AssertInitPattern<CacheHitRateAnalyzer>("CacheHitRateAnalyzer");
    AssertInitPattern<CacheReplicationStrategy>("CacheReplicationStrategy");
    // Memory
    AssertInitPattern<VirtualAllocTracker>("VirtualAllocTracker");
    AssertInitPattern<MemoryPageFaultMonitor>("MemoryPageFaultMonitor");
    AssertInitPattern<HeapFragmentationProbe>("HeapFragmentationProbe");
    AssertInitPattern<MemoryBudgetNegotiator>("MemoryBudgetNegotiator");
    AssertInitPattern<MemoryTrimPolicy>("MemoryTrimPolicy");
    // Decoders
    AssertInitPattern<ExifThumbnailExtractor>("ExifThumbnailExtractor");
    AssertInitPattern<ProgressiveDecodeStreamer>("ProgressiveDecodeStreamer");
    AssertInitPattern<DecoderCapabilityProbe>("DecoderCapabilityProbe");
    AssertInitPattern<MultiFrameDecodeRouter>("MultiFrameDecodeRouter");
    AssertInitPattern<ColorProfileValidator>("ColorProfileValidator");
    // Plugin
    AssertInitPattern<PluginEventBus>("PluginEventBus");
    AssertInitPattern<PluginSchemaValidator>("PluginSchemaValidator");
    AssertInitPattern<PluginFeatureToggle>("PluginFeatureToggle");
    AssertInitPattern<PluginAuditTrail>("PluginAuditTrail");
    AssertInitPattern<PluginSandboxMonitor>("PluginSandboxMonitor");
    // Utils
    AssertInitPattern<CrashReportBundler>("CrashReportBundler");
    AssertInitPattern<StartupTimingProfiler>("StartupTimingProfiler");
    AssertInitPattern<FeatureFlagRegistry>("FeatureFlagRegistry");
    AssertInitPattern<TelemetrySampler>("TelemetrySampler");
    AssertInitPattern<DeploymentVerifier>("DeploymentVerifier");
}

// Sprint 399: 35 boilerplate modules consolidated
TEST(Test_S399_BulkInitialization) {
    // Core
    AssertInitPattern<ThumbnailStreamMultiplexer>("ThumbnailStreamMultiplexer");
    AssertInitPattern<FileSystemWatchdog>("FileSystemWatchdog");
    AssertInitPattern<ShellBadgeRenderer>("ShellBadgeRenderer");
    AssertInitPattern<DynamicFormatRouter>("DynamicFormatRouter");
    AssertInitPattern<ThumbnailCacheWarmer>("ThumbnailCacheWarmer");
    AssertInitPattern<ContextualPreviewEngine>("ContextualPreviewEngine");
    AssertInitPattern<ConcurrentExtractScheduler>("ConcurrentExtractScheduler");
    AssertInitPattern<ThumbnailPrefetchOracle>("ThumbnailPrefetchOracle");
    AssertInitPattern<ShellPropertyStoreRouter>("ShellPropertyStoreRouter");
    // Pipeline
    AssertInitPattern<AdaptivePipelineScheduler>("AdaptivePipelineScheduler");
    AssertInitPattern<StreamingThumbnailEmitter>("StreamingThumbnailEmitter");
    AssertInitPattern<FormatDetectionOracle>("FormatDetectionOracle");
    AssertInitPattern<DecodeThroughputRegulator>("DecodeThroughputRegulator");
    AssertInitPattern<MemoryMappedPipelineStage>("MemoryMappedPipelineStage");
    AssertInitPattern<BatchPriorityScheduler>("BatchPriorityScheduler");
    AssertInitPattern<PipelineLatencyTracker>("PipelineLatencyTracker");
    // GPU
    AssertInitPattern<ShaderHotReloader>("ShaderHotReloader");
    AssertInitPattern<GPUFormatConverter>("GPUFormatConverter");
    AssertInitPattern<GPUThumbnailCompositor>("GPUThumbnailCompositor");
    // Cache
    AssertInitPattern<CachePredictiveLoader>("CachePredictiveLoader");
    AssertInitPattern<CacheDeduplicationEngine>("CacheDeduplicationEngine");
    AssertInitPattern<CacheMigrationManager>("CacheMigrationManager");
    AssertInitPattern<CacheVersionCoordinator>("CacheVersionCoordinator");
    AssertInitPattern<CacheDiagnosticReporter>("CacheDiagnosticReporter");
    // Memory
    AssertInitPattern<BitmapMemoryRecycler>("BitmapMemoryRecycler");
    AssertInitPattern<MemoryPressureResponder>("MemoryPressureResponder");
    AssertInitPattern<ZeroFragmentationHeap>("ZeroFragmentationHeap");
    AssertInitPattern<MemoryAllocationTracer>("MemoryAllocationTracer");
    AssertInitPattern<NUMANodeAllocator>("NUMANodeAllocator");
    // Decoders
    AssertInitPattern<AnimatedFormatDecoder>("AnimatedFormatDecoder");
    AssertInitPattern<EmbeddedThumbnailDecoder>("EmbeddedThumbnailDecoder");
    AssertInitPattern<MultipageDocumentDecoder>("MultipageDocumentDecoder");
    AssertInitPattern<SidecarMetadataDecoder>("SidecarMetadataDecoder");
    AssertInitPattern<ContainerPreviewDecoder>("ContainerPreviewDecoder");
    // AI
    AssertInitPattern<AutoOrientationCorrector>("AutoOrientationCorrector");
}

// ============================================================================
// Sprint 399: Domain-Specific Tests (15 tests with meaningful assertions)
// ============================================================================

TEST(Test_S399_FileSizeEstimator) {
    FileSizeEstimator f;
    ASSERT(f.Initialize());
    ASSERT(f.IsInitialized());
    ASSERT(f.GetName() == "FileSizeEstimator");
    ASSERT(f.GetConfig().maxHeaderRead == 4096);
}

TEST(Test_S399_PipelineLoadShedder) {
    PipelineLoadShedder p;
    ASSERT(p.Initialize());
    ASSERT(p.IsInitialized());
    ASSERT(p.GetName() == "PipelineLoadShedder");
    auto d = p.Evaluate(10, 100);
    ASSERT(d == PipelineLoadShedder::Decision::Accept);
}

TEST(Test_S399_GPUTextureMipChain) {
    GPUTextureMipChain g;
    ASSERT(g.Initialize());
    ASSERT(g.IsInitialized());
    ASSERT(g.GetName() == "GPUTextureMipChain");
    ASSERT(g.CalculateMipCount(1024, 1024) > 1);
}

TEST(Test_S399_GPUOccupancyCalculator) {
    GPUOccupancyCalculator g;
    ASSERT(g.Initialize());
    ASSERT(g.IsInitialized());
    ASSERT(g.GetName() == "GPUOccupancyCalculator");
    auto occ = g.Calculate(128, 32, 64);
    ASSERT(occ.theoreticalOccupancy >= 0.0f && occ.theoreticalOccupancy <= 1.0f);
}

TEST(Test_S399_ComputeDispatchOptimizer) {
    ComputeDispatchOptimizer c;
    ASSERT(c.Initialize());
    ASSERT(c.IsInitialized());
    ASSERT(c.GetName() == "ComputeDispatchOptimizer");
    auto d = c.CalculateOptimal(1920, 1080);
    ASSERT(d.x > 0 && d.y > 0);
}

TEST(Test_S399_CacheEvictionSimulator) {
    CacheEvictionSimulator c;
    ASSERT(c.Initialize());
    ASSERT(c.IsInitialized());
    ASSERT(c.GetName() == "CacheEvictionSimulator");
    auto r = c.SimulateLRU({ 1, 2, 3, 1, 2, 4, 5 });
    ASSERT(r.hits + r.misses == 7);
}

TEST(Test_S399_SmartCropPredictor) {
    SmartCropPredictor s;
    ASSERT(s.Initialize());
    ASSERT(s.IsInitialized());
    ASSERT(s.GetName() == "SmartCropPredictor");
    auto crop = s.PredictCrop(1920, 1080, 256, 256);
    ASSERT(crop.width > 0 && crop.height > 0);
}

TEST(Test_S399_ThumbnailQualityScorer) {
    ThumbnailQualityScorer t;
    ASSERT(t.Initialize());
    ASSERT(t.IsInitialized());
    ASSERT(t.GetName() == "ThumbnailQualityScorer");
    auto score = t.Score(256, 256, 0.5f, 0.3f);
    ASSERT(score.overall > 0.0f);
    ASSERT(t.IsAcceptable(score));
}

TEST(Test_S399_ContentClassifier) {
    ContentClassifier c;
    ASSERT(c.Initialize());
    ASSERT(c.IsInitialized());
    ASSERT(c.GetName() == "ContentClassifier");
    auto cat = c.Classify(1920, 1080, 0.5f, 0.1f);
    ASSERT(cat == ContentClassifier::Category::Landscape);
}

TEST(Test_S399_DuplicateImageDetector) {
    DuplicateImageDetector d;
    ASSERT(d.Initialize());
    ASSERT(d.IsInitialized());
    ASSERT(d.GetName() == "DuplicateImageDetector");
    ASSERT(d.IsExactMatch(0xDEADBEEF, 0xDEADBEEF));
    ASSERT(!d.IsExactMatch(0xDEADBEEF, 0xCAFEBABE));
    ASSERT(d.HammingDistance(0, 0) == 0);
}

TEST(Test_S399_PluginCapabilityNegotiator) {
    PluginCapabilityNegotiator p;
    ASSERT(p.Initialize());
    ASSERT(p.IsInitialized());
    ASSERT(p.GetName() == "PluginCapabilityNegotiator");
    ASSERT(p.IsAbiCompatible(2));
    auto r = p.Negotiate(2, true, false);
    ASSERT(r.compatible);
    ASSERT(r.agreedAbiVersion == 2);
}

TEST(Test_S399_PluginStateCoordinator) {
    PluginStateCoordinator p;
    ASSERT(p.Initialize());
    ASSERT(p.IsInitialized());
    ASSERT(p.GetName() == "PluginStateCoordinator");
    ASSERT(p.Activate("test-plugin"));
    ASSERT(p.GetActiveCount() == 1);
    ASSERT(p.Deactivate("test-plugin"));
    ASSERT(p.GetActiveCount() == 0);
}

TEST(Test_S399_PluginCommunicationBridge) {
    PluginCommunicationBridge p;
    ASSERT(p.Initialize());
    ASSERT(p.IsInitialized());
    ASSERT(p.GetName() == "PluginCommunicationBridge");
    ASSERT(p.Broadcast("sender", "topic", 42));
    ASSERT(p.GetMessagesSent() == 1);
}

TEST(Test_S399_DiagnosticBundleCollector) {
    DiagnosticBundleCollector d;
    ASSERT(d.Initialize());
    ASSERT(d.IsInitialized());
    ASSERT(d.GetName() == "DiagnosticBundleCollector");
    auto bundle = d.Collect();
    ASSERT(d.IsComplete(bundle));
    ASSERT(bundle.engineVersion == "15.0.0");
}

TEST(Test_S399_RegressionTestRunner) {
    RegressionTestRunner r;
    ASSERT(r.Initialize());
    ASSERT(r.IsInitialized());
    ASSERT(r.GetName() == "RegressionTestRunner");
    auto suite = r.RunSuite({ "test1", "test2", "test3" });
    ASSERT(suite.totalTests == 3);
    ASSERT(suite.passRate == 100.0f);
}
'@

# ============================================================================
# PART 2: Replace RUN_TEST() calls in main() (lines 30590-30760 → consolidated)
# ============================================================================

# Line 30590 = "    // Sprint 397: Enhancement Plan V15 — Phase 2"
# Line 30760 = "    RUN_TEST(Test_S399_RegressionTestRunner);"
# Line 30762 = "    std::wcout << std::endl;"
# Line 30764 = "    // Isolation & Stability Tests"

$runTestStart = 30589   # 0-based: line 30590
$runTestEnd = 30761     # 0-based: line 30762 (the endl before Isolation)

$beforeRunTests = $afterTests[0..($runTestStart - $s399DomainTestsEnd - 1)]
$afterRunTests = $afterTests[($runTestEnd - $s399DomainTestsEnd)..($afterTests.Count - 1)]

$runTestReplacement = @'
    // Sprint 397-399: Consolidated Initialization Tests
    std::wcout << std::endl;
    std::wcout << L"Sprint 397-399: Consolidated Init Tests..." << std::endl;
    RUN_TEST(Test_S397_BulkInitialization);
    RUN_TEST(Test_S398_BulkInitialization);
    RUN_TEST(Test_S399_BulkInitialization);

    // Sprint 399: Domain-Specific Tests
    std::wcout << std::endl;
    std::wcout << L"Sprint 399: Domain Tests..." << std::endl;
    RUN_TEST(Test_S399_FileSizeEstimator);
    RUN_TEST(Test_S399_PipelineLoadShedder);
    RUN_TEST(Test_S399_GPUTextureMipChain);
    RUN_TEST(Test_S399_GPUOccupancyCalculator);
    RUN_TEST(Test_S399_ComputeDispatchOptimizer);
    RUN_TEST(Test_S399_CacheEvictionSimulator);
    RUN_TEST(Test_S399_SmartCropPredictor);
    RUN_TEST(Test_S399_ThumbnailQualityScorer);
    RUN_TEST(Test_S399_ContentClassifier);
    RUN_TEST(Test_S399_DuplicateImageDetector);
    RUN_TEST(Test_S399_PluginCapabilityNegotiator);
    RUN_TEST(Test_S399_PluginStateCoordinator);
    RUN_TEST(Test_S399_PluginCommunicationBridge);
    RUN_TEST(Test_S399_DiagnosticBundleCollector);
    RUN_TEST(Test_S399_RegressionTestRunner);
'@

# ============================================================================
# PART 3: Assemble final file
# ============================================================================

$newContent = @()
$newContent += $beforeTests
$newContent += $testReplacement -split "`n"
$newContent += $beforeRunTests
$newContent += $runTestReplacement -split "`n"
$newContent += $afterRunTests

# Write file
$newContent | Set-Content $file -Encoding UTF8

$reduction = $lines.Count - $newContent.Count
Write-Host "New line count: $($newContent.Count)"
Write-Host "Lines reduced: $reduction"
Write-Host "Tests consolidated: 150 boilerplate -> 3 bulk + 15 domain = 18 tests"
Write-Host "Net test reduction: 132 (3070 -> 2938)"
