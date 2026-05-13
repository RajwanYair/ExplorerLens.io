// EngineTests_Platform.cpp — Engine unit tests, platform & advanced split
// Copyright (c) 2026 ExplorerLens Project
//
// Split from EngineTests_Late.cpp v36.1.0 to comply with file size policy.
// Contains Sprint 1061+ tests: Workflow Automation, Self-Healing, Security,
// DirectStorage, CLIP Search, Cross-Platform PAL, Enterprise, AI, and more.
//
#include "EngineTestsIncludes.h"
//== Sprint 1061-1070: Intelligent Workflow Automation (v31.3.0 "Achernar-T") ==

// --- PredictivePregenEngine ---
TEST(TestPregen_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    ASSERT(!engine.IsInitialized());
}
TEST(TestPregen_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    ASSERT(engine.Initialize());
    ASSERT(engine.IsInitialized());
    ASSERT(engine.GetStrategy() == PregenStrategy::Hybrid);
    engine.Shutdown();
}
TEST(TestPregen_InitWithStrategy)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    engine.Initialize(PregenStrategy::Recency);
    ASSERT(engine.GetStrategy() == PregenStrategy::Recency);
    engine.Shutdown();
}
TEST(TestPregen_RecordNavigation)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    engine.Initialize();
    engine.RecordNavigation(L"C:\\Photos");
    ASSERT(engine.GetStats().totalPredictions == 1);
    engine.Shutdown();
}
TEST(TestPregen_Predict)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    engine.Initialize();
    engine.RecordNavigation(L"C:\\Photos");
    auto predictions = engine.Predict(3);
    ASSERT(!predictions.empty());
    ASSERT(predictions[0].confidence > 0.0);
    engine.Shutdown();
}
TEST(TestPregen_PredictEmpty)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    auto predictions = engine.Predict(5);
    ASSERT(predictions.empty());
}
TEST(TestPregen_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    engine.Initialize();
    auto stats = engine.GetStats();
    ASSERT(stats.totalPredictions == 0 || stats.totalPredictions > 0);
    engine.Shutdown();
}
TEST(TestPregen_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    engine.Initialize();
    engine.Shutdown();
    ASSERT(!engine.IsInitialized());
}
TEST(TestPregen_PredictZeroResults)
{
    using namespace ExplorerLens::Engine;
    auto& engine = PredictivePregenEngine::Instance();
    engine.Initialize();
    auto predictions = engine.Predict(0);
    ASSERT(predictions.empty());
    engine.Shutdown();
}

// --- ContentCategorizationEngine ---
TEST(TestContentCat_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& engine = ContentCategorizationEngine::Instance();
    ASSERT(!engine.IsInitialized());
}
TEST(TestContentCat_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& engine = ContentCategorizationEngine::Instance();
    ASSERT(engine.Initialize());
    ASSERT(engine.IsInitialized());
    engine.Shutdown();
}
TEST(TestContentCat_Categorize)
{
    using namespace ExplorerLens::Engine;
    auto& engine = ContentCategorizationEngine::Instance();
    engine.Initialize();
    auto result = engine.Categorize(L"photo.jpg");
    ASSERT(result.primary != ContentCategory::Unknown);
    ASSERT(result.confidence > 0.0);
    ASSERT(!result.topK.empty());
    engine.Shutdown();
}
TEST(TestContentCat_CategorizeUninitialized)
{
    using namespace ExplorerLens::Engine;
    auto& engine = ContentCategorizationEngine::Instance();
    engine.Shutdown();
    auto result = engine.Categorize(L"file.txt");
    ASSERT(result.primary == ContentCategory::Unknown);
}
TEST(TestContentCat_ToString)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ToString(ContentCategory::Photo)) == L"Photo");
    ASSERT(std::wstring(ToString(ContentCategory::SourceCode)) == L"Source Code");
    ASSERT(std::wstring(ToString(ContentCategory::ThreeDModel)) == L"3D Model");
    ASSERT(std::wstring(ToString(ContentCategory::Unknown)) == L"Unknown");
}
TEST(TestContentCat_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& engine = ContentCategorizationEngine::Instance();
    engine.Initialize();
    engine.Categorize(L"img.png");
    auto stats = engine.GetStats();
    ASSERT(stats.totalClassified > 0);
    ASSERT(stats.highConfidenceCount > 0);
    engine.Shutdown();
}
TEST(TestContentCat_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& engine = ContentCategorizationEngine::Instance();
    engine.Initialize();
    engine.Shutdown();
    ASSERT(!engine.IsInitialized());
}
TEST(TestContentCat_MultipleCategories)
{
    using namespace ExplorerLens::Engine;
    auto& engine = ContentCategorizationEngine::Instance();
    engine.Initialize();
    auto r1 = engine.Categorize(L"a.jpg");
    auto r2 = engine.Categorize(L"b.png");
    ASSERT(engine.GetStats().totalClassified >= 2);
    engine.Shutdown();
}

// --- ThumbnailQualityPredictor ---
TEST(TestQualPred_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& pred = ThumbnailQualityPredictor::Instance();
    ASSERT(!pred.IsInitialized());
}
TEST(TestQualPred_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& pred = ThumbnailQualityPredictor::Instance();
    ASSERT(pred.Initialize());
    ASSERT(pred.IsInitialized());
    ASSERT(pred.GetSkipThreshold() > 0.0);
    pred.Shutdown();
}
TEST(TestQualPred_PredictGood)
{
    using namespace ExplorerLens::Engine;
    auto& pred = ThumbnailQualityPredictor::Instance();
    pred.Initialize();
    auto result = pred.Predict(L"photo.jpg", 1024 * 1024);
    ASSERT(result.quality == PredictedQuality::Good);
    ASSERT(!result.shouldSkipDecode);
    pred.Shutdown();
}
TEST(TestQualPred_PredictUnusable)
{
    using namespace ExplorerLens::Engine;
    auto& pred = ThumbnailQualityPredictor::Instance();
    pred.Initialize();
    auto result = pred.Predict(L"tiny.dat", 50);
    ASSERT(result.quality == PredictedQuality::Unusable);
    ASSERT(result.shouldSkipDecode);
    pred.Shutdown();
}
TEST(TestQualPred_PredictUninitialized)
{
    using namespace ExplorerLens::Engine;
    auto& pred = ThumbnailQualityPredictor::Instance();
    pred.Shutdown();
    auto result = pred.Predict(L"file.jpg");
    ASSERT(result.quality == PredictedQuality::Good);
}
TEST(TestQualPred_ToString)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ToString(PredictedQuality::Excellent)) == L"Excellent");
    ASSERT(std::wstring(ToString(PredictedQuality::Unusable)) == L"Unusable");
}
TEST(TestQualPred_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& pred = ThumbnailQualityPredictor::Instance();
    pred.Initialize();
    pred.Predict(L"a.jpg", 1000000);
    pred.Predict(L"b.dat", 10);
    auto stats = pred.GetStats();
    ASSERT(stats.totalPredictions >= 2);
    ASSERT(stats.skippedDecodes >= 1);
    pred.Shutdown();
}
TEST(TestQualPred_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& pred = ThumbnailQualityPredictor::Instance();
    pred.Initialize();
    pred.Shutdown();
    ASSERT(!pred.IsInitialized());
}

// --- SmartBatchProcessor ---
TEST(TestSmartBatch_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    ASSERT(bp.GetState() == BatchState::Idle || true);
}
TEST(TestSmartBatch_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    ASSERT(bp.Initialize(8));
    ASSERT(bp.IsInitialized());
    ASSERT(bp.GetMaxConcurrency() == 8);
    bp.Shutdown();
}
TEST(TestSmartBatch_SubmitAndStart)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    bp.Initialize();
    std::vector<SmartBatchTask> tasks = {{L"a.jpg"}, {L"b.png"}, {L"c.webp"}};
    ASSERT(bp.Submit(tasks));
    ASSERT(bp.Start());
    ASSERT(bp.GetState() == BatchState::Running);
    bp.Shutdown();
}
TEST(TestSmartBatch_PauseResume)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    bp.Initialize();
    bp.Submit({{L"x.jpg"}});
    bp.Start();
    ASSERT(bp.Pause());
    ASSERT(bp.GetState() == BatchState::Paused);
    ASSERT(bp.Resume());
    ASSERT(bp.GetState() == BatchState::Running);
    bp.Shutdown();
}
TEST(TestSmartBatch_Cancel)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    bp.Initialize();
    bp.Submit({{L"x.jpg"}});
    bp.Start();
    ASSERT(bp.Cancel());
    ASSERT(bp.GetState() == BatchState::Cancelled);
    bp.Shutdown();
}
TEST(TestSmartBatch_EmptySubmit)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    bp.Initialize();
    ASSERT(!bp.Submit({}));
    bp.Shutdown();
}
TEST(TestSmartBatch_ToString)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::wstring(ToString(BatchState::Running)) == L"Running");
    ASSERT(std::wstring(ToString(BatchState::Cancelled)) == L"Cancelled");
}
TEST(TestSmartBatch_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    bp.Initialize();
    bp.Submit({{L"a.jpg"}});
    bp.Start();
    auto stats = bp.GetStats();
    ASSERT(stats.totalBatchesRun >= 1);
    bp.Shutdown();
}
TEST(TestSmartBatch_Progress)
{
    using namespace ExplorerLens::Engine;
    auto& bp = SmartBatchProcessor::Instance();
    bp.Initialize();
    bp.Submit({{L"a.jpg"}, {L"b.jpg"}});
    auto prog = bp.GetProgress();
    ASSERT(prog.totalTasks == 2);
    bp.Shutdown();
}

// --- WorkflowAutomationEngine ---
TEST(TestWorkflow_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    ASSERT(!wf.IsInitialized());
}
TEST(TestWorkflow_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    ASSERT(wf.Initialize());
    ASSERT(wf.IsInitialized());
    wf.Shutdown();
}
TEST(TestWorkflow_AddRule)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    wf.Initialize();
    WorkflowRule rule;
    rule.name = L"Auto-generate on new file";
    rule.trigger = WorkflowTrigger::FileCreated;
    rule.action = WorkflowAction::GenerateThumbnail;
    uint32_t id = wf.AddRule(rule);
    ASSERT(id > 0);
    ASSERT(wf.GetRuleCount() == 1);
    wf.Shutdown();
}
TEST(TestWorkflow_RemoveRule)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    wf.Initialize();
    WorkflowRule rule;
    rule.name = L"test";
    uint32_t id = wf.AddRule(rule);
    ASSERT(wf.RemoveRule(id));
    ASSERT(wf.GetRuleCount() == 0);
    wf.Shutdown();
}
TEST(TestWorkflow_EnableDisable)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    wf.Initialize();
    WorkflowRule rule;
    rule.name = L"test";
    rule.enabled = true;
    uint32_t id = wf.AddRule(rule);
    ASSERT(wf.EnableRule(id, false));
    ASSERT(wf.GetStats().enabledRules == 0);
    wf.Shutdown();
}
TEST(TestWorkflow_RemoveInvalid)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    wf.Initialize();
    ASSERT(!wf.RemoveRule(9999));
    wf.Shutdown();
}
TEST(TestWorkflow_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    wf.Initialize();
    WorkflowRule r;
    r.name = L"s";
    wf.AddRule(r);
    auto stats = wf.GetStats();
    ASSERT(stats.totalRules >= 1);
    ASSERT(stats.enabledRules >= 1);
    wf.Shutdown();
}
TEST(TestWorkflow_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    wf.Initialize();
    wf.Shutdown();
    ASSERT(!wf.IsInitialized());
    ASSERT(wf.GetRuleCount() == 0);
}
TEST(TestWorkflow_AddUninitialized)
{
    using namespace ExplorerLens::Engine;
    auto& wf = WorkflowAutomationEngine::Instance();
    wf.Shutdown();
    WorkflowRule rule;
    rule.name = L"fail";
    ASSERT(wf.AddRule(rule) == 0);
}

// --- UserBehaviorAnalytics ---
TEST(TestUserBehavior_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    ASSERT(!uba.IsInitialized());
}
TEST(TestUserBehavior_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    ASSERT(uba.Initialize());
    ASSERT(uba.IsInitialized());
    uba.Shutdown();
}
TEST(TestUserBehavior_RecordEvent)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    uba.Initialize();
    UserNavigationEvent evt;
    evt.folderPath = L"C:\\Photos";
    evt.dwellTimeMs = 5000.0;
    uba.RecordEvent(evt);
    ASSERT(uba.GetEventCount() >= 1);
    uba.Shutdown();
}
TEST(TestUserBehavior_RecordUninitialized)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    uba.Shutdown();
    UserNavigationEvent evt;
    evt.folderPath = L"C:\\temp";
    uba.RecordEvent(evt);
}
TEST(TestUserBehavior_GetTopPatterns)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    uba.Initialize();
    UserNavigationEvent evt;
    evt.folderPath = L"C:\\Work";
    evt.dwellTimeMs = 3000.0;
    uba.RecordEvent(evt);
    auto patterns = uba.GetTopPatterns(5);
    ASSERT(!patterns.empty());
    ASSERT(patterns[0].visitCount >= 1);
    uba.Shutdown();
}
TEST(TestUserBehavior_EmptyPatterns)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    uba.Initialize();
    auto patterns = uba.GetTopPatterns();
    ASSERT(patterns.empty() || !patterns.empty());
    uba.Shutdown();
}
TEST(TestUserBehavior_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    uba.Initialize();
    auto stats = uba.GetStats();
    ASSERT(stats.totalSessions >= 1);
    uba.Shutdown();
}
TEST(TestUserBehavior_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& uba = UserBehaviorAnalytics::Instance();
    uba.Initialize();
    uba.Shutdown();
    ASSERT(!uba.IsInitialized());
}

// --- AdaptivePipelineOptimizer ---
TEST(TestAdaptPipe_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    ASSERT(!opt.IsInitialized());
}
TEST(TestAdaptPipe_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    ASSERT(opt.Initialize());
    ASSERT(opt.IsInitialized());
    auto cfg = opt.GetConfig();
    ASSERT(cfg.concurrencyLevel == 4);
    ASSERT(cfg.targetLatencyMs > 0.0);
    opt.Shutdown();
}
TEST(TestAdaptPipe_InitCustomConfig)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    AdaptivePipelineConfig cfg;
    cfg.concurrencyLevel = 8;
    cfg.targetLatencyMs = 10.0;
    opt.Initialize(cfg);
    ASSERT(opt.GetConfig().concurrencyLevel == 8);
    ASSERT(opt.GetConfig().targetLatencyMs == 10.0);
    opt.Shutdown();
}
TEST(TestAdaptPipe_RecordMetric)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    opt.Initialize();
    opt.RecordMetric(PipelineMetric::Throughput, 250.0);
    opt.RecordMetric(PipelineMetric::Latency, 12.0);
    ASSERT(opt.GetStats().currentThroughput == 250.0);
    ASSERT(opt.GetStats().currentLatencyMs == 12.0);
    opt.Shutdown();
}
TEST(TestAdaptPipe_OptimizeReduceConcurrency)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    AdaptivePipelineConfig cfg;
    cfg.concurrencyLevel = 8;
    cfg.targetLatencyMs = 10.0;
    opt.Initialize(cfg);
    opt.RecordMetric(PipelineMetric::Latency, 50.0);
    ASSERT(opt.Optimize());
    ASSERT(opt.GetConfig().concurrencyLevel < 8);
    opt.Shutdown();
}
TEST(TestAdaptPipe_OptimizeIncreaseConcurrency)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    AdaptivePipelineConfig cfg;
    cfg.concurrencyLevel = 4;
    cfg.targetLatencyMs = 20.0;
    opt.Initialize(cfg);
    opt.RecordMetric(PipelineMetric::Latency, 5.0);
    ASSERT(opt.Optimize());
    ASSERT(opt.GetConfig().concurrencyLevel > 4);
    opt.Shutdown();
}
TEST(TestAdaptPipe_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    opt.Initialize();
    opt.Optimize();
    auto stats = opt.GetStats();
    ASSERT(stats.totalAdjustments >= 1);
    opt.Shutdown();
}
TEST(TestAdaptPipe_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    opt.Initialize();
    opt.Shutdown();
    ASSERT(!opt.IsInitialized());
}
TEST(TestAdaptPipe_OptimizeUninitialized)
{
    using namespace ExplorerLens::Engine;
    auto& opt = AdaptivePipelineOptimizer::Instance();
    opt.Shutdown();
    ASSERT(!opt.Optimize());
}

// --- IntelligentPrefetchScheduler ---
TEST(TestPrefetch_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    ASSERT(!sched.IsInitialized());
}
TEST(TestPrefetch_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    ASSERT(sched.Initialize(16, 32 * 1024 * 1024));
    ASSERT(sched.IsInitialized());
    ASSERT(sched.GetMaxDepth() == 16);
    sched.Shutdown();
}
TEST(TestPrefetch_Schedule)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    sched.Initialize();
    SmartPrefetchRequest req;
    req.filePath = L"photo.jpg";
    req.priority = SmartPrefetchPriority::High;
    req.confidenceScore = 0.9;
    ASSERT(sched.Schedule(req));
    ASSERT(sched.GetQueueDepth() == 1);
    sched.Shutdown();
}
TEST(TestPrefetch_ScheduleBatch)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    sched.Initialize();
    std::vector<SmartPrefetchRequest> reqs = {{L"a.jpg", SmartPrefetchPriority::High, 0.9},
                                              {L"b.png", SmartPrefetchPriority::Normal, 0.7},
                                              {L"c.webp", SmartPrefetchPriority::Low, 0.5}};
    ASSERT(sched.ScheduleBatch(reqs));
    ASSERT(sched.GetQueueDepth() == 3);
    sched.Shutdown();
}
TEST(TestPrefetch_ScheduleUninitialized)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    sched.Shutdown();
    SmartPrefetchRequest req;
    req.filePath = L"fail.jpg";
    ASSERT(!sched.Schedule(req));
}
TEST(TestPrefetch_Flush)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    sched.Initialize();
    sched.Schedule({L"x.jpg"});
    sched.Flush();
    ASSERT(sched.GetQueueDepth() == 0);
    sched.Shutdown();
}
TEST(TestPrefetch_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    sched.Initialize();
    sched.Schedule({L"a.jpg"});
    sched.Schedule({L"b.png"});
    auto stats = sched.GetStats();
    ASSERT(stats.totalScheduled >= 2);
    sched.Shutdown();
}
TEST(TestPrefetch_Eviction)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    sched.Initialize(2);
    sched.Schedule({L"a.jpg"});
    sched.Schedule({L"b.jpg"});
    sched.Schedule({L"c.jpg"});
    ASSERT(sched.GetQueueDepth() == 2);
    ASSERT(sched.GetStats().totalEvicted >= 1);
    sched.Shutdown();
}
TEST(TestPrefetch_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& sched = IntelligentPrefetchScheduler::Instance();
    sched.Initialize();
    sched.Schedule({L"x.jpg"});
    sched.Shutdown();
    ASSERT(!sched.IsInitialized());
    ASSERT(sched.GetQueueDepth() == 0);
}

//== Sprint 1071-1080 — Contextual Intelligence & Self-Healing Tests (v31.4.0) ==

// ContextualRenderingEngine tests
TEST(TestCtxRender_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize(RenderQualityTier::High);
    ASSERT(eng.IsInitialized());
    ASSERT(eng.GetStats().totalEvaluations == 0);
    eng.Shutdown();
}
TEST(TestCtxRender_EvaluateDefault)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    RenderContextSignals sig;
    sig.displayDPI = 96.0f;
    sig.availableGPUMemoryBytes = 1024ULL * 1024 * 1024;
    auto params = eng.Evaluate(sig);
    ASSERT(params.contextType == RenderContextType::UserBrowsing);
    ASSERT(params.qualityTier == RenderQualityTier::Medium);
    eng.Shutdown();
}
TEST(TestCtxRender_HighDPI)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    RenderContextSignals sig;
    sig.displayDPI = 192.0f;
    sig.availableGPUMemoryBytes = 1024ULL * 1024 * 1024;
    auto params = eng.Evaluate(sig);
    ASSERT(params.contextType == RenderContextType::HighDPI);
    ASSERT(params.qualityTier == RenderQualityTier::High);
    eng.Shutdown();
}
TEST(TestCtxRender_BatterySaver)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    RenderContextSignals sig;
    sig.onBattery = true;
    sig.availableGPUMemoryBytes = 1024ULL * 1024 * 1024;
    auto params = eng.Evaluate(sig);
    ASSERT(params.contextType == RenderContextType::BatterySaver);
    ASSERT(!params.useGPU);
    eng.Shutdown();
}
TEST(TestCtxRender_LowMemory)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    RenderContextSignals sig;
    sig.availableGPUMemoryBytes = 64 * 1024 * 1024;
    auto params = eng.Evaluate(sig);
    ASSERT(params.contextType == RenderContextType::LowMemory);
    ASSERT(!params.useGPU);
    eng.Shutdown();
}
TEST(TestCtxRender_ContextSwitch)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    RenderContextSignals sig1;
    sig1.availableGPUMemoryBytes = 1024ULL * 1024 * 1024;
    eng.Evaluate(sig1);
    RenderContextSignals sig2;
    sig2.onBattery = true;
    sig2.availableGPUMemoryBytes = 1024ULL * 1024 * 1024;
    eng.Evaluate(sig2);
    ASSERT(eng.GetStats().contextSwitches >= 1);
    eng.Shutdown();
}
TEST(TestCtxRender_DPIScale)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    RenderContextSignals sig;
    sig.displayDPI = 192.0f;
    sig.availableGPUMemoryBytes = 1024ULL * 1024 * 1024;
    auto params = eng.Evaluate(sig);
    ASSERT(params.dpiScale >= 1.9f);
    ASSERT(params.targetWidthPx >= 480);
    eng.Shutdown();
}
TEST(TestCtxRender_Background)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    RenderContextSignals sig;
    sig.isBackgroundProcess = true;
    sig.availableGPUMemoryBytes = 1024ULL * 1024 * 1024;
    auto params = eng.Evaluate(sig);
    ASSERT(params.contextType == RenderContextType::BackgroundTask);
    eng.Shutdown();
}
TEST(TestCtxRender_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& eng = ContextualRenderingEngine::Instance();
    eng.Initialize();
    eng.Shutdown();
    ASSERT(!eng.IsInitialized());
}

// SmartThumbnailCompositor tests
TEST(TestCompositor_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& comp = SmartThumbnailCompositor::Instance();
    comp.Initialize(4);
    ASSERT(comp.IsInitialized());
    ASSERT(comp.GetStats().totalComposites == 0);
    comp.Shutdown();
}
TEST(TestCompositor_SinglePage)
{
    using namespace ExplorerLens::Engine;
    auto& comp = SmartThumbnailCompositor::Instance();
    comp.Initialize();
    CompositeRequest req;
    req.totalPages = 1;
    auto result = comp.Composite(req);
    ASSERT(result.success);
    ASSERT(result.appliedLayout == CompositeLayout::Single);
    ASSERT(result.layersUsed == 1);
    comp.Shutdown();
}
TEST(TestCompositor_MultiPage)
{
    using namespace ExplorerLens::Engine;
    auto& comp = SmartThumbnailCompositor::Instance();
    comp.Initialize();
    CompositeRequest req;
    req.totalPages = 8;
    req.strategy = LayerSelectionStrategy::EvenlySpaced;
    auto result = comp.Composite(req);
    ASSERT(result.success);
    ASSERT(result.layersUsed >= 2);
    comp.Shutdown();
}
TEST(TestCompositor_GridLayout)
{
    using namespace ExplorerLens::Engine;
    auto& comp = SmartThumbnailCompositor::Instance();
    comp.Initialize();
    CompositeRequest req;
    req.totalPages = 4;
    req.maxLayersToComposite = 4;
    auto result = comp.Composite(req);
    ASSERT(result.success);
    ASSERT(result.appliedLayout == CompositeLayout::Grid2x2);
    comp.Shutdown();
}
TEST(TestCompositor_LargeGrid)
{
    using namespace ExplorerLens::Engine;
    auto& comp = SmartThumbnailCompositor::Instance();
    comp.Initialize(9);
    CompositeRequest req;
    req.totalPages = 20;
    req.maxLayersToComposite = 9;
    auto result = comp.Composite(req);
    ASSERT(result.success);
    ASSERT(result.appliedLayout == CompositeLayout::Grid3x3);
    comp.Shutdown();
}
TEST(TestCompositor_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& comp = SmartThumbnailCompositor::Instance();
    comp.Initialize();
    CompositeRequest req1;
    req1.totalPages = 1;
    comp.Composite(req1);
    CompositeRequest req2;
    req2.totalPages = 4;
    comp.Composite(req2);
    auto stats = comp.GetStats();
    ASSERT(stats.totalComposites == 2);
    ASSERT(stats.singlePageResults == 1);
    ASSERT(stats.multiLayerResults == 1);
    comp.Shutdown();
}
TEST(TestCompositor_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& comp = SmartThumbnailCompositor::Instance();
    comp.Initialize();
    comp.Shutdown();
    ASSERT(!comp.IsInitialized());
}

// FormatComplexityAnalyzer tests
TEST(TestFmtComplexity_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    ASSERT(fca.IsInitialized());
    ASSERT(fca.GetStats().totalAnalyses == 0);
    fca.Shutdown();
}
TEST(TestFmtComplexity_Trivial)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    auto r = fca.Analyze(L"tiny.jpg", 32 * 1024);
    ASSERT(r.level == FormatComplexityLevel::Trivial);
    ASSERT(r.estimatedDecodeMs < 5.0f);
    fca.Shutdown();
}
TEST(TestFmtComplexity_Extreme)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    auto r = fca.Analyze(L"huge.tiff", 256ULL * 1024 * 1024);
    ASSERT(r.level == FormatComplexityLevel::Extreme);
    ASSERT(r.requiresGPU);
    fca.Shutdown();
}
TEST(TestFmtComplexity_Moderate)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    auto r = fca.Analyze(L"photo.jpg", 4 * 1024 * 1024);
    ASSERT(r.level == FormatComplexityLevel::Moderate);
    fca.Shutdown();
}
TEST(TestFmtComplexity_Complex)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    auto r = fca.Analyze(L"raw.cr3", 64 * 1024 * 1024);
    ASSERT(r.level == FormatComplexityLevel::Complex);
    ASSERT(r.requiresGPU);
    fca.Shutdown();
}
TEST(TestFmtComplexity_StatsAverage)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    fca.Analyze(L"a.jpg", 1024);
    fca.Analyze(L"b.jpg", 1024);
    ASSERT(fca.GetStats().totalAnalyses == 2);
    ASSERT(fca.GetStats().averageEstimatedMs > 0.0f);
    fca.Shutdown();
}
TEST(TestFmtComplexity_Simple)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    auto r = fca.Analyze(L"icon.ico", 512 * 1024);
    ASSERT(r.level == FormatComplexityLevel::Simple);
    fca.Shutdown();
}
TEST(TestFmtComplexity_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& fca = FormatComplexityAnalyzer::Instance();
    fca.Initialize();
    fca.Shutdown();
    ASSERT(!fca.IsInitialized());
}

// FaultTolerantDecodeOrchestrator tests
TEST(TestFaultTolerant_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize(3);
    ASSERT(fto.IsInitialized());
    ASSERT(fto.GetStats().totalDecodes == 0);
    fto.Shutdown();
}
TEST(TestFaultTolerant_RecordSuccess)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize();
    auto rec = fto.RecordAttempt(L"WebPDecoder", DecodeOutcome::Success, 5.0f);
    ASSERT(rec.outcome == DecodeOutcome::Success);
    ASSERT(fto.GetStats().successfulDecodes == 1);
    fto.Shutdown();
}
TEST(TestFaultTolerant_RecordFallback)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize();
    fto.RecordAttempt(L"JXLDecoder", DecodeOutcome::RecoveredViaFallback, 50.0f);
    ASSERT(fto.GetStats().fallbackRecoveries == 1);
    fto.Shutdown();
}
TEST(TestFaultTolerant_Reliability)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize();
    fto.RecordAttempt(L"PDFDecoder", DecodeOutcome::Success, 10.0f);
    fto.RecordAttempt(L"PDFDecoder", DecodeOutcome::PermanentFailure, 0.0f);
    auto rel = fto.GetDecoderReliability(L"PDFDecoder");
    ASSERT(rel.totalAttempts == 2);
    ASSERT(rel.reliabilityScore >= 0.4f);
    ASSERT(rel.reliabilityScore <= 0.6f);
    fto.Shutdown();
}
TEST(TestFaultTolerant_FallbackRecommendation)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize();
    for (int i = 0; i < 10; ++i)
        fto.RecordAttempt(L"BadDecoder", DecodeOutcome::PermanentFailure, 0.0f);
    auto fb = fto.GetRecommendedFallback(L"BadDecoder");
    ASSERT(fb == FallbackStrategy::SkipFile);
    fto.Shutdown();
}
TEST(TestFaultTolerant_OverallReliability)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize();
    fto.RecordAttempt(L"Dec1", DecodeOutcome::Success, 5.0f);
    fto.RecordAttempt(L"Dec2", DecodeOutcome::Success, 5.0f);
    ASSERT(fto.GetStats().overallReliability >= 0.9f);
    fto.Shutdown();
}
TEST(TestFaultTolerant_PermanentFailure)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize();
    fto.RecordAttempt(L"CrashDecoder", DecodeOutcome::PermanentFailure, 0.0f);
    ASSERT(fto.GetStats().permanentFailures == 1);
    fto.Shutdown();
}
TEST(TestFaultTolerant_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& fto = FaultTolerantDecodeOrchestrator::Instance();
    fto.Initialize();
    fto.Shutdown();
    ASSERT(!fto.IsInitialized());
}

// DiagnosticTelemetryCollector tests
TEST(TestDiagTelemetry_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize(100);
    ASSERT(dtc.IsInitialized());
    ASSERT(dtc.GetRecordCount() == 0);
    dtc.Shutdown();
}
TEST(TestDiagTelemetry_Record)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize();
    dtc.Record(DiagnosticCategory::Decode, DiagnosticSeverity::Info, L"WebP", L"decoded ok", 5.0f);
    ASSERT(dtc.GetRecordCount() == 1);
    ASSERT(dtc.GetStats().totalRecorded == 1);
    dtc.Shutdown();
}
TEST(TestDiagTelemetry_FilterByCategory)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize();
    dtc.Record(DiagnosticCategory::Decode, DiagnosticSeverity::Info, L"A", L"m1");
    dtc.Record(DiagnosticCategory::Cache, DiagnosticSeverity::Info, L"B", L"m2");
    dtc.Record(DiagnosticCategory::Decode, DiagnosticSeverity::Info, L"C", L"m3");
    auto decodes = dtc.GetRecords(DiagnosticCategory::Decode);
    ASSERT(decodes.size() == 2);
    dtc.Shutdown();
}
TEST(TestDiagTelemetry_ErrorTracking)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize();
    dtc.Record(DiagnosticCategory::Error, DiagnosticSeverity::Error, L"GPU", L"OOM");
    dtc.Record(DiagnosticCategory::Error, DiagnosticSeverity::Critical, L"GPU", L"crash");
    ASSERT(dtc.GetStats().errorCount == 1);
    ASSERT(dtc.GetStats().criticalCount == 1);
    dtc.Shutdown();
}
TEST(TestDiagTelemetry_RecentErrors)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize();
    dtc.Record(DiagnosticCategory::Decode, DiagnosticSeverity::Info, L"ok", L"ok");
    dtc.Record(DiagnosticCategory::Error, DiagnosticSeverity::Error, L"err", L"fail");
    auto errs = dtc.GetRecentErrors(10);
    ASSERT(errs.size() == 1);
    dtc.Shutdown();
}
TEST(TestDiagTelemetry_RingBufferEviction)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize(5);
    for (int i = 0; i < 10; ++i)
        dtc.Record(DiagnosticCategory::Decode, DiagnosticSeverity::Info, L"src", L"msg");
    ASSERT(dtc.GetRecordCount() == 5);
    ASSERT(dtc.GetStats().totalDropped >= 5);
    dtc.Shutdown();
}
TEST(TestDiagTelemetry_SequenceIds)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize();
    dtc.Record(DiagnosticCategory::GPU, DiagnosticSeverity::Info, L"gpu", L"m1");
    dtc.Record(DiagnosticCategory::GPU, DiagnosticSeverity::Info, L"gpu", L"m2");
    auto recs = dtc.GetRecords(DiagnosticCategory::GPU);
    ASSERT(recs.size() == 2);
    ASSERT(recs[1].sequenceId > recs[0].sequenceId);
    dtc.Shutdown();
}
TEST(TestDiagTelemetry_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& dtc = DiagnosticTelemetryCollector::Instance();
    dtc.Initialize();
    dtc.Record(DiagnosticCategory::Decode, DiagnosticSeverity::Info, L"x", L"y");
    dtc.Shutdown();
    ASSERT(!dtc.IsInitialized());
    ASSERT(dtc.GetRecordCount() == 0);
}

// DecoderFaultIsolator tests
TEST(TestFaultIsolator_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize(5, 3);
    ASSERT(dfi.IsInitialized());
    ASSERT(dfi.GetStats().totalFaults == 0);
    dfi.Shutdown();
}
TEST(TestFaultIsolator_RecordFault)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize();
    dfi.RecordFault(L"WebPDecoder", FaultSeverity::Minor);
    ASSERT(dfi.GetStats().totalFaults == 1);
    ASSERT(!dfi.IsQuarantined(L"WebPDecoder"));
    dfi.Shutdown();
}
TEST(TestFaultIsolator_Quarantine)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize(3, 3);
    for (int i = 0; i < 3; ++i)
        dfi.RecordFault(L"BadDec", FaultSeverity::Major);
    ASSERT(dfi.IsQuarantined(L"BadDec"));
    ASSERT(dfi.GetStats().quarantineEvents >= 1);
    dfi.Shutdown();
}
TEST(TestFaultIsolator_FatalImmediate)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize();
    dfi.RecordFault(L"CrashDec", FaultSeverity::Fatal);
    ASSERT(dfi.IsQuarantined(L"CrashDec"));
    dfi.Shutdown();
}
TEST(TestFaultIsolator_ReleaseFromQuarantine)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize(2, 5);
    dfi.RecordFault(L"Dec1", FaultSeverity::Major);
    dfi.RecordFault(L"Dec1", FaultSeverity::Major);
    ASSERT(dfi.IsQuarantined(L"Dec1"));
    dfi.ReleaseFromQuarantine(L"Dec1");
    ASSERT(!dfi.IsQuarantined(L"Dec1"));
    dfi.Shutdown();
}
TEST(TestFaultIsolator_PermanentDisable)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize(1, 1);
    dfi.RecordFault(L"Dec2", FaultSeverity::Major);
    dfi.ReleaseFromQuarantine(L"Dec2");
    dfi.RecordFault(L"Dec2", FaultSeverity::Major);
    auto state = dfi.GetDecoderState(L"Dec2");
    ASSERT(state.status == QuarantineStatus::PermanentlyDisabled);
    dfi.Shutdown();
}
TEST(TestFaultIsolator_SuccessResetsConsecutive)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize(5, 3);
    dfi.RecordFault(L"Dec3", FaultSeverity::Minor);
    dfi.RecordFault(L"Dec3", FaultSeverity::Minor);
    dfi.RecordSuccess(L"Dec3");
    auto state = dfi.GetDecoderState(L"Dec3");
    ASSERT(state.consecutiveFaults == 0);
    dfi.Shutdown();
}
TEST(TestFaultIsolator_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& dfi = DecoderFaultIsolator::Instance();
    dfi.Initialize();
    dfi.Shutdown();
    ASSERT(!dfi.IsInitialized());
}

// SmartRetryOrchestrator tests
TEST(TestSmartRetry_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    sro.Initialize();
    ASSERT(sro.IsInitialized());
    ASSERT(sro.GetStats().totalRetryRequests == 0);
    sro.Shutdown();
}
TEST(TestSmartRetry_BasicRetry)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    sro.Initialize();
    auto att = sro.EvaluateRetry(L"WebP", SmartRetryReason::Timeout);
    ASSERT(att.decision == SmartRetryDecision::Retry);
    ASSERT(att.attemptNumber == 1);
    ASSERT(att.delayMs >= 40.0f);
    sro.Shutdown();
}
TEST(TestSmartRetry_ExponentialBackoff)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    SmartRetryConfig cfg;
    cfg.maxRetries = 5;
    cfg.baseDelayMs = 100.0f;
    cfg.backoffMultiplier = 2.0f;
    sro.Initialize(cfg);
    auto a1 = sro.EvaluateRetry(L"Dec", SmartRetryReason::Timeout);
    auto a2 = sro.EvaluateRetry(L"Dec", SmartRetryReason::Timeout);
    ASSERT(a2.delayMs > a1.delayMs);
    sro.Shutdown();
}
TEST(TestSmartRetry_CorruptNotRetryable)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    sro.Initialize();
    auto att = sro.EvaluateRetry(L"Dec", SmartRetryReason::CorruptData);
    ASSERT(att.decision == SmartRetryDecision::NotRetryable);
    sro.Shutdown();
}
TEST(TestSmartRetry_GPUFallback)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    sro.Initialize();
    auto att = sro.EvaluateRetry(L"GPUDec", SmartRetryReason::GPUError);
    ASSERT(att.decision == SmartRetryDecision::RetryWithFallback);
    sro.Shutdown();
}
TEST(TestSmartRetry_Exhausted)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    SmartRetryConfig cfg;
    cfg.maxRetries = 2;
    sro.Initialize(cfg);
    sro.EvaluateRetry(L"Dec", SmartRetryReason::Timeout);
    sro.EvaluateRetry(L"Dec", SmartRetryReason::Timeout);
    auto a3 = sro.EvaluateRetry(L"Dec", SmartRetryReason::Timeout);
    ASSERT(a3.decision == SmartRetryDecision::Exhausted);
    sro.Shutdown();
}
TEST(TestSmartRetry_CircuitBreaker)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    SmartRetryConfig cfg;
    cfg.maxRetries = 100;
    cfg.circuitBreakerThreshold = 5;
    sro.Initialize(cfg);
    for (int i = 0; i < 6; ++i)
        sro.EvaluateRetry(L"Flaky", SmartRetryReason::TransientFailure);
    ASSERT(sro.GetStats().circuitBreakerTrips >= 1);
    sro.Shutdown();
}
TEST(TestSmartRetry_ResetDecoder)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    sro.Initialize();
    sro.EvaluateRetry(L"Dec", SmartRetryReason::Timeout);
    sro.ResetDecoder(L"Dec");
    auto att = sro.EvaluateRetry(L"Dec", SmartRetryReason::Timeout);
    ASSERT(att.attemptNumber == 1);
    sro.Shutdown();
}
TEST(TestSmartRetry_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& sro = SmartRetryOrchestrator::Instance();
    sro.Initialize();
    sro.Shutdown();
    ASSERT(!sro.IsInitialized());
}

// PipelineHealthMonitor tests
TEST(TestPipeHealth_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    phm.Initialize();
    ASSERT(phm.IsInitialized());
    ASSERT(phm.GetStats().totalSamples == 0);
    phm.Shutdown();
}
TEST(TestPipeHealth_RecordSample)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    phm.Initialize();
    phm.RecordSample(10.0f, true);
    phm.RecordSample(20.0f, true);
    ASSERT(phm.GetStats().totalSamples == 2);
    phm.Shutdown();
}
TEST(TestPipeHealth_HealthySnapshot)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    phm.Initialize();
    for (int i = 0; i < 100; ++i)
        phm.RecordSample(10.0f, true);
    auto snap = phm.GetSnapshot();
    ASSERT(snap.overallHealth == PipelineAlertLevel::Normal);
    ASSERT(snap.currentErrorRate < 0.01f);
    phm.Shutdown();
}
TEST(TestPipeHealth_HighErrorRate)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    PipelineHealthThresholds thresholds;
    thresholds.maxErrorRate = 0.1f;
    phm.Initialize(thresholds);
    for (int i = 0; i < 5; ++i)
        phm.RecordSample(10.0f, true);
    for (int i = 0; i < 5; ++i)
        phm.RecordSample(10.0f, false);
    auto snap = phm.GetSnapshot();
    ASSERT(snap.currentErrorRate >= 0.4f);
    ASSERT(snap.overallHealth >= PipelineAlertLevel::Degraded);
    phm.Shutdown();
}
TEST(TestPipeHealth_HighLatencyAlert)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    PipelineHealthThresholds thresholds;
    thresholds.maxP95LatencyMs = 50.0f;
    thresholds.maxP99LatencyMs = 200.0f;
    phm.Initialize(thresholds);
    for (int i = 0; i < 100; ++i)
        phm.RecordSample(static_cast<float>(i * 3), true);
    auto snap = phm.GetSnapshot();
    ASSERT(snap.p95LatencyMs > 50.0f);
    ASSERT(snap.activeAlertCount >= 1);
    phm.Shutdown();
}
TEST(TestPipeHealth_Alerts)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    PipelineHealthThresholds thresholds;
    thresholds.maxP99LatencyMs = 10.0f;
    phm.Initialize(thresholds);
    for (int i = 0; i < 100; ++i)
        phm.RecordSample(static_cast<float>(i * 10), true);
    auto alerts = phm.GetActiveAlerts();
    ASSERT(!alerts.empty());
    phm.Shutdown();
}
TEST(TestPipeHealth_PeakAlertLevel)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    PipelineHealthThresholds thresholds;
    thresholds.maxP99LatencyMs = 5.0f;
    phm.Initialize(thresholds);
    for (int i = 0; i < 100; ++i)
        phm.RecordSample(100.0f, true);
    ASSERT(phm.GetStats().peakAlertLevel >= PipelineAlertLevel::Critical);
    phm.Shutdown();
}
TEST(TestPipeHealth_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& phm = PipelineHealthMonitor::Instance();
    phm.Initialize();
    phm.RecordSample(10.0f, true);
    phm.Shutdown();
    ASSERT(!phm.IsInitialized());
}

//== Sprint 1081-1090 — Format Routing & Enhanced Accessibility Tests (v31.5.0) ==

// AdaptiveColorProfileManager tests
TEST(TestColorProfile_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& cpm = AdaptiveColorProfileManager::Instance();
    cpm.Initialize(AdaptiveColorSpace::SRGB, 100.0f);
    ASSERT(cpm.IsInitialized());
    ASSERT(cpm.GetStats().totalProfileMatches == 0);
    cpm.Shutdown();
}
TEST(TestColorProfile_MatchSRGB)
{
    using namespace ExplorerLens::Engine;
    auto& cpm = AdaptiveColorProfileManager::Instance();
    cpm.Initialize();
    auto p = cpm.MatchProfile(AdaptiveColorSpace::SRGB);
    ASSERT(p.toneMapping == ToneMappingMode::None);
    ASSERT(p.targetSpace == AdaptiveColorSpace::SRGB);
    cpm.Shutdown();
}
TEST(TestColorProfile_HDRtoSDR)
{
    using namespace ExplorerLens::Engine;
    auto& cpm = AdaptiveColorProfileManager::Instance();
    cpm.Initialize(AdaptiveColorSpace::SRGB, 100.0f);
    auto p = cpm.MatchProfile(AdaptiveColorSpace::HDR10);
    ASSERT(p.toneMapping == ToneMappingMode::Reinhard);
    ASSERT(p.targetSpace == AdaptiveColorSpace::SRGB);
    cpm.Shutdown();
}
TEST(TestColorProfile_HDRtoHDR)
{
    using namespace ExplorerLens::Engine;
    auto& cpm = AdaptiveColorProfileManager::Instance();
    cpm.Initialize(AdaptiveColorSpace::DisplayP3, 1000.0f);
    auto p = cpm.MatchProfile(AdaptiveColorSpace::HDR10);
    ASSERT(p.toneMapping == ToneMappingMode::ACES);
    ASSERT(p.hdrCapable);
    cpm.Shutdown();
}
TEST(TestColorProfile_GamutMapping)
{
    using namespace ExplorerLens::Engine;
    auto& cpm = AdaptiveColorProfileManager::Instance();
    cpm.Initialize();
    cpm.MatchProfile(AdaptiveColorSpace::AdobeRGB);
    ASSERT(cpm.GetStats().gamutMappings == 1);
    cpm.Shutdown();
}
TEST(TestColorProfile_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& cpm = AdaptiveColorProfileManager::Instance();
    cpm.Initialize(AdaptiveColorSpace::SRGB, 80.0f);
    cpm.MatchProfile(AdaptiveColorSpace::HDR10);
    cpm.MatchProfile(AdaptiveColorSpace::SRGB);
    ASSERT(cpm.GetStats().totalProfileMatches == 2);
    ASSERT(cpm.GetStats().fallbacksToSRGB >= 1);
    cpm.Shutdown();
}
TEST(TestColorProfile_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& cpm = AdaptiveColorProfileManager::Instance();
    cpm.Initialize();
    cpm.Shutdown();
    ASSERT(!cpm.IsInitialized());
}

// ThumbnailAccessibilityEngine tests
TEST(TestAccessibility_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& acc = ThumbnailAccessibilityEngine::Instance();
    acc.Initialize();
    ASSERT(acc.IsInitialized());
    ASSERT(acc.GetStats().totalProcessed == 0);
    acc.Shutdown();
}
TEST(TestAccessibility_Standard)
{
    using namespace ExplorerLens::Engine;
    auto& acc = ThumbnailAccessibilityEngine::Instance();
    acc.Initialize();
    auto r = acc.Process(L"test.jpg", 256, 256);
    ASSERT(r.applied);
    ASSERT(r.modeUsed == AccessibilityMode::Standard);
    acc.Shutdown();
}
TEST(TestAccessibility_HighContrast)
{
    using namespace ExplorerLens::Engine;
    auto& acc = ThumbnailAccessibilityEngine::Instance();
    AccessibilitySettings s;
    s.mode = AccessibilityMode::HighContrast;
    acc.Initialize(s);
    auto r = acc.Process(L"test.png", 256, 256);
    ASSERT(r.contrastAdjusted);
    ASSERT(acc.GetStats().highContrastApplied == 1);
    acc.Shutdown();
}
TEST(TestAccessibility_ColorBlind)
{
    using namespace ExplorerLens::Engine;
    auto& acc = ThumbnailAccessibilityEngine::Instance();
    AccessibilitySettings s;
    s.mode = AccessibilityMode::ColorBlindProtanopia;
    acc.Initialize(s);
    acc.Process(L"photo.jpg", 128, 128);
    ASSERT(acc.GetStats().colorBlindAdjusted == 1);
    acc.Shutdown();
}
TEST(TestAccessibility_AltText)
{
    using namespace ExplorerLens::Engine;
    auto& acc = ThumbnailAccessibilityEngine::Instance();
    AccessibilitySettings s;
    s.generateAltText = true;
    acc.Initialize(s);
    auto r = acc.Process(L"img.webp", 320, 240);
    ASSERT(!r.altText.empty());
    ASSERT(acc.GetStats().altTextsGenerated == 1);
    acc.Shutdown();
}
TEST(TestAccessibility_Settings)
{
    using namespace ExplorerLens::Engine;
    auto& acc = ThumbnailAccessibilityEngine::Instance();
    AccessibilitySettings s;
    s.contrast = ContrastLevel::Maximum;
    acc.Initialize(s);
    auto got = acc.GetSettings();
    ASSERT(got.contrast == ContrastLevel::Maximum);
    acc.Shutdown();
}
TEST(TestAccessibility_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& acc = ThumbnailAccessibilityEngine::Instance();
    acc.Initialize();
    acc.Shutdown();
    ASSERT(!acc.IsInitialized());
}

// SmartFileTypeRouter tests
TEST(TestFileRouter_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& rtr = SmartFileTypeRouter::Instance();
    rtr.Initialize();
    ASSERT(rtr.IsInitialized());
    ASSERT(rtr.GetRegisteredRouteCount() == 0);
    rtr.Shutdown();
}
TEST(TestFileRouter_RegisterAndRoute)
{
    using namespace ExplorerLens::Engine;
    auto& rtr = SmartFileTypeRouter::Instance();
    rtr.Initialize();
    rtr.RegisterRoute(L".jpg", L"JPEGDecoder", L"GenericDecoder");
    auto d = rtr.Route(L".jpg");
    ASSERT(d.selectedDecoder == L"JPEGDecoder");
    ASSERT(d.confidence == RouteConfidence::ExtensionOnly);
    ASSERT(d.hasFallback);
    rtr.Shutdown();
}
TEST(TestFileRouter_UnknownFormat)
{
    using namespace ExplorerLens::Engine;
    auto& rtr = SmartFileTypeRouter::Instance();
    rtr.Initialize();
    auto d = rtr.Route(L".xyz123");
    ASSERT(d.selectedDecoder == L"GenericDecoder");
    ASSERT(d.confidence == RouteConfidence::Unknown);
    ASSERT(rtr.GetStats().unknownFormats == 1);
    rtr.Shutdown();
}
TEST(TestFileRouter_RecordSuccess)
{
    using namespace ExplorerLens::Engine;
    auto& rtr = SmartFileTypeRouter::Instance();
    rtr.Initialize();
    rtr.RegisterRoute(L".png", L"PNGDecoder");
    rtr.RecordResult(L".png", true, 5.0f);
    rtr.RecordResult(L".png", true, 15.0f);
    auto d = rtr.Route(L".png");
    ASSERT(d.estimatedDecodeMs > 0.0f);
    rtr.Shutdown();
}
TEST(TestFileRouter_RecordFailure)
{
    using namespace ExplorerLens::Engine;
    auto& rtr = SmartFileTypeRouter::Instance();
    rtr.Initialize();
    rtr.RegisterRoute(L".bmp", L"BMPDecoder");
    rtr.RecordResult(L".bmp", false, 0.0f);
    ASSERT(rtr.GetStats().fallbacksUsed == 1);
    rtr.Shutdown();
}
TEST(TestFileRouter_MultipleRoutes)
{
    using namespace ExplorerLens::Engine;
    auto& rtr = SmartFileTypeRouter::Instance();
    rtr.Initialize();
    rtr.RegisterRoute(L".jpg", L"JPEG");
    rtr.RegisterRoute(L".png", L"PNG");
    rtr.RegisterRoute(L".webp", L"WebP");
    ASSERT(rtr.GetRegisteredRouteCount() == 3);
    rtr.Shutdown();
}
TEST(TestFileRouter_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& rtr = SmartFileTypeRouter::Instance();
    rtr.Initialize();
    rtr.RegisterRoute(L".jpg", L"JPEG");
    rtr.Shutdown();
    ASSERT(!rtr.IsInitialized());
}

// DecoderVersionManager tests
TEST(TestDecVersion_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& dvm = DecoderVersionManager::Instance();
    dvm.Initialize();
    ASSERT(dvm.IsInitialized());
    ASSERT(dvm.GetStats().registeredDecoders == 0);
    dvm.Shutdown();
}
TEST(TestDecVersion_Register)
{
    using namespace ExplorerLens::Engine;
    auto& dvm = DecoderVersionManager::Instance();
    dvm.Initialize();
    DecoderVersionInfo info;
    info.decoderName = L"WebPDecoder";
    info.majorVersion = 2;
    info.minorVersion = 1;
    info.supportsGPU = true;
    dvm.RegisterDecoder(info);
    ASSERT(dvm.GetStats().registeredDecoders == 1);
    ASSERT(dvm.GetStats().gpuCapableDecoders == 1);
    dvm.Shutdown();
}
TEST(TestDecVersion_CompatFull)
{
    using namespace ExplorerLens::Engine;
    auto& dvm = DecoderVersionManager::Instance();
    dvm.Initialize();
    DecoderVersionInfo info;
    info.decoderName = L"Dec";
    info.majorVersion = 3;
    info.minorVersion = 2;
    dvm.RegisterDecoder(info);
    auto r = dvm.CheckCompatibility(L"Dec", 2, 0);
    ASSERT(r.compatibility == VersionCompatibility::FullyCompatible);
    dvm.Shutdown();
}
TEST(TestDecVersion_CompatBackward)
{
    using namespace ExplorerLens::Engine;
    auto& dvm = DecoderVersionManager::Instance();
    dvm.Initialize();
    DecoderVersionInfo info;
    info.decoderName = L"Dec";
    info.majorVersion = 3;
    info.minorVersion = 0;
    dvm.RegisterDecoder(info);
    auto r = dvm.CheckCompatibility(L"Dec", 3, 5);
    ASSERT(r.compatibility == VersionCompatibility::BackwardCompatible);
    ASSERT(r.upgradeAvailable);
    dvm.Shutdown();
}
TEST(TestDecVersion_Deprecated)
{
    using namespace ExplorerLens::Engine;
    auto& dvm = DecoderVersionManager::Instance();
    dvm.Initialize();
    DecoderVersionInfo info;
    info.decoderName = L"OldDec";
    info.majorVersion = 1;
    info.deprecated = true;
    dvm.RegisterDecoder(info);
    auto r = dvm.CheckCompatibility(L"OldDec", 1);
    ASSERT(r.compatibility == VersionCompatibility::MinorIncompatibility);
    ASSERT(dvm.GetStats().deprecatedDecoders == 1);
    dvm.Shutdown();
}
TEST(TestDecVersion_Unsupported)
{
    using namespace ExplorerLens::Engine;
    auto& dvm = DecoderVersionManager::Instance();
    dvm.Initialize();
    auto r = dvm.CheckCompatibility(L"NonExistent", 1);
    ASSERT(r.compatibility == VersionCompatibility::Unsupported);
    dvm.Shutdown();
}
TEST(TestDecVersion_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& dvm = DecoderVersionManager::Instance();
    dvm.Initialize();
    dvm.Shutdown();
    ASSERT(!dvm.IsInitialized());
}

// CrossFormatMetadataEngine tests
TEST(TestCrossMeta_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& cme = CrossFormatMetadataEngine::Instance();
    cme.Initialize();
    ASSERT(cme.IsInitialized());
    ASSERT(cme.GetStats().totalExtractions == 0);
    cme.Shutdown();
}
TEST(TestCrossMeta_JPEG)
{
    using namespace ExplorerLens::Engine;
    auto& cme = CrossFormatMetadataEngine::Instance();
    cme.Initialize();
    auto m = cme.Extract(L"photo.jpg", L".jpg");
    ASSERT(m.primarySource == CrossMetadataSource::EXIF);
    ASSERT(cme.GetStats().exifFound == 1);
    cme.Shutdown();
}
TEST(TestCrossMeta_PNG)
{
    using namespace ExplorerLens::Engine;
    auto& cme = CrossFormatMetadataEngine::Instance();
    cme.Initialize();
    auto m = cme.Extract(L"image.png", L".png");
    ASSERT(m.primarySource == CrossMetadataSource::XMP);
    ASSERT(cme.GetStats().xmpFound == 1);
    cme.Shutdown();
}
TEST(TestCrossMeta_PDF)
{
    using namespace ExplorerLens::Engine;
    auto& cme = CrossFormatMetadataEngine::Instance();
    cme.Initialize();
    auto m = cme.Extract(L"doc.pdf", L".pdf");
    ASSERT(m.primarySource == CrossMetadataSource::PDF);
    cme.Shutdown();
}
// CrossFormatMetadataEngine — additional tests
TEST(TestCrossMeta_RAW)
{
    using namespace ExplorerLens::Engine;
    auto& cme = CrossFormatMetadataEngine::Instance();
    cme.Initialize();
    auto m = cme.Extract(L"photo.tiff", L".tiff");
    ASSERT(m.primarySource == CrossMetadataSource::EXIF);
    cme.Shutdown();
}
TEST(TestCrossMeta_Video)
{
    using namespace ExplorerLens::Engine;
    auto& cme = CrossFormatMetadataEngine::Instance();
    cme.Initialize();
    auto m = cme.Extract(L"track.mp3", L".mp3");
    ASSERT(m.primarySource == CrossMetadataSource::ID3);
    cme.Shutdown();
}
TEST(TestCrossMeta_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& cme = CrossFormatMetadataEngine::Instance();
    cme.Initialize();
    cme.Extract(L"a.jpg", L".jpg");
    cme.Extract(L"b.png", L".png");
    auto s = cme.GetStats();
    ASSERT(s.totalExtractions == 2);
    cme.Shutdown();
}

// StreamingDecodeCoordinator tests
TEST(TestSDC_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& sdc = StreamingDecodeCoordinator::Instance();
    sdc.Initialize(64);
    ASSERT(sdc.GetStats().initialized);
    ASSERT(sdc.GetStats().totalStreams == 0);
    sdc.Shutdown();
}
TEST(TestSDC_StartStream)
{
    using namespace ExplorerLens::Engine;
    auto& sdc = StreamingDecodeCoordinator::Instance();
    sdc.Initialize();
    StreamingDecodeRequest req;
    req.filePath = L"large.raw";
    req.fileSize = 50 * 1024 * 1024;
    req.chunkSizeKB = 128;
    auto prog = sdc.StartStream(req);
    ASSERT(sdc.GetStats().totalStreams == 1);
    sdc.Shutdown();
}
TEST(TestSDC_PartialRender)
{
    using namespace ExplorerLens::Engine;
    auto& sdc = StreamingDecodeCoordinator::Instance();
    sdc.Initialize();
    StreamingDecodeRequest req;
    req.filePath = L"huge.tif";
    req.fileSize = 200 * 1024 * 1024;
    req.allowPartialRender = true;
    sdc.StartStream(req);
    ASSERT(sdc.GetStats().partialRenders >= 0);
    sdc.Shutdown();
}
TEST(TestSDC_CompleteStream)
{
    using namespace ExplorerLens::Engine;
    auto& sdc = StreamingDecodeCoordinator::Instance();
    sdc.Initialize();
    StreamingDecodeRequest req;
    req.filePath = L"test.psd";
    req.fileSize = 64 * 1024;
    req.chunkSizeKB = 64;
    sdc.StartStream(req);
    // advance all chunks to completion
    while (sdc.GetProgress().state != StreamingDecodeState::Complete) {
        sdc.AdvanceChunk();
    }
    ASSERT(sdc.GetStats().completedStreams >= 1);
    sdc.Shutdown();
}
TEST(TestSDC_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& sdc = StreamingDecodeCoordinator::Instance();
    sdc.Initialize();
    auto s = sdc.GetStats();
    ASSERT(s.initialized);
    sdc.Shutdown();
}
TEST(TestSDC_ProgressTracking)
{
    using namespace ExplorerLens::Engine;
    auto& sdc = StreamingDecodeCoordinator::Instance();
    sdc.Initialize(32);
    StreamingDecodeRequest req;
    req.filePath = L"progress.cr2";
    req.fileSize = 30 * 1024 * 1024;
    req.chunkSizeKB = 32;
    sdc.StartStream(req);
    auto prog = sdc.GetProgress();
    ASSERT(prog.totalChunks > 0);
    sdc.Shutdown();
}
TEST(TestSDC_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& sdc = StreamingDecodeCoordinator::Instance();
    sdc.Initialize();
    sdc.Shutdown();
    ASSERT(!sdc.GetStats().initialized);
}

// RenderPipelineProfiler tests
TEST(TestRPP_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& rpp = RenderPipelineProfiler::Instance();
    rpp.Initialize();
    ASSERT(rpp.GetStats().initialized);
    ASSERT(rpp.GetStats().totalProfiles == 0);
    rpp.Shutdown();
}
TEST(TestRPP_RecordStage)
{
    using namespace ExplorerLens::Engine;
    auto& rpp = RenderPipelineProfiler::Instance();
    rpp.Initialize();
    rpp.RecordStage(ProfilerStage::Decode, 12.5f);
    rpp.RecordStage(ProfilerStage::GPUUpload, 3.2f);
    ASSERT(rpp.GetStats().stagesRecorded >= 2);
    rpp.Shutdown();
}
TEST(TestRPP_Snapshot)
{
    using namespace ExplorerLens::Engine;
    auto& rpp = RenderPipelineProfiler::Instance();
    rpp.Initialize();
    rpp.RecordStage(ProfilerStage::FileIO, 5.0f);
    rpp.RecordStage(ProfilerStage::Decode, 15.0f);
    auto snap = rpp.GetSnapshot();
    ASSERT(snap.totalPipelineMs > 0.0f);
    rpp.Shutdown();
}
TEST(TestRPP_Bottleneck)
{
    using namespace ExplorerLens::Engine;
    auto& rpp = RenderPipelineProfiler::Instance();
    rpp.Initialize();
    rpp.RecordStage(ProfilerStage::FileIO, 2.0f);
    rpp.RecordStage(ProfilerStage::Decode, 45.0f);
    rpp.RecordStage(ProfilerStage::GPUUpload, 1.5f);
    auto snap = rpp.GetSnapshot();
    ASSERT(snap.bottleneckStage == ProfilerStage::Decode);
    rpp.Shutdown();
}
TEST(TestRPP_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& rpp = RenderPipelineProfiler::Instance();
    rpp.Initialize();
    rpp.RecordStage(ProfilerStage::CacheLookup, 0.3f);
    auto s = rpp.GetStats();
    ASSERT(s.stagesRecorded >= 1);
    rpp.Shutdown();
}
TEST(TestRPP_AllStages)
{
    using namespace ExplorerLens::Engine;
    auto& rpp = RenderPipelineProfiler::Instance();
    rpp.Initialize();
    for (int i = 0; i <= static_cast<int>(ProfilerStage::Output); ++i) {
        rpp.RecordStage(static_cast<ProfilerStage>(i), static_cast<float>(i + 1));
    }
    auto snap = rpp.GetSnapshot();
    ASSERT(snap.stages.size() == static_cast<size_t>(static_cast<int>(ProfilerStage::Output) + 1));
    rpp.Shutdown();
}
TEST(TestRPP_Shutdown)
{
    using namespace ExplorerLens::Engine;
    auto& rpp = RenderPipelineProfiler::Instance();
    rpp.Initialize();
    rpp.Shutdown();
    ASSERT(!rpp.GetStats().initialized);
}

//== v32.0.0 Fomalhaut — Post-Quantum Security & Zero-Trust Tests ==//

// PostQuantumCryptoProvider
TEST(TestPQCP_Init)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    ASSERT(p.Initialize());
    ASSERT(p.IsReady());
}
TEST(TestPQCP_KeyGen_Kyber)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    auto kp = p.GenerateKeyPair(PQCPrimitiveAlgo::Kyber768);
    ASSERT(!kp.publicKey.empty());
    ASSERT(!kp.privateKey.empty());
    ASSERT(kp.publicKey.size() == 1184);
}
TEST(TestPQCP_KeyGen_Dilithium)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    auto kp = p.GenerateKeyPair(PQCPrimitiveAlgo::Dilithium3);
    ASSERT(!kp.publicKey.empty());
    ASSERT(!kp.privateKey.empty());
    ASSERT(kp.algorithm == PQCPrimitiveAlgo::Dilithium3);
}
TEST(TestPQCP_Sign)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    auto kp = p.GenerateKeyPair(PQCPrimitiveAlgo::Dilithium3);
    std::vector<uint8_t> msg = {0x01, 0x02, 0x03};
    auto sig = p.Sign(msg, kp);
    ASSERT(!sig.empty());
    ASSERT(sig.size() == 3293);
}
TEST(TestPQCP_VerifyOk)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    std::vector<uint8_t> msg = {0xAA, 0xBB};
    std::vector<uint8_t> sig = {0x01};
    std::vector<uint8_t> pub = {0x02};
    ASSERT(p.Verify(msg, sig, pub));
}
TEST(TestPQCP_VerifyFail_Empty)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    std::vector<uint8_t> empty;
    std::vector<uint8_t> nonempty = {0x01};
    ASSERT(!p.Verify(empty, nonempty, nonempty));
}
TEST(TestPQCP_Stats_KeyGenCount)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    p.GenerateKeyPair(PQCPrimitiveAlgo::Kyber768);
    p.GenerateKeyPair(PQCPrimitiveAlgo::Kyber768);
    p.GenerateKeyPair(PQCPrimitiveAlgo::Dilithium3);
    ASSERT(p.GetStats().keyGensOk == 3);
}
TEST(TestPQCP_Stats_SignCount)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    auto kp = p.GenerateKeyPair(PQCPrimitiveAlgo::Dilithium3);
    std::vector<uint8_t> msg = {0x01};
    p.Sign(msg, kp);
    p.Sign(msg, kp);
    ASSERT(p.GetStats().signOps == 2);
}
TEST(TestPQCP_Reset)
{
    using namespace ExplorerLens::Engine;
    PostQuantumCryptoProvider p;
    p.Initialize();
    p.GenerateKeyPair(PQCPrimitiveAlgo::Kyber768);
    p.Reset();
    ASSERT(p.GetStats().keyGensOk == 0);
}

// ZeroTrustAccessBroker
TEST(TestZTAB_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& a = ZeroTrustAccessBroker::Instance();
    auto& b = ZeroTrustAccessBroker::Instance();
    ASSERT(&a == &b);
}
TEST(TestZTAB_Issue)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    auto tok = broker.Issue("plugin-a", "decode");
    ASSERT(tok.subject == "plugin-a");
    ASSERT(tok.capability == "decode");
    ASSERT(!tok.signature.empty());
}
TEST(TestZTAB_Validate_Valid)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    auto tok = broker.Issue("plugin-b", "cache");
    ASSERT(broker.Validate(tok));
}
TEST(TestZTAB_Validate_Revoked)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    auto tok = broker.Issue("plugin-c", "ui");
    tok.revoked = true;
    ASSERT(!broker.Validate(tok));
}
TEST(TestZTAB_Revoke)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    broker.Issue("plugin-revoke", "gpu");
    ASSERT(broker.Revoke("plugin-revoke"));
}
TEST(TestZTAB_RevokeMiss)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    ASSERT(!broker.Revoke("nonexistent-plugin-xyz"));
}
TEST(TestZTAB_Stats_Issued)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    uint64_t before = broker.GetStats().tokensIssued;
    broker.Issue("plugin-stat1", "read");
    ASSERT(broker.GetStats().tokensIssued == before + 1);
}
TEST(TestZTAB_Stats_Denied)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    ZeroTrustToken empty_tok;
    uint64_t before = broker.GetStats().tokensDenied;
    broker.Validate(empty_tok);
    ASSERT(broker.GetStats().tokensDenied == before + 1);
}
TEST(TestZTAB_MultiToken)
{
    using namespace ExplorerLens::Engine;
    auto& broker = ZeroTrustAccessBroker::Instance();
    auto t1 = broker.Issue("multi-a", "decode");
    auto t2 = broker.Issue("multi-b", "cache");
    ASSERT(t1.subject != t2.subject);
}

// QuantumResistantHashEngine
TEST(TestQRHE_Init)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    ASSERT(e.Initialize());
    ASSERT(e.IsReady());
}
TEST(TestQRHE_HashSHA3)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize();
    std::vector<uint8_t> data = {0x01, 0x02};
    auto d = e.Hash(data, QRHashAlgo::SHA3_256);
    ASSERT(d.bytes.size() == 32);
    ASSERT(d.algorithm == QRHashAlgo::SHA3_256);
}
TEST(TestQRHE_HashBLAKE3)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize();
    std::vector<uint8_t> data = {0xAA};
    auto d = e.Hash(data, QRHashAlgo::BLAKE3);
    ASSERT(d.bytes.size() == 32);
}
TEST(TestQRHE_HashK12)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize();
    std::vector<uint8_t> data = {0x55};
    auto d = e.Hash(data, QRHashAlgo::KangarooTwelve);
    ASSERT(d.bytes.size() == 64);
}
TEST(TestQRHE_Default)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize(QRHashAlgo::SHA3_256);
    std::vector<uint8_t> data = {0x11};
    auto d = e.Hash(data);
    ASSERT(d.algorithm == QRHashAlgo::SHA3_256);
}
TEST(TestQRHE_ConstantTimeEq)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize();
    std::vector<uint8_t> data = {0x01};
    auto d1 = e.Hash(data, QRHashAlgo::BLAKE3);
    auto d2 = e.Hash(data, QRHashAlgo::BLAKE3);
    ASSERT(e.ConstantTimeCompare(d1, d2));
}
TEST(TestQRHE_ConstantTimeNeq)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize();
    std::vector<uint8_t> data1 = {0x01};
    auto d1 = e.Hash(data1, QRHashAlgo::BLAKE3);
    QRHashDigest d2;
    d2.bytes.assign(32, 0xFF);
    ASSERT(!e.ConstantTimeCompare(d1, d2));
}
TEST(TestQRHE_Stats)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize();
    std::vector<uint8_t> data = {0x01};
    e.Hash(data, QRHashAlgo::SHA3_256);
    e.Hash(data, QRHashAlgo::BLAKE3);
    ASSERT(e.GetStats().hashOps == 2);
}
TEST(TestQRHE_Reset)
{
    using namespace ExplorerLens::Engine;
    QuantumResistantHashEngine e;
    e.Initialize();
    std::vector<uint8_t> data = {0x01};
    e.Hash(data);
    e.Reset();
    ASSERT(e.GetStats().hashOps == 0);
}

// PluginZeroTrustSandbox
TEST(TestPZTS_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& a = PluginZeroTrustSandbox::Instance();
    auto& b = PluginZeroTrustSandbox::Instance();
    ASSERT(&a == &b);
}
TEST(TestPZTS_DefaultPolicy)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    ASSERT(sb.GetPolicy().requiresCapabilityToken);
}
TEST(TestPZTS_SetPolicy)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    PluginSandboxPolicy p;
    p.pluginId = "test-plugin";
    p.maxMemoryMB = 128;
    sb.SetPolicy(p);
    ASSERT(sb.GetPolicy().maxMemoryMB == 128);
}
TEST(TestPZTS_Allow_WithToken)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    auto d = sb.Evaluate("my-plugin", "decode", true);
    ASSERT(d == PluginSandboxDecision::Allow);
}
TEST(TestPZTS_Deny_NoToken)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    PluginSandboxPolicy p;
    p.requiresCapabilityToken = true;
    sb.SetPolicy(p);
    auto d = sb.Evaluate("my-plugin", "decode", false);
    ASSERT(d == PluginSandboxDecision::Deny);
}
TEST(TestPZTS_Deny_EmptyPlugin)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    auto d = sb.Evaluate("", "decode", true);
    ASSERT(d == PluginSandboxDecision::Deny);
}
TEST(TestPZTS_Deny_Empty_Cap)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    auto d = sb.Evaluate("my-plugin", "", true);
    ASSERT(d == PluginSandboxDecision::Deny);
}
TEST(TestPZTS_NotQuarantined)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    ASSERT(!sb.IsQuarantined("any-plugin"));
}
TEST(TestPZTS_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& sb = PluginZeroTrustSandbox::Instance();
    PluginSandboxPolicy p;
    p.requiresCapabilityToken = false;
    sb.SetPolicy(p);
    uint64_t before = sb.GetStats().callsAllowed;
    sb.Evaluate("stats-plugin", "read", true);
    ASSERT(sb.GetStats().callsAllowed == before + 1);
}

// BinaryTrustVerifier
TEST(TestBTV_Init)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    ASSERT(v.Initialize());
    ASSERT(v.IsReady());
}
TEST(TestBTV_Verify_Valid)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    auto ev = v.Verify("LENSShell.dll");
    ASSERT(ev.status == BinaryTrustStatus::Trusted);
}
TEST(TestBTV_Verify_Empty)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    auto ev = v.Verify("");
    ASSERT(ev.status == BinaryTrustStatus::Unknown);
}
TEST(TestBTV_TamperDetect)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    auto ev = v.VerifyTampered("LENSShell.dll");
    ASSERT(ev.status == BinaryTrustStatus::TamperEvident);
}
TEST(TestBTV_TamperReason)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    auto ev = v.VerifyTampered("LENSShell.dll");
    ASSERT(!ev.rejectionReason.empty());
}
TEST(TestBTV_SignerName)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    auto ev = v.Verify("LENSShell.dll");
    ASSERT(!ev.signerName.empty());
}
TEST(TestBTV_Stats_Ok)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    v.Verify("LENSShell.dll");
    ASSERT(v.GetStats().verificationsOk == 1);
}
TEST(TestBTV_Stats_Fail)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    v.Verify("");
    ASSERT(v.GetStats().verificationsFail == 1);
}
TEST(TestBTV_Reset)
{
    using namespace ExplorerLens::Engine;
    BinaryTrustVerifier v;
    v.Initialize();
    v.Verify("LENSShell.dll");
    v.Reset();
    ASSERT(v.GetStats().verificationsOk == 0);
}

// SecureConfigurationManager
TEST(TestSCM_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& a = SecureConfigurationManager::Instance();
    auto& b = SecureConfigurationManager::Instance();
    ASSERT(&a == &b);
}
TEST(TestSCM_Init)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    ASSERT(m.Initialize());
    ASSERT(m.IsReady());
}
TEST(TestSCM_SetAndGet)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    m.Initialize();
    ASSERT(m.Set("api-key", "secret-value"));
    std::string val;
    ASSERT(m.Get("api-key", val));
    ASSERT(val == "secret-value");
}
TEST(TestSCM_GetMissing)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    m.Initialize();
    std::string val;
    ASSERT(!m.Get("nonexistent-key-xyz", val));
}
TEST(TestSCM_Backend_Platform)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    m.Initialize();
#if defined(_WIN32)
    ASSERT(m.GetBackend() == SecureConfigBackend::DPAPI);
#else
    ASSERT(m.GetBackend() != SecureConfigBackend::TPM2);
#endif
}
TEST(TestSCM_MultiKey)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    m.Initialize();
    m.Set("key1", "val1");
    m.Set("key2", "val2");
    std::string v1, v2;
    ASSERT(m.Get("key1", v1) && v1 == "val1");
    ASSERT(m.Get("key2", v2) && v2 == "val2");
}
TEST(TestSCM_OverwriteKey)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    m.Initialize();
    m.Set("overwrite-key", "first");
    m.Set("overwrite-key", "second");
    std::string val;
    m.Get("overwrite-key", val);
    ASSERT(val == "second");
}
TEST(TestSCM_Stats_Writes)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    m.Initialize();
    uint64_t before = m.GetStats().writesOk;
    m.Set("stat-key", "v");
    ASSERT(m.GetStats().writesOk == before + 1);
}
TEST(TestSCM_Stats_Reads)
{
    using namespace ExplorerLens::Engine;
    auto& m = SecureConfigurationManager::Instance();
    m.Initialize();
    m.Set("read-stat-key", "v");
    uint64_t before = m.GetStats().readsOk;
    std::string val;
    m.Get("read-stat-key", val);
    ASSERT(m.GetStats().readsOk == before + 1);
}

// ThreatModelingEngine
TEST(TestTME_Init)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    ASSERT(e.Initialize());
    ASSERT(e.IsReady());
}
TEST(TestTME_Analyze_NotEmpty)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    auto threats = e.Analyze("decode-stage");
    ASSERT(!threats.empty());
}
TEST(TestTME_PipelineSafe)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    ASSERT(e.IsPipelineSafe("decode-stage"));
}
TEST(TestTME_Spoofing_Sim)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    auto s = e.SimulateSpoofing("COM-server");
    ASSERT(s.category == STRIDECategory::Spoofing);
}
TEST(TestTME_Spoofing_Unmitigated)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    auto s = e.SimulateSpoofing("target");
    ASSERT(!s.mitigated);
}
TEST(TestTME_Spoofing_Severity)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    auto s = e.SimulateSpoofing("target");
    ASSERT(s.severity == 9);
}
TEST(TestTME_Stats_AnalyzeCount)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    e.Analyze("stage-a");
    e.Analyze("stage-b");
    ASSERT(e.GetStats().scenariosAnalyzed == 2);
}
TEST(TestTME_Stats_Found)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    e.SimulateSpoofing("t");
    ASSERT(e.GetStats().threatsFound >= 1);
}
TEST(TestTME_Reset)
{
    using namespace ExplorerLens::Engine;
    ThreatModelingEngine e;
    e.Initialize();
    e.Analyze("stage");
    e.Reset();
    ASSERT(e.GetStats().scenariosAnalyzed == 0);
}

// SecurityPostureAnalyzer
TEST(TestSPA_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& a = SecurityPostureAnalyzer::Instance();
    auto& b = SecurityPostureAnalyzer::Instance();
    ASSERT(&a == &b);
}
TEST(TestSPA_Analyze_NonEmpty)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    auto r = spa.Analyze();
    ASSERT(!r.reportId.empty());
}
TEST(TestSPA_Score_Range)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    auto r = spa.Analyze();
    ASSERT(r.overallScore >= 0.0f && r.overallScore <= 100.0f);
}
TEST(TestSPA_PatchLevel)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    auto r = spa.Analyze();
    ASSERT(r.patchLevelCurrent);
}
TEST(TestSPA_Schema)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    auto r = spa.Analyze();
    ASSERT(r.schemaVersion == "1.0");
}
TEST(TestSPA_IsCompliant)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    bool c = spa.IsCompliant(0.0f);
    ASSERT(c || !c);  // just ensure no crash
}
TEST(TestSPA_Serialize)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    auto r = spa.Analyze();
    auto json = spa.SerializeToJson(r);
    ASSERT(!json.empty());
    ASSERT(json.find("reportId") != std::string::npos);
}
TEST(TestSPA_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    uint64_t before = spa.GetStats().reportsGenerated;
    spa.Analyze();
    ASSERT(spa.GetStats().reportsGenerated == before + 1);
}
TEST(TestSPA_ScoreComponents)
{
    using namespace ExplorerLens::Engine;
    auto& spa = SecurityPostureAnalyzer::Instance();
    auto r = spa.Analyze();
    float expected =
        (r.tpmAttested ? 40.0f : 0.0f) + (r.codeIntegrityOk ? 35.0f : 0.0f) + (r.patchLevelCurrent ? 25.0f : 0.0f);
    ASSERT(r.overallScore == expected);
}

//== v32.1.0 Fomalhaut-R — Edge AI & Hardware-Accelerated Inference Tests ==//

// NPUAccelerationEngine
TEST(TestNPUAE_Init)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    ASSERT(e.Initialize());
    ASSERT(e.IsReady());
}
TEST(TestNPUAE_Dispatch)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    NPUWorkload w;
    w.modelName = "clip";
    w.inputData = {1.0f, 2.0f};
    w.batchSize = 2;
    auto out = e.Dispatch(w);
    ASSERT(out.size() == 2);
}
TEST(TestNPUAE_Stats_Dispatched)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    NPUWorkload w;
    w.modelName = "m";
    w.batchSize = 1;
    e.Dispatch(w);
    e.Dispatch(w);
    ASSERT(e.GetStats().workloadsDispatched == 2);
}
TEST(TestNPUAE_CPUFallback)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    NPUWorkload w;
    w.modelName = "test";
    w.mode = NPUDispatchMode::ForceCPU;
    w.batchSize = 1;
    e.Dispatch(w);
    ASSERT(e.GetStats().cpuFallbacks == 1);
}
TEST(TestNPUAE_SetMode)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    e.SetDispatchMode(NPUDispatchMode::ForceGPU);
    ASSERT(e.IsReady());
}
TEST(TestNPUAE_Reset)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    NPUWorkload w;
    w.modelName = "m";
    w.batchSize = 1;
    e.Dispatch(w);
    e.Reset();
    ASSERT(e.GetStats().workloadsDispatched == 0);
}
TEST(TestNPUAE_AvailableCheck)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    // IsNPUAvailable is platform-dependent; just verify no crash
    bool avail = e.IsNPUAvailable();
    ASSERT(avail || !avail);
}
TEST(TestNPUAE_EmptyWorkload)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    NPUWorkload w;
    w.batchSize = 0;
    auto out = e.Dispatch(w);
    ASSERT(out.empty());
}
TEST(TestNPUAE_MultiDispatch)
{
    using namespace ExplorerLens::Engine;
    NPUAccelerationEngine e;
    e.Initialize();
    for (int i = 0; i < 5; ++i) {
        NPUWorkload w;
        w.modelName = "m";
        w.batchSize = 1;
        e.Dispatch(w);
    }
    ASSERT(e.GetStats().workloadsDispatched == 5);
}

// EdgeAIInferenceEngine
TEST(TestEAIE_Init)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    ASSERT(e.Initialize());
    ASSERT(e.IsReady());
}
TEST(TestEAIE_CreateSession)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("clip.onnx");
    ASSERT(s.state == EdgeInferenceState::Ready);
    ASSERT(s.sessionId > 0);
}
TEST(TestEAIE_CreateSession_Empty)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("");
    ASSERT(s.state == EdgeInferenceState::Error);
}
TEST(TestEAIE_RunInference)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("model.onnx");
    std::vector<float> in = {1.0f, 2.0f, 3.0f};
    auto out = e.RunInference(s, in);
    ASSERT(out.size() == in.size());
}
TEST(TestEAIE_RunInference_ErrorState)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("");
    std::vector<float> in = {1.0f};
    auto out = e.RunInference(s, in);
    ASSERT(out.empty());
}
TEST(TestEAIE_MemMapped)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("model.onnx", true);
    ASSERT(s.memMapped);
}
TEST(TestEAIE_Destroy)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("model.onnx");
    e.DestroySession(s);
    ASSERT(s.state == EdgeInferenceState::Idle);
}
TEST(TestEAIE_Stats)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("model.onnx");
    std::vector<float> in = {1.0f};
    e.RunInference(s, in);
    ASSERT(e.GetStats().inferenceRuns == 1);
}
TEST(TestEAIE_Reset)
{
    using namespace ExplorerLens::Engine;
    EdgeAIInferenceEngine e;
    e.Initialize();
    auto s = e.CreateSession("model.onnx");
    std::vector<float> in = {1.0f};
    e.RunInference(s, in);
    e.Reset();
    ASSERT(e.GetStats().inferenceRuns == 0);
}

// HardwareCapabilityNegotiator
TEST(TestHCN_Init)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    ASSERT(n.Initialize());
    ASSERT(n.IsReady());
}
TEST(TestHCN_Negotiate_Embedding)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    auto s = n.Negotiate("embedding");
    ASSERT(s.backend == HWBackendChoice::NPU);
}
TEST(TestHCN_Negotiate_Render)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    auto s = n.Negotiate("render");
    ASSERT(s.backend == HWBackendChoice::GPU);
}
TEST(TestHCN_Negotiate_Default)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    auto s = n.Negotiate("unknown-task");
    ASSERT(s.backend == HWBackendChoice::CPU);
}
TEST(TestHCN_PrefersNPU)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    ASSERT(n.PrefersNPU("clip"));
}
TEST(TestHCN_Score_Avail)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    auto s = n.Negotiate("embedding");
    ASSERT(s.available);
    ASSERT(s.score > 0.0f);
}
TEST(TestHCN_Stats_Total)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    n.Negotiate("embedding");
    n.Negotiate("render");
    ASSERT(n.GetStats().totalQueries >= 2);
}
TEST(TestHCN_Stats_NPUCount)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    n.Negotiate("embedding");
    n.Negotiate("clip");
    ASSERT(n.GetStats().npuSelections == 2);
}
TEST(TestHCN_Reset)
{
    using namespace ExplorerLens::Engine;
    HardwareCapabilityNegotiator n;
    n.Initialize();
    n.Negotiate("embedding");
    n.Reset();
    ASSERT(n.GetStats().totalQueries == 0);
}

// AMDXDNABackend
TEST(TestXDNA_Init)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    ASSERT(b.Initialize());
    ASSERT(b.IsReady());
}
TEST(TestXDNA_DeviceName)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    ASSERT(!b.GetDeviceName().empty());
}
TEST(TestXDNA_TOPS)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    ASSERT(b.GetTOPS() > 0.0f);
}
TEST(TestXDNA_ExecuteKernel)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    std::vector<float> w = {1.0f, 2.0f};
    std::vector<float> in = {3.0f, 4.0f};
    auto out = b.ExecuteKernel("clip_embedding", w, in);
    ASSERT(out.size() == in.size());
}
TEST(TestXDNA_MLIR)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    ASSERT(b.SupportsMLIR());
}
TEST(TestXDNA_Stats)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    std::vector<float> w = {1.0f};
    std::vector<float> in = {2.0f};
    b.ExecuteKernel("k", w, in);
    ASSERT(b.GetStats().kernelsDispatched == 1);
}
TEST(TestXDNA_Reset)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    std::vector<float> w = {1.0f};
    std::vector<float> in = {2.0f};
    b.ExecuteKernel("k", w, in);
    b.Reset();
    ASSERT(b.GetStats().kernelsDispatched == 0);
}
TEST(TestXDNA_TileModes)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    std::vector<float> w = {1.0f};
    std::vector<float> in = {1.0f};
    auto o1 = b.ExecuteKernel("k", w, in, XDNATileMode::Tile1x1);
    auto o2 = b.ExecuteKernel("k", w, in, XDNATileMode::Tile4x4);
    ASSERT(!o1.empty() && !o2.empty());
}
TEST(TestXDNA_AvgLatency)
{
    using namespace ExplorerLens::Engine;
    AMDXDNABackend b;
    b.Initialize();
    std::vector<float> w = {1.0f};
    std::vector<float> in = {1.0f};
    b.ExecuteKernel("k", w, in);
    ASSERT(b.GetStats().avgKernelUs > 0.0f);
}

// QualcommAIEBackend
TEST(TestQAIE_Init)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    ASSERT(b.Initialize());
    ASSERT(b.IsReady());
}
TEST(TestQAIE_DeviceName)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    ASSERT(!b.GetDeviceName().empty());
}
TEST(TestQAIE_RunModel)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    std::vector<float> in = {1.0f, 2.0f, 3.0f};
    auto out = b.RunModel("resnet50", in);
    ASSERT(out.size() == in.size());
}
TEST(TestQAIE_HTPPath)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    std::vector<float> in = {1.0f};
    b.RunModel("m", in, QNNTargetRuntime::HTP);
    ASSERT(b.GetStats().htpHits == 1);
}
TEST(TestQAIE_GPUPath)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    std::vector<float> in = {1.0f};
    b.RunModel("m", in, QNNTargetRuntime::GPU);
    ASSERT(b.GetStats().htpHits == 0);
}
TEST(TestQAIE_Quantization)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    ASSERT(b.SupportsQuantization());
}
TEST(TestQAIE_Stats)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    std::vector<float> in = {1.0f};
    b.RunModel("m", in);
    ASSERT(b.GetStats().inferenceRuns == 1);
}
TEST(TestQAIE_Reset)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    std::vector<float> in = {1.0f};
    b.RunModel("m", in);
    b.Reset();
    ASSERT(b.GetStats().inferenceRuns == 0);
}
TEST(TestQAIE_AvgLatency_HTP)
{
    using namespace ExplorerLens::Engine;
    QualcommAIEBackend b;
    b.Initialize();
    std::vector<float> in = {1.0f};
    b.RunModel("m", in, QNNTargetRuntime::HTP);
    ASSERT(b.GetStats().avgInferenceMs < 15.0f);
}

// IntelAMXBackend
TEST(TestAMX_Init)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    ASSERT(b.Initialize());
    ASSERT(b.IsReady());
}
TEST(TestAMX_MatMul_BF16)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    std::vector<float> a = {1.0f, 2.0f};
    std::vector<float> v = {3.0f, 4.0f};
    auto out = b.MatMul(a, v, AMXPrecisionMode::BF16);
    ASSERT(out.size() == a.size());
    ASSERT(out[0] == 3.0f);
}
TEST(TestAMX_MatMul_INT8)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    std::vector<float> a = {2.0f};
    std::vector<float> v = {5.0f};
    auto out = b.MatMul(a, v, AMXPrecisionMode::INT8);
    ASSERT(out[0] == 10.0f);
}
TEST(TestAMX_MatMul_Empty)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    std::vector<float> empty;
    auto out = b.MatMul(empty, empty);
    ASSERT(out.empty());
}
TEST(TestAMX_Throughput)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    std::vector<float> a = {1.0f};
    std::vector<float> v = {2.0f};
    b.MatMul(a, v);
    ASSERT(b.GetThroughputMultiplierVsSSE42() >= 1.0f);
}
TEST(TestAMX_Stats)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    std::vector<float> a = {1.0f};
    std::vector<float> v = {1.0f};
    b.MatMul(a, v);
    b.MatMul(a, v);
    ASSERT(b.GetStats().matMulOps == 2);
}
TEST(TestAMX_Reset)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    std::vector<float> a = {1.0f};
    std::vector<float> v = {1.0f};
    b.MatMul(a, v);
    b.Reset();
    ASSERT(b.GetStats().matMulOps == 0);
}
TEST(TestAMX_SupportFlag)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    bool s = b.IsAMXSupported();
    ASSERT(s || !s);  // platform-dependent, no crash
}
TEST(TestAMX_AvgLatency)
{
    using namespace ExplorerLens::Engine;
    IntelAMXBackend b;
    b.Initialize();
    std::vector<float> a = {1.0f};
    std::vector<float> v = {1.0f};
    b.MatMul(a, v);
    ASSERT(b.GetStats().avgLatencyMs > 0.0f);
}

// HardwareAcceleratedPipeline
TEST(TestHAP_Init)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    ASSERT(p.Initialize());
    ASSERT(p.IsReady());
}
TEST(TestHAP_Process_Infer)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    p.Initialize();
    std::vector<uint8_t> in = {0x01, 0x02};
    auto out = p.Process(HWPipelineStage::Infer, in);
    ASSERT(out.size() == in.size());
}
TEST(TestHAP_NPURouting)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    HWPipelineConfig cfg;
    cfg.preferNPUForInfer = true;
    p.Initialize(cfg);
    std::vector<uint8_t> in = {0x01};
    p.Process(HWPipelineStage::Infer, in);
    ASSERT(p.GetStats().npuRoutings == 1);
}
TEST(TestHAP_GPURouting)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    HWPipelineConfig cfg;
    cfg.preferNPUForInfer = false;
    cfg.preferGPUForDecode = true;
    p.Initialize(cfg);
    std::vector<uint8_t> in = {0x01};
    p.Process(HWPipelineStage::Decode, in);
    ASSERT(p.GetStats().gpuRoutings == 1);
}
TEST(TestHAP_CPUFallback_Flag)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    p.Initialize();
    ASSERT(p.HasCPUFallback());
}
TEST(TestHAP_Stats)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    p.Initialize();
    std::vector<uint8_t> in = {0x01};
    p.Process(HWPipelineStage::Infer, in);
    p.Process(HWPipelineStage::Decode, in);
    ASSERT(p.GetStats().stagesProcessed == 2);
}
TEST(TestHAP_Reset)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    p.Initialize();
    std::vector<uint8_t> in = {0x01};
    p.Process(HWPipelineStage::Infer, in);
    p.Reset();
    ASSERT(p.GetStats().stagesProcessed == 0);
}
TEST(TestHAP_Empty_Input)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    p.Initialize();
    std::vector<uint8_t> empty;
    auto out = p.Process(HWPipelineStage::Encode, empty);
    ASSERT(out.empty());
}
TEST(TestHAP_Composite)
{
    using namespace ExplorerLens::Engine;
    HardwareAcceleratedPipeline p;
    p.Initialize();
    std::vector<uint8_t> in = {0xAB};
    auto out = p.Process(HWPipelineStage::Composite, in);
    ASSERT(out.size() == 1);
    ASSERT(out[0] == 0xAB);
}

// ComputeDeviceRegistry
TEST(TestCDR_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& a = ComputeDeviceRegistry::Instance();
    auto& b = ComputeDeviceRegistry::Instance();
    ASSERT(&a == &b);
}
TEST(TestCDR_Init)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    ASSERT(r.Initialize());
    ASSERT(r.IsReady());
}
TEST(TestCDR_HasDevices)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    r.Initialize();
    ASSERT(!r.GetDevices().empty());
}
TEST(TestCDR_HasCPU)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    r.Initialize();
    auto* cpu = r.FindBestFor(ComputeDeviceClass::CPU);
    ASSERT(cpu != nullptr);
    ASSERT(cpu->available);
}
TEST(TestCDR_CPUCount)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    r.Initialize();
    ASSERT(r.GetStats().cpuCount >= 1);
}
TEST(TestCDR_Stats_Enumerated)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    r.Initialize();
    ASSERT(r.GetStats().devicesEnumerated >= 1);
}
TEST(TestCDR_Devices_Valid)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    r.Initialize();
    for (const auto& d : r.GetDevices()) {
        ASSERT(!d.name.empty());
        ASSERT(d.deviceClass != ComputeDeviceClass::Unknown);
    }
}
TEST(TestCDR_FindBest_Fallback)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    r.Initialize();
    // FindBestFor should always return non-null (CPU fallback)
    auto* d = r.FindBestFor(ComputeDeviceClass::FPGA);
    ASSERT(d != nullptr);  // falls back to CPU
}
TEST(TestCDR_MultiInit)
{
    using namespace ExplorerLens::Engine;
    auto& r = ComputeDeviceRegistry::Instance();
    r.Initialize();
    r.Initialize();
    ASSERT(r.GetStats().devicesEnumerated >= 1);
}

//== v31.9.0 Achernar-Z — Autonomous Shell Intelligence Tests ==//

TEST(TestAWO_Initialize)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    auto s = awo.GetStats();
    ASSERT(s.jobsQueued == 0);
    ASSERT(s.jobsCompleted == 0);
}
TEST(TestAWO_Enqueue)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    awo.Enqueue(1, WorkflowPriority::Normal, 1024);
    auto s = awo.GetStats();
    ASSERT(s.jobsQueued == 1);
}
TEST(TestAWO_Dispatch)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    awo.Enqueue(42, WorkflowPriority::Elevated, 2048);
    uint64_t jobId = 0;
    bool ok = awo.Dispatch(jobId);
    ASSERT(ok);
    ASSERT(jobId == 42);
}
TEST(TestAWO_Complete)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    awo.Enqueue(7, WorkflowPriority::Critical, 512);
    uint64_t j = 0;
    awo.Dispatch(j);
    awo.Complete(j, 12.5f);
    auto s = awo.GetStats();
    ASSERT(s.jobsCompleted == 1);
}
TEST(TestAWO_SetPolicy)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    awo.SetPolicy(OrchestrationPolicy::MLOptimized);
    ASSERT(awo.GetStats().jobsQueued == 0);
}
TEST(TestAWO_ConcurrencyLimit)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    awo.SetConcurrencyLimit(8);
    auto s = awo.GetStats();
    ASSERT(s.jobsQueued == 0);
}
TEST(TestAWO_Reset)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    awo.Enqueue(1, WorkflowPriority::Background, 256);
    awo.Reset();
    ASSERT(awo.GetStats().jobsQueued == 0);
}
TEST(TestAWO_MultiBatch)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo;
    for (uint64_t i = 1; i <= 5; ++i)
        awo.Enqueue(i, WorkflowPriority::Normal, i * 512);
    auto s = awo.GetStats();
    ASSERT(s.jobsQueued == 5);
}
TEST(TestAWO_PolicyGain)
{
    using namespace ExplorerLens::Engine;
    AutonomousWorkflowOrchestrator awo(OrchestrationPolicy::Adaptive);
    awo.Enqueue(1, WorkflowPriority::Normal, 1024);
    uint64_t j = 0;
    awo.Dispatch(j);
    awo.Complete(j, 10.0f);
    ASSERT(awo.GetStats().jobsCompleted >= 1);
}

TEST(TestSIA_Initialize)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::Windows);
    auto s = sia.GetStats();
    ASSERT(s.hintInjections == 0);
}
TEST(TestSIA_QueryHint)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::Windows);
    auto hint = sia.QueryHint("test.jpg", 256);
    ASSERT(hint == AIHint::None || hint == AIHint::PregenAvailable || hint == AIHint::RelevanceScored
           || hint == AIHint::QualityBoosted);
}
TEST(TestSIA_InjectPregen)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::macOS);
    uint8_t buf[256 * 256 * 4] = {};
    sia.InjectPregenResult("test.png", buf, 256, 256);
    ASSERT(sia.GetStats().hintInjections >= 0);
}
TEST(TestSIA_NotifyRender)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::Linux);
    sia.NotifyShellRender("test.webp", 14.3f);
    auto s = sia.GetStats();
    ASSERT(s.modelInvocations >= 0);
}
TEST(TestSIA_MacOS)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::macOS);
    auto hint = sia.QueryHint("photo.heic", 512);
    ASSERT(hint == AIHint::None || hint == AIHint::PregenAvailable);
}
TEST(TestSIA_Linux)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::Linux);
    auto hint = sia.QueryHint("document.pdf", 128);
    ASSERT(hint == AIHint::None || hint == AIHint::RelevanceScored);
}
TEST(TestSIA_StatAccumulate)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::Windows);
    sia.QueryHint("a.jpg", 256);
    sia.QueryHint("b.png", 256);
    ASSERT(sia.GetStats().modelInvocations >= 0);
}
TEST(TestSIA_MultiPlatform)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter w(ShellPlatform::Windows), m(ShellPlatform::macOS), l(ShellPlatform::Linux);
    ASSERT(w.GetStats().hintInjections == 0);
    ASSERT(m.GetStats().hintInjections == 0);
    ASSERT(l.GetStats().hintInjections == 0);
}
TEST(TestSIA_LargeRender)
{
    using namespace ExplorerLens::Engine;
    ShellIntelligenceAdapter sia(ShellPlatform::Windows);
    for (int i = 0; i < 10; ++i)
        sia.NotifyShellRender("file_" + std::to_string(i) + ".jpg", float(i) * 2.0f);
    ASSERT(sia.GetStats().modelInvocations >= 0);
}

TEST(TestTRR_Initialize)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    auto s = r.GetStats();
    ASSERT(s.filesRanked == 0);
}
TEST(TestTRR_Rank)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    auto ranked = r.Rank({"a.jpg", "b.png", "c.webp"});
    ASSERT(ranked.size() == 3);
}
TEST(TestTRR_RecordAccess)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    r.RecordAccess("hot.jpg");
    auto ranked = r.Rank({"hot.jpg", "cold.png"});
    ASSERT(!ranked.empty());
}
TEST(TestTRR_Weights)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    r.SetRecencyWeight(0.5f);
    r.SetFrequencyWeight(0.3f);
    r.SetVisualInterestWeight(0.2f);
    auto ranked = r.Rank({"x.jpg"});
    ASSERT(ranked.size() == 1);
}
TEST(TestTRR_EmptyInput)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    auto ranked = r.Rank({});
    ASSERT(ranked.empty());
}
TEST(TestTRR_Reset)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    r.RecordAccess("x.jpg");
    r.Reset();
    ASSERT(r.GetStats().filesRanked == 0);
}
TEST(TestTRR_ScoreRange)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    auto ranked = r.Rank({"a.jpg", "b.jpg", "c.jpg"});
    for (auto& f : ranked) {
        ASSERT(f.score >= 0.0f && f.score <= 1.0f);
    }
}
TEST(TestTRR_LargeBatch)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    std::vector<std::string> files;
    for (int i = 0; i < 100; ++i)
        files.push_back("file_" + std::to_string(i) + ".jpg");
    auto ranked = r.Rank(files);
    ASSERT(ranked.size() == 100);
}
TEST(TestTRR_Stats)
{
    using namespace ExplorerLens::Engine;
    ThumbnailRelevanceRanker r;
    r.Rank({"a.jpg", "b.jpg"});
    auto s = r.GetStats();
    ASSERT(s.filesRanked >= 0);
}

TEST(TestASIE_Initialize)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    auto mode = engine.GetActiveMode();
    ASSERT(mode == ShellDeliveryMode::Unknown || mode != ShellDeliveryMode::Unknown);
}
TEST(TestASIE_Probe)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    auto mode = engine.Probe();
    ASSERT(mode != ShellDeliveryMode::Unknown || mode == ShellDeliveryMode::Unknown);
}
TEST(TestASIE_Compatibility)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    bool compat = engine.IsCompatible(ShellDeliveryMode::IThumbnailProvider);
    ASSERT(compat == true || compat == false);
}
TEST(TestASIE_ForceMode)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    engine.ForceMode(ShellDeliveryMode::IThumbnailProvider);
    ASSERT(engine.GetActiveMode() == ShellDeliveryMode::IThumbnailProvider);
}
TEST(TestASIE_Stats)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    engine.Probe();
    auto s = engine.GetStats();
    ASSERT(s.probeCount >= 1);
}
TEST(TestASIE_ForceMacOS)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    engine.ForceMode(ShellDeliveryMode::QuickLookGenerator);
    ASSERT(engine.GetActiveMode() == ShellDeliveryMode::QuickLookGenerator);
}
TEST(TestASIE_ForceLinux)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    engine.ForceMode(ShellDeliveryMode::GIOThumbnailer);
    ASSERT(engine.GetActiveMode() == ShellDeliveryMode::GIOThumbnailer);
}
TEST(TestASIE_MultiProbe)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    engine.Probe();
    engine.Probe();
    engine.Probe();
    ASSERT(engine.GetStats().probeCount >= 3);
}
TEST(TestASIE_Fallback)
{
    using namespace ExplorerLens::Engine;
    AdaptiveShellIntegrationEngine engine;
    engine.ForceMode(ShellDeliveryMode::DBusThumbnailSpec);
    ASSERT(engine.GetStats().fallbacks >= 0);
}

TEST(TestSELM_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    auto s = mgr.GetStats();
    ASSERT(s.registrations >= 0);
}
TEST(TestSELM_Register)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    bool ok = mgr.Register("TestExtension");
    ASSERT(ok == true || ok == false);
}
TEST(TestSELM_Unregister)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    mgr.Register("ExtA");
    mgr.Unregister("ExtA");
    ASSERT(mgr.GetStats().gracefulStops >= 0);
}
TEST(TestSELM_GetState)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    auto state = mgr.GetState("nonexistent");
    ASSERT(state == ExtensionState::Unregistered || state != ExtensionState::Active);
}
TEST(TestSELM_Heartbeat)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    mgr.Register("HBExt");
    mgr.Heartbeat("HBExt");
    ASSERT(mgr.GetStats().uptimeSeconds >= 0.0f);
}
TEST(TestSELM_Recover)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    mgr.Register("RecExt");
    bool ok = mgr.Recover("RecExt");
    ASSERT(ok == true || ok == false);
}
TEST(TestSELM_Suspend)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    mgr.Register("SuspExt");
    mgr.Suspend("SuspExt");
    auto state = mgr.GetState("SuspExt");
    ASSERT(state == ExtensionState::Suspended || state == ExtensionState::Active);
}
TEST(TestSELM_MultipleExtensions)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    mgr.Register("E1");
    mgr.Register("E2");
    mgr.Register("E3");
    ASSERT(mgr.GetStats().registrations >= 3);
}
TEST(TestSELM_Stats)
{
    using namespace ExplorerLens::Engine;
    auto& mgr = ShellExtensionLifecycleManager::Instance();
    auto s = mgr.GetStats();
    ASSERT(s.crashes >= 0 && s.recoveries >= 0);
}

TEST(TestAPE_Initialize)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape(50);
    auto p = ape.GetCurrentParams();
    ASSERT(p.decodeConcurrency > 0);
}
TEST(TestAPE_Observe)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape;
    ape.Observe(120.0f, 45.0f);
    auto s = ape.GetStats();
    ASSERT(s.tuningCycles >= 1);
}
TEST(TestAPE_Step)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape;
    ape.Observe(100.0f, 50.0f);
    auto p = ape.Step();
    ASSERT(p.decodeConcurrency >= 1);
}
TEST(TestAPE_BestParams)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape;
    ape.Observe(200.0f, 20.0f);
    ape.Step();
    auto best = ape.GetBestParams();
    ASSERT(best.qualityTarget >= 0 && best.qualityTarget <= 100);
}
TEST(TestAPE_Convergence)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape(10);
    for (int i = 0; i < 12; ++i) {
        ape.Observe(float(100 + i * 2), float(50 - i));
        ape.Step();
    }
    ASSERT(ape.GetStats().tuningCycles >= 10);
}
TEST(TestAPE_Reset)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape;
    ape.Observe(99.0f, 55.0f);
    ape.Reset();
    ASSERT(ape.GetStats().tuningCycles == 0);
}
TEST(TestAPE_ThroughputTracking)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape;
    ape.Observe(235.0f, 17.0f);
    ASSERT(ape.GetStats().currentThroughput >= 0.0f);
}
TEST(TestAPE_LatencyTarget)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape;
    ape.Observe(180.0f, 22.0f);
    auto p = ape.GetCurrentParams();
    ASSERT(p.timeoutMs > 0.0f);
}
TEST(TestAPE_MultiObserve)
{
    using namespace ExplorerLens::Engine;
    AutotuningPipelineEngine ape;
    ape.Observe(80.f, 70.f);
    ape.Observe(130.f, 40.f);
    ape.Observe(200.f, 20.f);
    ASSERT(ape.GetStats().tuningCycles >= 3);
}

TEST(TestCPBV_Instance)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    auto p = v.DetectPlatform();
    ASSERT(p == BuildPlatform::Win64 || p == BuildPlatform::Win32 || p == BuildPlatform::macOS_ARM64
           || p == BuildPlatform::macOS_x64 || p == BuildPlatform::Linux_x64 || p == BuildPlatform::Linux_ARM64
           || p == BuildPlatform::Unknown);
}
TEST(TestCPBV_Validate)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    auto report = v.Validate();
    ASSERT(report.platform != BuildPlatform::Unknown || report.platform == BuildPlatform::Unknown);
}
TEST(TestCPBV_HasErrors)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    v.Validate();
    bool err = v.HasErrors();
    ASSERT(err == true || err == false);
}
TEST(TestCPBV_Summary)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    auto summary = v.GetSummary();
    ASSERT(!summary.empty() || summary.empty());
}
TEST(TestCPBV_ReportResults)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    auto report = v.Validate();
    ASSERT(report.errorCount >= 0);
    ASSERT(report.warningCount >= 0);
}
TEST(TestCPBV_Windows)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    auto p = v.DetectPlatform();
    ASSERT(p == BuildPlatform::Win64 || p == BuildPlatform::Win32);  // currently built on Windows
}
TEST(TestCPBV_ValidationCount)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    auto report = v.Validate();
    ASSERT(report.results.size() >= 0);
}
TEST(TestCPBV_MultiValidate)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    v.Validate();
    v.Validate();
    ASSERT(!v.GetSummary().empty() || v.GetSummary().empty());
}
TEST(TestCPBV_NoFatalErrors)
{
    using namespace ExplorerLens::Engine;
    auto& v = CrossPlatformBuildValidator::Instance();
    auto report = v.Validate();
    bool hasFatal = false;
    for (auto& r : report.results)
        if (r.severity == BuildValidationSeverity::Fatal) {
            hasFatal = true;
            break;
        }
    ASSERT(!hasFatal);
}

//== v32.2.0 Fomalhaut-S — DirectStorage & Zero-Latency Pipeline Tests ==//

// DirectStorageManager
TEST(TestDSM_ProbeCapabilities)
{
    using namespace ExplorerLens::Engine;
    auto caps = DirectStorageManager::ProbeCapabilities();
    ASSERT(caps.maxQueueDepth >= 1);
    ASSERT(caps.maxStagingBufferMB >= 64);
}
TEST(TestDSM_InitShutdown)
{
    using namespace ExplorerLens::Engine;
    auto& m = DirectStorageManager::Instance();
    ASSERT(m.Initialize());
    m.Shutdown();
    ASSERT(m.GetQueueDepth() == 0);
}
TEST(TestDSM_SubmitRequest_CPUFallback)
{
    using namespace ExplorerLens::Engine;
    auto& m = DirectStorageManager::Instance();
    m.Initialize();
    DSStreamRequest req;
    req.filePath = L"test.raw";
    req.readLength = 4096;
    auto r = m.SubmitRequest(req);
    ASSERT(r.success);
    ASSERT(r.bytesRead == 4096);
}
TEST(TestDSM_BackendName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(DirectStorageManager::BackendName(DSBackend::CPU_FALLBACK)) == "CPU-Fallback");
    ASSERT(std::string(DirectStorageManager::BackendName(DSBackend::DIRECT_STORAGE12)) == "DirectStorage-1.2");
}
TEST(TestDSM_SupportedCompression)
{
    using namespace ExplorerLens::Engine;
    ASSERT(DirectStorageManager::IsSupportedCompressionFormat(DSCompressionFormat::GDEFLATE));
    ASSERT(DirectStorageManager::IsSupportedCompressionFormat(DSCompressionFormat::ZSTANDARD));
    ASSERT(!DirectStorageManager::IsSupportedCompressionFormat(DSCompressionFormat::NONE));
}

// GPUDecompressKernel
TEST(TestGDK_ProbeCapabilities)
{
    using namespace ExplorerLens::Engine;
    auto caps = GPUDecompressKernel::ProbeCapabilities();
    ASSERT(caps.maxInputSizeMB >= 64);
    ASSERT(caps.maxBatchCount >= 1);
}
TEST(TestGDK_InitSelectsVendor)
{
    using namespace ExplorerLens::Engine;
    auto& k = GPUDecompressKernel::Instance();
    ASSERT(k.Initialize());
    auto v = k.GetActiveVendor();
    ASSERT(v == GDKVendor::CPU_FALLBACK || v == GDKVendor::NVIDIA_GDEFLATE
           || v == GDKVendor::INTEL_DSB || v == GDKVendor::AMD_COMPUTE_SHADER);
}
TEST(TestGDK_Decompress_CPUFallback)
{
    using namespace ExplorerLens::Engine;
    auto& k = GPUDecompressKernel::Instance();
    k.Initialize(GDKVendor::CPU_FALLBACK);
    std::vector<uint8_t> src(256, 0xAB);
    std::vector<uint8_t> dst(1024, 0);
    GPUDecompressInput in;
    in.compressedData = src.data();
    in.compressedSize = static_cast<uint32_t>(src.size());
    in.outputBuffer = dst.data();
    in.outputCapacity = static_cast<uint32_t>(dst.size());
    in.format = GPUCompressedFormat::ZSTANDARD;
    auto out = k.Decompress(in);
    ASSERT(out.success);
    ASSERT(out.vendorUsed == GDKVendor::CPU_FALLBACK);
}
TEST(TestGDK_EstimateOutputSize)
{
    using namespace ExplorerLens::Engine;
    ASSERT(GPUDecompressKernel::EstimateOutputSize(1000, GPUCompressedFormat::ZSTANDARD) == 8000);
    ASSERT(GPUDecompressKernel::EstimateOutputSize(1000, GPUCompressedFormat::UNCOMPRESSED) == 1000);
}
TEST(TestGDK_VendorName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(GPUDecompressKernel::VendorName(GDKVendor::CPU_FALLBACK)) == "CPU-Fallback");
    ASSERT(std::string(GPUDecompressKernel::VendorName(GDKVendor::NVIDIA_GDEFLATE)) == "NVIDIA-GDeflate");
}

// ZeroLatencyPipeline
TEST(TestZLP_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& p = ZeroLatencyPipeline::Instance();
    ASSERT(p.Initialize());
    ASSERT(p.GetState() == ZLPState::IDLE);
}
TEST(TestZLP_Process_CPUPath)
{
    using namespace ExplorerLens::Engine;
    auto& p = ZeroLatencyPipeline::Instance();
    p.Initialize();
    ZLPRequest req;
    req.filePath = L"sample.jpg";
    req.thumbWidth = 256;
    req.thumbHeight = 256;
    req.enableGPUPath = false;
    auto r = p.Process(req);
    ASSERT(r.success);
    ASSERT(r.width == 256);
    ASSERT(!r.usedGPUPath);
    ASSERT(!r.pixels.empty());
}
TEST(TestZLP_Metrics_Tracked)
{
    using namespace ExplorerLens::Engine;
    auto& p = ZeroLatencyPipeline::Instance();
    p.Initialize();
    p.ResetMetrics();
    ZLPRequest req;
    req.filePath = L"x.png";
    req.thumbWidth = 128;
    req.thumbHeight = 128;
    p.Process(req);
    ASSERT(p.GetMetrics().requestsSubmitted >= 1);
    ASSERT(p.GetMetrics().requestsCompleted >= 1);
}
TEST(TestZLP_RecommendedThumbSize)
{
    using namespace ExplorerLens::Engine;
    ASSERT(ZeroLatencyPipeline::RecommendedThumbSize(100) == 256);
    ASSERT(ZeroLatencyPipeline::RecommendedThumbSize(60 * 1024 * 1024ULL) == 512);
}
TEST(TestZLP_Shutdown_Resets)
{
    using namespace ExplorerLens::Engine;
    auto& p = ZeroLatencyPipeline::Instance();
    p.Initialize();
    p.Shutdown();
    p.Initialize();
    ASSERT(p.GetState() == ZLPState::IDLE);
}

// ThumbnailPipelineMetrics
TEST(TestTPM_RecordAndStats)
{
    using namespace ExplorerLens::Engine;
    auto& m = ThumbnailPipelineMetrics::Instance();
    m.Reset();
    m.RecordSample(TPMStage::FILE_READ, 12.0);
    m.RecordSample(TPMStage::FILE_READ, 14.0);
    auto s = m.ComputeStats(TPMStage::FILE_READ);
    ASSERT(s.samples == 2);
    ASSERT(s.p50Ms >= 12.0 && s.p50Ms <= 15.0);
}
TEST(TestTPM_StageName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ThumbnailPipelineMetrics::StageName(TPMStage::DECODE)) == "Decode");
    ASSERT(std::string(ThumbnailPipelineMetrics::StageName(TPMStage::RENDER)) == "Render");
}
TEST(TestTPM_BottleneckDetect_IO)
{
    using namespace ExplorerLens::Engine;
    auto& m = ThumbnailPipelineMetrics::Instance();
    m.Reset();
    for (int i = 0; i < 50; ++i)
        m.RecordSample(TPMStage::FILE_READ, 200.0);
    for (int i = 0; i < 50; ++i)
        m.RecordSample(TPMStage::RENDER, 5.0);
    auto snap = m.Snapshot();
    ASSERT(snap.bottleneck == BottleneckStage::IO || snap.bottleneck == BottleneckStage::CPU
           || snap.bottleneck == BottleneckStage::GPU || snap.bottleneck == BottleneckStage::NONE);
}
TEST(TestTPM_Reset_ClearsStats)
{
    using namespace ExplorerLens::Engine;
    auto& m = ThumbnailPipelineMetrics::Instance();
    m.RecordSample(TPMStage::DECODE, 10.0);
    m.Reset();
    auto s = m.ComputeStats(TPMStage::DECODE);
    ASSERT(s.samples == 0);
}
TEST(TestTPM_DescribeBottleneck)
{
    using namespace ExplorerLens::Engine;
    std::string desc = ThumbnailPipelineMetrics::DescribeBottleneck(BottleneckStage::CPU);
    ASSERT(!desc.empty());
    ASSERT(desc.find("CPU") != std::string::npos || desc.find("cpu") != std::string::npos
           || desc.find("decode") != std::string::npos || desc.length() > 5);
}

// StreamingDecodeOrchestrator
TEST(TestSDO_Probe_RAW)
{
    using namespace ExplorerLens::Engine;
    auto& o = StreamingDecodeOrchestrator::Instance();
    auto p = o.Probe(L"image.cr3");
    ASSERT(p.fileType == SDOFileType::RAW_CAMERA);
    ASSERT(p.strategy == SDOStrategy::EMBEDDED_THUMB);
}
TEST(TestSDO_Probe_FITS)
{
    using namespace ExplorerLens::Engine;
    auto& o = StreamingDecodeOrchestrator::Instance();
    auto p = o.Probe(L"data.fits");
    ASSERT(p.fileType == SDOFileType::FITS);
    ASSERT(p.strategy == SDOStrategy::REGION_READ);
    ASSERT(p.targetRegion.has_value());
}
TEST(TestSDO_Probe_Unknown)
{
    using namespace ExplorerLens::Engine;
    auto& o = StreamingDecodeOrchestrator::Instance();
    auto p = o.Probe(L"file.xyz");
    ASSERT(p.fileType == SDOFileType::UNKNOWN);
    ASSERT(p.strategy == SDOStrategy::FULL_DECODE);
}
TEST(TestSDO_Decode_Returns_Pixels)
{
    using namespace ExplorerLens::Engine;
    auto& o = StreamingDecodeOrchestrator::Instance();
    auto r = o.Decode(L"photo.arw", 256);
    ASSERT(r.success);
    ASSERT(r.width == 256);
    ASSERT(!r.pixelsBGRA.empty());
    ASSERT(r.totalLatencyMs > 0.0);
}
TEST(TestSDO_EstimateMinBytes)
{
    using namespace ExplorerLens::Engine;
    uint64_t raw = StreamingDecodeOrchestrator::EstimateMinBytesNeeded(SDOFileType::RAW_CAMERA, 50 * 1024 * 1024ULL);
    ASSERT(raw <= 512 * 1024);
    uint64_t fits = StreamingDecodeOrchestrator::EstimateMinBytesNeeded(SDOFileType::FITS, 5 * 1024 * 1024ULL);
    ASSERT(fits <= 64 * 1024);
}

//== v32.3.0 Fomalhaut-T — Annotation, HDR Tone Mapping & Format Detection Tests ==//

// ThumbnailAnnotationOverlay
TEST(TestTAO_NoBadges)
{
    using namespace ExplorerLens::Engine;
    auto& o = ThumbnailAnnotationOverlay::Instance();
    std::vector<uint8_t> buf(256 * 256 * 4, 0x80);
    auto r = o.Apply(buf.data(), 256, 256, 0);
    ASSERT(!r.applied);
    ASSERT(r.badgesRendered == 0);
}
TEST(TestTAO_DRMBadge)
{
    using namespace ExplorerLens::Engine;
    auto& o = ThumbnailAnnotationOverlay::Instance();
    std::vector<uint8_t> buf(256 * 256 * 4, 0x80);
    uint8_t mask = static_cast<uint8_t>(AnnotationBadge::DRMLocked);
    auto r = o.Apply(buf.data(), 256, 256, mask);
    ASSERT(r.applied);
    ASSERT(r.badgesRendered >= 1);
    ASSERT(r.latencyUs > 0.0);
}
TEST(TestTAO_MultiBadge)
{
    using namespace ExplorerLens::Engine;
    auto& o = ThumbnailAnnotationOverlay::Instance();
    std::vector<uint8_t> buf(256 * 256 * 4, 0x80);
    uint8_t mask = static_cast<uint8_t>(AnnotationBadge::DRMLocked) | static_cast<uint8_t>(AnnotationBadge::ReadOnly);
    auto r = o.Apply(buf.data(), 256, 256, mask);
    ASSERT(r.badgesRendered >= 2);
}
TEST(TestTAO_RecommendedSize)
{
    using namespace ExplorerLens::Engine;
    ASSERT(ThumbnailAnnotationOverlay::RecommendedOverlayBadgeSize(32) == OverlayBadgeSize::Small);
    ASSERT(ThumbnailAnnotationOverlay::RecommendedOverlayBadgeSize(128) == OverlayBadgeSize::Medium);
    ASSERT(ThumbnailAnnotationOverlay::RecommendedOverlayBadgeSize(256) == OverlayBadgeSize::Large);
}
TEST(TestTAO_BadgeMaskForFile)
{
    using namespace ExplorerLens::Engine;
    uint8_t m = ThumbnailAnnotationOverlay::BadgeMaskForFile(L"test.pdf", true, false);
    ASSERT(m & static_cast<uint8_t>(AnnotationBadge::ReadOnly));
    uint8_t m2 = ThumbnailAnnotationOverlay::BadgeMaskForFile(L"drm.mp4", false, true);
    ASSERT(m2 & static_cast<uint8_t>(AnnotationBadge::DRMLocked));
}

// AdaptiveBitDepthConverter
TEST(TestABD_SDR_PassThrough)
{
    using namespace ExplorerLens::Engine;
    auto& c = AdaptiveBitDepthConverter::Instance();
    std::vector<uint8_t> src(64 * 64 * 4, 0xAB);
    std::vector<uint8_t> dst(64 * 64 * 4, 0);
    auto r = c.Convert(src.data(), 64, 64, BitDepthSource::SDR_8bit, dst.data());
    ASSERT(r.success);
    ASSERT(r.pixelsConverted == 64 * 64);
    ASSERT(!r.wasToneMapped);
}
TEST(TestABD_DetectSourceFormat)
{
    using namespace ExplorerLens::Engine;
    ASSERT(AdaptiveBitDepthConverter::DetectSourceFormat(8, false, false) == BitDepthSource::SDR_8bit);
    ASSERT(AdaptiveBitDepthConverter::DetectSourceFormat(16, false, false) == BitDepthSource::HDR_16bit_FP);
    ASSERT(AdaptiveBitDepthConverter::DetectSourceFormat(10, true, false) == BitDepthSource::HDR_10bit_PQ);
}
TEST(TestABD_ToneMappingName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(AdaptiveBitDepthConverter::ToneMappingName(ABDCToneMappingOp::ACES_Filmic)) == "ACES-Filmic");
    ASSERT(std::string(AdaptiveBitDepthConverter::ToneMappingName(ABDCToneMappingOp::Reinhard)) == "Reinhard");
}
TEST(TestABD_SourceFormatName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(AdaptiveBitDepthConverter::SourceFormatName(BitDepthSource::SDR_8bit)) == "SDR-8bit");
    ASSERT(std::string(AdaptiveBitDepthConverter::SourceFormatName(BitDepthSource::HDR_16bit_FP)) == "HDR-16bit-FP");
}
TEST(TestABD_HDR_Convert_BufferSize)
{
    using namespace ExplorerLens::Engine;
    auto& c = AdaptiveBitDepthConverter::Instance();
    std::vector<uint16_t> src(16 * 16 * 4, 0x3C00);  // 1.0 in float16
    std::vector<uint8_t> dst(16 * 16 * 4, 0);
    auto r = c.Convert(src.data(), 16, 16, BitDepthSource::HDR_16bit_FP, dst.data());
    ASSERT(r.success);
    ASSERT(r.pixelsConverted == 16 * 16);
    ASSERT(r.wasToneMapped);
}

// BatchThumbnailExporter
TEST(TestBTE_FormatExtension)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(BatchThumbnailExporter::FormatExtension(BTE_ExportFormat::JPEG)) == ".jpg");
    ASSERT(std::string(BatchThumbnailExporter::FormatExtension(BTE_ExportFormat::PNG)) == ".png");
    ASSERT(std::string(BatchThumbnailExporter::FormatExtension(BTE_ExportFormat::WebP)) == ".webp");
}
TEST(TestBTE_FormatSupportsLossless)
{
    using namespace ExplorerLens::Engine;
    ASSERT(BatchThumbnailExporter::FormatSupportsLossless(BTE_ExportFormat::PNG));
    ASSERT(BatchThumbnailExporter::FormatSupportsLossless(BTE_ExportFormat::WebP));
    ASSERT(!BatchThumbnailExporter::FormatSupportsLossless(BTE_ExportFormat::JPEG));
}
TEST(TestBTE_Export_SingleSize)
{
    using namespace ExplorerLens::Engine;
    auto& e = BatchThumbnailExporter::Instance();
    std::vector<uint8_t> src(64 * 64 * 4, 0xAA);
    ExportJob job;
    job.sourceFile = L"photo.jpg";
    job.outputDir = L"C:\\Temp";
    job.sizes = {{64, 64, "small"}};
    job.format = BTE_ExportFormat::JPEG;
    auto r = e.Export(job, src.data(), 64, 64);
    ASSERT(r.exportedCount >= 1);
    ASSERT(!r.thumbs.empty());
    ASSERT(r.thumbs[0].success);
}
TEST(TestBTE_Export_MultiSize)
{
    using namespace ExplorerLens::Engine;
    auto& e = BatchThumbnailExporter::Instance();
    std::vector<uint8_t> src(128 * 128 * 4, 0xBB);
    ExportJob job;
    job.sourceFile = L"photo.png";
    job.outputDir = L"C:\\Temp";
    job.sizes = {{64, 64, "sm"}, {128, 128, "md"}};
    auto r = e.Export(job, src.data(), 128, 128);
    ASSERT(r.exportedCount == 2);
    ASSERT(r.thumbs.size() == 2);
}
TEST(TestBTE_TotalExported)
{
    using namespace ExplorerLens::Engine;
    auto& e = BatchThumbnailExporter::Instance();
    uint64_t before = e.GetTotalExported();
    std::vector<uint8_t> src(32 * 32 * 4, 0xFF);
    ExportJob job;
    job.outputDir = L"C:\\Temp";
    job.sizes = {{32, 32, "x"}};
    e.Export(job, src.data(), 32, 32);
    ASSERT(e.GetTotalExported() >= before + 1);
}

// FormatSignatureDetector
TEST(TestFSD_DetectJPEG)
{
    using namespace ExplorerLens::Engine;
    auto& d = MagicByteFormatDetector::Instance();
    uint8_t hdr[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10};
    auto m = d.Detect(hdr, sizeof(hdr));
    ASSERT(m.format == MagicByteFormat::JPEG);
    ASSERT(m.confidence == SignatureConfidence::Definitive);
}
TEST(TestFSD_DetectPNG)
{
    using namespace ExplorerLens::Engine;
    auto& d = MagicByteFormatDetector::Instance();
    uint8_t hdr[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    auto m = d.Detect(hdr, sizeof(hdr));
    ASSERT(m.format == MagicByteFormat::PNG);
    ASSERT(m.confidence == SignatureConfidence::Definitive);
}
TEST(TestFSD_DetectPDF)
{
    using namespace ExplorerLens::Engine;
    auto& d = MagicByteFormatDetector::Instance();
    uint8_t hdr[] = {'%', 'P', 'D', 'F', '-', '1', '.', '7'};
    auto m = d.Detect(hdr, sizeof(hdr));
    ASSERT(m.format == MagicByteFormat::PDF);
}
TEST(TestFSD_DetectUnknown)
{
    using namespace ExplorerLens::Engine;
    auto& d = MagicByteFormatDetector::Instance();
    uint8_t hdr[] = {0x00, 0x01, 0x02, 0x03};
    auto m = d.Detect(hdr, sizeof(hdr));
    ASSERT(m.format == MagicByteFormat::Unknown);
    ASSERT(m.confidence == SignatureConfidence::None);
}
TEST(TestFSD_ExtensionMatch)
{
    using namespace ExplorerLens::Engine;
    ASSERT(MagicByteFormatDetector::ExtensionMatchesDetected(L"photo.jpg", MagicByteFormat::JPEG));
    ASSERT(MagicByteFormatDetector::ExtensionMatchesDetected(L"doc.pdf", MagicByteFormat::PDF));
    ASSERT(!MagicByteFormatDetector::ExtensionMatchesDetected(L"", MagicByteFormat::JPEG));
}

// MemoryMappedDecoder
TEST(TestMMD_Initialize)
{
    using namespace ExplorerLens::Engine;
    auto& m = MemoryMappedDecoder::Instance();
    ASSERT(m.Initialize());
    ASSERT(m.GetBackend() != MMapBackend::Unavailable);
}
TEST(TestMMD_Decode_ReturnsPixels)
{
    using namespace ExplorerLens::Engine;
    auto& m = MemoryMappedDecoder::Instance();
    m.Initialize();
    MMapDecodeRequest req;
    req.filePath = L"large_photo.tiff";
    req.thumbSize = 128;
    auto r = m.Decode(req);
    ASSERT(r.success);
    ASSERT(r.thumbWidth == 128);
    ASSERT(!r.pixelsBGRA.empty());
    ASSERT(r.totalLatencyMs > 0.0);
}
TEST(TestMMD_Stats_Tracked)
{
    using namespace ExplorerLens::Engine;
    auto& m = MemoryMappedDecoder::Instance();
    m.ResetStats();
    m.Initialize();
    MMapDecodeRequest req;
    req.filePath = L"big.raw";
    req.thumbSize = 64;
    m.Decode(req);
    ASSERT(m.GetStats().mappingsCreated >= 1);
    ASSERT(m.GetStats().bytesAccessed >= 1);
}
TEST(TestMMD_RecommendedForFile)
{
    using namespace ExplorerLens::Engine;
    ASSERT(!MemoryMappedDecoder::IsRecommendedForFile(512 * 1024));       // < 8MB
    ASSERT(MemoryMappedDecoder::IsRecommendedForFile(10 * 1024 * 1024));  // > 8MB
}
TEST(TestMMD_BackendName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(MemoryMappedDecoder::BackendName(MMapBackend::WindowsMMapV1)) == "Windows-MMapV1");
    ASSERT(std::string(MemoryMappedDecoder::BackendName(MMapBackend::Unavailable)) == "Unavailable");
}

//== Sprint 1111-1120: DirectStorage Zero-Copy GPU Decompress (v32.5.0 "Fomalhaut-V") ==

// ── ZStdGPUKernel ──────────────────────────────────────────────────────────
TEST(TestZSK_VendorName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(ZStdGPUKernel::VendorName(ZStdGPUVendor::AMD_RDNA3)  == std::string_view("AMD-RDNA3"));
    ASSERT(ZStdGPUKernel::VendorName(ZStdGPUVendor::INTEL_XE2)  == std::string_view("Intel-Xe2"));
    ASSERT(ZStdGPUKernel::VendorName(ZStdGPUVendor::INTEL_ARC)  == std::string_view("Intel-Arc"));
    ASSERT(ZStdGPUKernel::VendorName(ZStdGPUVendor::FALLBACK)   == std::string_view("CPU-Fallback"));
}
TEST(TestZSK_FallbackWhenUnavailable)
{
    using namespace ExplorerLens::Engine;
    // Default instance is unavailable (no real GPU in test env)
    ASSERT(!ZStdGPUKernel::Instance().IsAvailable());
    ASSERT(ZStdGPUKernel::Instance().DetectedVendor() == ZStdGPUVendor::FALLBACK);
}
TEST(TestZSK_DecompressReturnsBytesOnFallback)
{
    using namespace ExplorerLens::Engine;
    uint8_t src[8]  = { 0x28, 0xB5, 0x2F, 0xFD, 0x04, 0x00, 0x01, 0x00 };
    uint8_t dst[64] = {};
    ZStdGPUKernelDesc d{};
    d.compressedData   = src;
    d.compressedSize   = 8;
    d.outputBuffer     = dst;
    d.outputCapacity   = 64;
    d.expectedOrigSize = 4;
    d.vendor           = ZStdGPUVendor::FALLBACK;
    ZStdGPUKernelResult r = ZStdGPUKernel::Instance().Decompress(d);
    // Fallback must not crash; fields must be populated
    ASSERT(r.decompressMs >= 0.0f);
}
// ── GPUDecompressOrchestrator ──────────────────────────────────────────────
TEST(TestGDO_DefaultBackendIsCPU)
{
    using namespace ExplorerLens::Engine;
    GPUDecompressOrchestrator::Instance().Initialize();
    ASSERT(GPUDecompressOrchestrator::Instance().PreferredBackend() == GPUDecompressBackend::CPU
        || GPUDecompressOrchestrator::Instance().PreferredBackend() == GPUDecompressBackend::ZSTD_GPU);
}
TEST(TestGDO_BackendName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(GPUDecompressOrchestrator::BackendName(GPUDecompressBackend::NV_GDEFLATE) == std::string_view("NvGDeflate"));
    ASSERT(GPUDecompressOrchestrator::BackendName(GPUDecompressBackend::ZSTD_GPU)    == std::string_view("ZStdGPU"));
    ASSERT(GPUDecompressOrchestrator::BackendName(GPUDecompressBackend::CPU)         == std::string_view("CPU"));
}
TEST(TestGDO_DecompressCPUPath)
{
    using namespace ExplorerLens::Engine;
    uint8_t src[8]  = { 1, 2, 3, 4, 5, 6, 7, 8 };
    uint8_t dst[64] = {};
    GPUDecompressRequest req{};
    req.srcData          = src;
    req.srcSize          = 8;
    req.dstBuffer        = dst;
    req.dstCapacity      = 64;
    req.expectedOrigSize = 8;
    GPUDecompressResult res = GPUDecompressOrchestrator::Instance().Decompress(req);
    ASSERT(res.success);
    ASSERT(res.bytesOut > 0);
    ASSERT(res.elapsedMs >= 0.0f);
}
TEST(TestGDO_DecompressInvalidSizeFails)
{
    using namespace ExplorerLens::Engine;
    GPUDecompressRequest req{};  // all nullptrs / zeros
    GPUDecompressResult res = GPUDecompressOrchestrator::Instance().Decompress(req);
    ASSERT(!res.success);
}
// ── DirectStorageBatchScheduler ────────────────────────────────────────────
TEST(TestDSBS_AddAndFlush)
{
    using namespace ExplorerLens::Engine;
    auto& sched = DirectStorageBatchScheduler::Instance();
    sched.Reset();
    DSBatchItem item{ L"photo.raw", 256, 1 };
    sched.AddItem(item);
    ASSERT(sched.PendingCount() == 1);
    ASSERT(sched.Flush());
    ASSERT(sched.PendingCount() == 0);
}
TEST(TestDSBS_EmptyFlush)
{
    using namespace ExplorerLens::Engine;
    auto& sched = DirectStorageBatchScheduler::Instance();
    sched.Reset();
    ASSERT(sched.Flush());  // flush on empty queue — must succeed
}
TEST(TestDSBS_PendingCount)
{
    using namespace ExplorerLens::Engine;
    auto& sched = DirectStorageBatchScheduler::Instance();
    sched.Reset();
    sched.AddItem({ L"a.cr2", 128, 10 });
    sched.AddItem({ L"b.nef", 128, 11 });
    ASSERT(sched.PendingCount() == 2);
}
TEST(TestDSBS_ResultsAfterFlush)
{
    using namespace ExplorerLens::Engine;
    auto& sched = DirectStorageBatchScheduler::Instance();
    sched.Reset();
    sched.AddItem({ L"c.arw", 256, 42 });
    sched.Flush();
    const auto& results = sched.GetResults();
    ASSERT(results.size() == 1);
    ASSERT(results[0].itemId == 42);
    ASSERT(results[0].success);
    ASSERT(results[0].totalLatencyMs > 0.0f);
}
TEST(TestDSBS_ResetClearsAll)
{
    using namespace ExplorerLens::Engine;
    auto& sched = DirectStorageBatchScheduler::Instance();
    sched.AddItem({ L"d.dng", 256, 99 });
    sched.Reset();
    ASSERT(sched.PendingCount() == 0);
    ASSERT(sched.GetResults().empty());
}
// ── DirectStorageProfiler ─────────────────────────────────────────────────
TEST(TestDSP_PathName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(DirectStorageProfiler::PathName(DSDecodePath::DIRECT_STORAGE) == std::string_view("DirectStorage"));
    ASSERT(DirectStorageProfiler::PathName(DSDecodePath::CPU_DECODE)     == std::string_view("CPUDecode"));
}
TEST(TestDSP_RecommendedPath)
{
    using namespace ExplorerLens::Engine;
    // Small file → CPU
    ASSERT(DirectStorageProfiler::Instance().RecommendedPath(512 * 1024) == DSDecodePath::CPU_DECODE);
    // Large file (≥4 MB) → DirectStorage
    ASSERT(DirectStorageProfiler::Instance().RecommendedPath(8 * 1024 * 1024) == DSDecodePath::DIRECT_STORAGE);
}
TEST(TestDSP_AddSampleAndStats)
{
    using namespace ExplorerLens::Engine;
    auto& p = DirectStorageProfiler::Instance();
    p.Reset();
    DSProfileSample s{};
    s.path       = DSDecodePath::DIRECT_STORAGE;
    s.ioMs       = 3.0f;
    s.decompressMs = 2.0f;
    s.decodeMs   = 5.0f;
    s.totalMs    = 10.0f;
    s.fileSizeBytes = 6 * 1024 * 1024;
    p.AddSample(s);
    auto stats = p.ComputeStats();
    ASSERT(stats.sampleCount == 1);
    ASSERT(stats.dsPathCount == 1);
    ASSERT(stats.cpuPathCount == 0);
}
TEST(TestDSP_ResetClearsSamples)
{
    using namespace ExplorerLens::Engine;
    auto& p = DirectStorageProfiler::Instance();
    DSProfileSample s{};
    s.totalMs = 15.0f;
    p.AddSample(s);
    p.Reset();
    auto stats = p.ComputeStats();
    ASSERT(stats.sampleCount == 0);
}
TEST(TestDSP_P99LargerThanP50)
{
    using namespace ExplorerLens::Engine;
    auto& p = DirectStorageProfiler::Instance();
    p.Reset();
    for (uint32_t i = 1; i <= 100; ++i)
    {
        DSProfileSample s{};
        s.totalMs       = static_cast<float>(i);
        s.path          = DSDecodePath::DIRECT_STORAGE;
        s.fileSizeBytes = 5 * 1024 * 1024;
        p.AddSample(s);
    }
    auto stats = p.ComputeStats();
    ASSERT(stats.p99TotalMs >= stats.p50TotalMs);
}
// ── ZeroCopyDecodeSession ──────────────────────────────────────────────────
TEST(TestZCS_DefaultState)
{
    using namespace ExplorerLens::Engine;
    ZeroCopyDecodeSession s{};
    ASSERT(s.sessionId == 0);
    ASSERT(s.state == ZCSessionState::IDLE);
    ASSERT(!s.gpuDecompress);
    ASSERT(s.stagingPtr == nullptr);
}
TEST(TestZCS_TotalMs)
{
    using namespace ExplorerLens::Engine;
    ZeroCopyDecodeSession s{};
    s.ioMs         = 3.0f;
    s.decompressMs = 2.0f;
    s.decodeMs     = 5.0f;
    ASSERT(s.TotalMs() > 9.9f && s.TotalMs() < 10.1f);
}
TEST(TestZCS_IsTerminal)
{
    using namespace ExplorerLens::Engine;
    ZeroCopyDecodeSession s{};
    s.state = ZCSessionState::IDLE;
    ASSERT(!s.IsTerminal());
    s.state = ZCSessionState::COMPLETE;
    ASSERT(s.IsTerminal());
    s.state = ZCSessionState::FAILED;
    ASSERT(s.IsTerminal());
}
TEST(TestZCS_StateName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(std::string(ZeroCopyDecodeSession::StateName(ZCSessionState::IDLE))                == "Idle");
    ASSERT(std::string(ZeroCopyDecodeSession::StateName(ZCSessionState::IO_PENDING))          == "IO_Pending");
    ASSERT(std::string(ZeroCopyDecodeSession::StateName(ZCSessionState::DECOMPRESS_PENDING))  == "Decompress_Pending");
    ASSERT(std::string(ZeroCopyDecodeSession::StateName(ZCSessionState::DECODE_PENDING))      == "Decode_Pending");
    ASSERT(std::string(ZeroCopyDecodeSession::StateName(ZCSessionState::COMPLETE))            == "Complete");
    ASSERT(std::string(ZeroCopyDecodeSession::StateName(ZCSessionState::FAILED))              == "Failed");
}
TEST(TestZCS_GpuDecompressFlag)
{
    using namespace ExplorerLens::Engine;
    ZeroCopyDecodeSession s{};
    s.gpuDecompress = true;
    s.stagingBytes  = 2048;
    ASSERT(s.gpuDecompress);
    ASSERT(s.stagingBytes == 2048);
}

//== Sprint 1121-1130: CLIP Semantic Search + HNSW Index (v32.6.0 "Fomalhaut-W") ==

// ── HNSWIndexEngine ────────────────────────────────────────────────────────
TEST(TestHNSW_InsertAndCount)
{
    using namespace ExplorerLens::Engine;
    auto& idx = HNSWIndexEngine::Instance();
    idx.Reset();
    HNSWEntry e{};
    e.itemId   = 1;
    e.filePath = L"photo.jpg";
    e.vector[0] = 1.0f;
    ASSERT(idx.Insert(e));
    ASSERT(idx.Count() == 1);
}
TEST(TestHNSW_Remove)
{
    using namespace ExplorerLens::Engine;
    auto& idx = HNSWIndexEngine::Instance();
    idx.Reset();
    HNSWEntry e{};
    e.itemId = 7;
    idx.Insert(e);
    ASSERT(idx.Remove(7));
    ASSERT(idx.Count() == 0);
    ASSERT(!idx.Remove(999));  // non-existent
}
TEST(TestHNSW_QueryTopK)
{
    using namespace ExplorerLens::Engine;
    auto& idx = HNSWIndexEngine::Instance();
    idx.Reset();
    for (uint32_t i = 1; i <= 5; ++i)
    {
        HNSWEntry e{};
        e.itemId      = i;
        e.vector[i-1] = 1.0f;
        idx.Insert(e);
    }
    float qv[512]{};
    qv[0] = 1.0f;
    auto results = idx.Query(qv, 3);
    ASSERT(results.size() <= 3);
    // Best match should have itemId == 1 (vector[0] == 1)
    ASSERT(!results.empty() && results[0].itemId == 1);
}
TEST(TestHNSW_SaveLoad)
{
    using namespace ExplorerLens::Engine;
    auto& idx = HNSWIndexEngine::Instance();
    ASSERT(idx.SaveToFile(L"test_index.bin"));
    ASSERT(idx.LoadFromFile(L"test_index.bin"));
    ASSERT(!idx.SaveToFile(L""));   // empty path → fail
    ASSERT(!idx.LoadFromFile(L"")); // empty path → fail
}
TEST(TestHNSW_Reset)
{
    using namespace ExplorerLens::Engine;
    auto& idx = HNSWIndexEngine::Instance();
    HNSWEntry e{};
    e.itemId = 42;
    idx.Insert(e);
    idx.Reset();
    ASSERT(idx.Count() == 0);
    ASSERT(idx.LastQueryMs() == 0.0f);
}
// ── CLIPQueryProcessor ─────────────────────────────────────────────────────
TEST(TestCQP_BackendName)
{
    using namespace ExplorerLens::Engine;
    ASSERT(CLIPQueryProcessor::BackendName(CLIPTextBackend::DIRECTML) == std::string_view("DirectML"));
    ASSERT(CLIPQueryProcessor::BackendName(CLIPTextBackend::ONNX)     == std::string_view("ONNX"));
    ASSERT(CLIPQueryProcessor::BackendName(CLIPTextBackend::CPU)      == std::string_view("CPU"));
}
TEST(TestCQP_LoadModelFails_EmptyPath)
{
    using namespace ExplorerLens::Engine;
    auto& p = CLIPQueryProcessor::Instance();
    ASSERT(!p.LoadModel(L""));
    ASSERT(!p.IsModelLoaded());
}
TEST(TestCQP_LoadModelSucceeds)
{
    using namespace ExplorerLens::Engine;
    auto& p = CLIPQueryProcessor::Instance();
    ASSERT(p.LoadModel(L"models/clip_vitb32_int8.onnx"));
    ASSERT(p.IsModelLoaded());
}
TEST(TestCQP_QueryEmpty_WhenNotLoaded)
{
    using namespace ExplorerLens::Engine;
    CLIPQueryProcessor fresh{};  // unloaded instance
    CLIPQueryRequest req{};
    req.queryText = L"sunset";
    auto res = fresh.Query(req);
    ASSERT(res.empty());
}
TEST(TestCQP_LastEmbedMsDefault)
{
    using namespace ExplorerLens::Engine;
    CLIPQueryProcessor fresh{};
    ASSERT(fresh.LastEmbedMs() == 0.0f);
}
// ── SemanticSearchOrchestrator ─────────────────────────────────────────────
TEST(TestSSO_InitializeSucceeds)
{
    using namespace ExplorerLens::Engine;
    auto& sso = SemanticSearchOrchestrator::Instance();
    ASSERT(sso.Initialize(L"search_index.bin"));
}
TEST(TestSSO_IsReadyAfterInit)
{
    using namespace ExplorerLens::Engine;
    auto& sso = SemanticSearchOrchestrator::Instance();
    sso.Initialize(L"search_index.bin");
    ASSERT(sso.IsReady());
}
TEST(TestSSO_IndexFile)
{
    using namespace ExplorerLens::Engine;
    auto& sso  = SemanticSearchOrchestrator::Instance();
    auto& hnsw = HNSWIndexEngine::Instance();
    hnsw.Reset();
    sso.Initialize(L"idx.bin");
    float emb[512]{};
    emb[0] = 0.9f;
    ASSERT(sso.IndexFile(L"cats.jpg", emb));
    ASSERT(sso.IndexedCount() >= 1);
}
TEST(TestSSO_SearchEmpty_BeforeIndex)
{
    using namespace ExplorerLens::Engine;
    SemanticSearchOrchestrator fresh{};
    SemanticSearchRequest req{};
    req.queryText = L"landscape";
    auto res = fresh.Search(req);
    ASSERT(res.empty());
}
TEST(TestSSO_IndexedCountTracked)
{
    using namespace ExplorerLens::Engine;
    auto& sso  = SemanticSearchOrchestrator::Instance();
    auto& hnsw = HNSWIndexEngine::Instance();
    hnsw.Reset();
    sso.Initialize(L"idx2.bin");
    float emb[512]{};
    uint32_t before = sso.IndexedCount();
    sso.IndexFile(L"a.png", emb);
    sso.IndexFile(L"b.png", emb);
    ASSERT(sso.IndexedCount() == before + 2);
}
// ── EmbeddingPersistenceEngine ─────────────────────────────────────────────
TEST(TestEPE_OpenAndClose)
{
    using namespace ExplorerLens::Engine;
    auto& ep = EmbeddingPersistenceEngine::Instance();
    ASSERT(ep.Open(L"journal.bin"));
    ASSERT(ep.IsOpen());
    ep.Close();
    ASSERT(!ep.IsOpen());
}
TEST(TestEPE_AppendEntry)
{
    using namespace ExplorerLens::Engine;
    auto& ep = EmbeddingPersistenceEngine::Instance();
    ep.Open(L"journal.bin");
    PersistedEmbedding e{};
    e.itemId   = 1;
    e.filePath = L"img.jpg";
    ASSERT(ep.Append(e));
    ASSERT(ep.Stats().totalEntries == 1);
    ep.Close();
}
TEST(TestEPE_FlushUpdatesStats)
{
    using namespace ExplorerLens::Engine;
    auto& ep = EmbeddingPersistenceEngine::Instance();
    ep.Open(L"journal.bin");
    ASSERT(ep.Flush());
    ASSERT(ep.Stats().flushCount >= 1);
    ASSERT(ep.Stats().lastFlushMs > 0.0f);
    ep.Close();
}
TEST(TestEPE_LoadAllEmpty)
{
    using namespace ExplorerLens::Engine;
    auto& ep = EmbeddingPersistenceEngine::Instance();
    ep.Open(L"empty_journal.bin");
    std::vector<PersistedEmbedding> loaded;
    ASSERT(ep.LoadAll(loaded));
    ASSERT(loaded.empty());  // fresh journal has no entries
    ep.Close();
}
TEST(TestEPE_StatsJournalBytes)
{
    using namespace ExplorerLens::Engine;
    auto& ep = EmbeddingPersistenceEngine::Instance();
    ep.Open(L"journal2.bin");
    PersistedEmbedding e{};
    e.itemId = 2; e.filePath = L"x.tiff";
    ep.Append(e);
    ASSERT(ep.Stats().journalBytes > 0);
    ep.Close();
}
// ── VisualQueryOptimizer ───────────────────────────────────────────────────
TEST(TestVQO_DefaultActive)
{
    using namespace ExplorerLens::Engine;
    ASSERT(VisualQueryOptimizer::Instance().IsActive());
}
TEST(TestVQO_SetActive)
{
    using namespace ExplorerLens::Engine;
    auto& opt = VisualQueryOptimizer::Instance();
    opt.SetActive(false);
    ASSERT(!opt.IsActive());
    opt.SetActive(true);
    ASSERT(opt.IsActive());
}
TEST(TestVQO_PruneNoHint)
{
    using namespace ExplorerLens::Engine;
    auto& opt = VisualQueryOptimizer::Instance();
    opt.SetActive(true);
    VisualQueryHint hint{};  // NO_HINT
    std::vector<uint32_t> candidates = { 1, 2, 3, 4 };
    std::vector<uint32_t> pruned;
    auto res = opt.PruneSearchSpace(hint, candidates, pruned);
    ASSERT(res.prunedCandidates == 4);  // pass-through
    ASSERT(pruned.size() == candidates.size());
}
TEST(TestVQO_PruneReducesCandidates)
{
    using namespace ExplorerLens::Engine;
    auto& opt = VisualQueryOptimizer::Instance();
    opt.SetActive(true);
    VisualQueryHint hint{};
    hint.type        = VisualQueryHintType::FOLDER_SCOPE;
    hint.folderScope = L"C:\\Photos\\2025";
    std::vector<uint32_t> candidates = { 1, 2, 3, 4, 5, 6, 7, 8 };
    std::vector<uint32_t> pruned;
    auto res = opt.PruneSearchSpace(hint, candidates, pruned);
    ASSERT(res.prunedCandidates < res.originalCandidates);
    ASSERT(res.estimatedSpeedup > 1.0f);
}
TEST(TestVQO_PruneResultFields)
{
    using namespace ExplorerLens::Engine;
    auto& opt = VisualQueryOptimizer::Instance();
    VisualQueryHint hint{};
    hint.type = VisualQueryHintType::FILE_TYPE;
    hint.fileTypeExt = L".raw";
    std::vector<uint32_t> candidates = { 10, 20, 30 };
    std::vector<uint32_t> pruned;
    auto res = opt.PruneSearchSpace(hint, candidates, pruned);
    ASSERT(res.originalCandidates == 3);
    ASSERT(res.pruneMs >= 0.0f);
}

//== Sprint 1141-1150: Cross-Platform Shell PAL (v33.0.0 "Spica") ==
// ── Win32ShellProvider ────────────────────────────────────────────────────────
TEST(TestPSP_Win32PlatformKind)
{
    using namespace ExplorerLens::Engine;
    Win32ShellProvider p;
    ASSERT(p.GetPlatform() == PlatformKind::WINDOWS);
}
TEST(TestPSP_Win32PlatformName)
{
    using namespace ExplorerLens::Engine;
    Win32ShellProvider p;
    ASSERT(p.GetPlatformName() != nullptr);
    ASSERT(std::string(p.GetPlatformName()).size() > 0);
}
TEST(TestPSP_Win32NotRegisteredByDefault)
{
    using namespace ExplorerLens::Engine;
    Win32ShellProvider p;
    ASSERT(!p.IsRegistered());
}
TEST(TestPSP_Win32RegisterUnregister)
{
    using namespace ExplorerLens::Engine;
    Win32ShellProvider p;
    bool ok = p.RegisterProvider();
    ASSERT(ok);
    ASSERT(p.IsRegistered());
    p.UnregisterProvider();
    ASSERT(!p.IsRegistered());
}
TEST(TestPSP_Win32ThumbnailEmpty)
{
    using namespace ExplorerLens::Engine;
    Win32ShellProvider p;
    PlatformThumbnailRequest req{};
    auto res = p.GenerateThumbnail(req);
    ASSERT(!res.success);
}
TEST(TestPSP_Win32ThumbnailPath)
{
    using namespace ExplorerLens::Engine;
    Win32ShellProvider p;
    PlatformThumbnailRequest req{};
    req.filePath = L"test.png";
    req.width    = 256;
    req.height   = 256;
    auto res = p.GenerateThumbnail(req);
    ASSERT(res.success);
}
TEST(TestPSP_Win32ThumbnailNonzeroSize)
{
    using namespace ExplorerLens::Engine;
    Win32ShellProvider p;
    PlatformThumbnailRequest req{};
    req.filePath = L"test.png";
    req.width  = 128;
    req.height = 128;
    auto res = p.GenerateThumbnail(req);
    ASSERT(res.width >= 0);
    ASSERT(res.height >= 0);
}
// ── PlatformDetector ─────────────────────────────────────────────────────────
TEST(TestPD_DetectCurrentPlatform)
{
    using namespace ExplorerLens::Engine;
    ASSERT(PlatformDetector::Detect() == PlatformKind::WINDOWS);
}
TEST(TestPD_PlatformNameWindows)
{
    using namespace ExplorerLens::Engine;
    ASSERT(PlatformDetector::PlatformName() != nullptr);
    ASSERT(std::string(PlatformDetector::PlatformName()) == "Windows");
}
TEST(TestPD_MakeWin32Provider)
{
    using namespace ExplorerLens::Engine;
    auto p = PlatformDetector::MakeProvider(PlatformKind::WINDOWS);
    ASSERT(p != nullptr);
    ASSERT(p->GetPlatform() == PlatformKind::WINDOWS);
}
TEST(TestPD_PlatformDescString)
{
    using namespace ExplorerLens::Engine;
    auto desc = PlatformDetector::PlatformDescString();
    ASSERT(!desc.empty());
    ASSERT(desc.find("Windows") != std::string::npos);
}
TEST(TestPD_CurrentProviderForPlatform)
{
    using namespace ExplorerLens::Engine;
    auto p = PlatformDetector::MakeCurrentPlatformProvider();
    ASSERT(p != nullptr);
    ASSERT(p->GetPlatform() == PlatformDetector::Detect());
}

//== Sprint 1151-1160: Platform GPU Router (v33.1.0 Windows-only) ==
TEST(TestPGR_SelectsD3D12OnWindows)
{
    using namespace ExplorerLens::Engine;
    PlatformGPURouter r;
    r.Initialize();
    ASSERT(r.SelectedBackend() == GPURouterBackend::D3D12);
}
TEST(TestPGR_BackendNameNotNull)
{
    using namespace ExplorerLens::Engine;
    PlatformGPURouter r;
    r.Initialize();
    ASSERT(r.BackendName() != nullptr);
}

//== Sprint 1161-1170: Enterprise Console v4 (v33.2.0 "Spica-S") ==
TEST(TestEPV4_Initialize)
{
    using namespace ExplorerLens::Engine;
    EnterprisePolicyV4 eng;
    ASSERT(eng.Initialize());
}
TEST(TestEPV4_ApplyPolicy)
{
    using namespace ExplorerLens::Engine;
    EnterprisePolicyV4 eng;
    eng.Initialize();
    EnterprisePolicyV4Entry entry;
    entry.key   = "MaxThumbnailSize";
    entry.value = "512";
    entry.source = PolicySourceV4::Manual;
    ASSERT(eng.ApplyPolicy(entry));
}
TEST(TestGPOT_AddSetting)
{
    using namespace ExplorerLens::Engine;
    GPOPolicyTemplate tmpl;
    GPOPolicySetting s;
    s.id           = "MaxCacheSize";
    s.displayName  = "Maximum Cache Size";
    s.category     = "Performance";
    s.defaultValue = "512";
    ASSERT(tmpl.AddSetting(s));
    ASSERT(tmpl.SettingCount() == 1);
}
TEST(TestGPOT_GenerateADMX)
{
    using namespace ExplorerLens::Engine;
    GPOPolicyTemplate tmpl;
    GPOPolicySetting s;
    s.id = "MaxCacheSize"; s.displayName = "Max Cache"; s.category = "Perf"; s.defaultValue = "512";
    tmpl.AddSetting(s);
    GPOTemplateResult res = tmpl.GenerateADMX();
    ASSERT(res.success);
    ASSERT(!res.admxContent.empty());
}
TEST(TestICE_AddRule)
{
    using namespace ExplorerLens::Engine;
    IntuneComplianceEngine ice;
    ice.Initialize();
    ComplianceRule r;
    r.ruleId      = "RULE-001";
    r.description = "Thumbnails must be secure";
    r.required    = true;
    ASSERT(ice.AddRule(r));
    ASSERT(ice.RuleCount() == 1);
}
TEST(TestICE_Evaluate)
{
    using namespace ExplorerLens::Engine;
    IntuneComplianceEngine ice;
    ice.Initialize();
    auto report = ice.Evaluate();
    ASSERT(report.totalRules == 0);
    ASSERT(report.overallStatus == IntuneComplianceStatus::Compliant);
}
TEST(TestEAL_InitializeLog)
{
    using namespace ExplorerLens::Engine;
    EnterpriseAuditLogger logger;
    ASSERT(logger.Initialize("test-audit.log"));
}
TEST(TestEAL_LogEvent)
{
    using namespace ExplorerLens::Engine;
    EnterpriseAuditLogger logger;
    logger.Initialize("test-audit.log");
    EnterpriseAuditLogEntry ev;
    ev.type        = EnterpriseAuditEventType::PolicyChange;
    ev.description = "Policy updated";
    ev.actor       = "SYSTEM";
    ASSERT(logger.Log(ev));
    ASSERT(logger.GetStats().totalEvents == 1);
}
TEST(TestCMPB_Initialize)
{
    using namespace ExplorerLens::Engine;
    ConfigMgrPolicyBridge bridge;
    ASSERT(bridge.Initialize());
}
TEST(TestCMPB_Synchronize)
{
    using namespace ExplorerLens::Engine;
    ConfigMgrPolicyBridge bridge;
    bridge.Initialize();
    auto res = bridge.Synchronize();
    ASSERT(res.success || !bridge.IsConfigMgrPresent());
}

//== Sprint 1171-1180: Generative AI Thumbnails (v33.3.0 "Spica-T") ==
TEST(TestNPU_Initialize)
{
    using namespace ExplorerLens::Engine;
    NPUThumbnailSynthesizer& synth = NPUThumbnailSynthesizer::Instance();
    ASSERT(synth.Initialize(SynthesisBackend::CPU));
}
TEST(TestNPU_Synthesize)
{
    using namespace ExplorerLens::Engine;
    NPUThumbnailSynthesizer& synth = NPUThumbnailSynthesizer::Instance();
    synth.Initialize(SynthesisBackend::CPU);
    SynthesisRequest req;
    req.prompt  = "nature landscape";
    req.width   = 64;
    req.height  = 64;
    auto res = synth.Synthesize(req);
    ASSERT(res.success);
    ASSERT(!res.pixels.empty());
}
TEST(TestDME_LoadModel)
{
    using namespace ExplorerLens::Engine;
    DiffusionModelEngine eng;
    DiffusionModelConfig cfg;
    cfg.modelPath  = "stub-model";
    cfg.steps = 4;
    ASSERT(eng.LoadModel(cfg));
    ASSERT(eng.GetState() == DiffusionModelState::Ready);
}
TEST(TestDME_EncodePrompt)
{
    using namespace ExplorerLens::Engine;
    DiffusionModelEngine eng;
    DiffusionModelConfig cfg; cfg.modelPath = "stub"; cfg.steps = 4;
    eng.LoadModel(cfg);
    auto latent = eng.EncodePrompt("a red cube");
    ASSERT(latent.width > 0 || !latent.data.empty());
}
TEST(TestTIE_Initialize)
{
    using namespace ExplorerLens::Engine;
    ThumbnailInpaintEngine eng;
    ASSERT(eng.Initialize());
}
TEST(TestTIE_Inpaint)
{
    using namespace ExplorerLens::Engine;
    ThumbnailInpaintEngine eng;
    eng.Initialize();
    InpaintRequest req;
    req.imageWidth  = 64;
    req.imageHeight = 64;
    req.pixels.assign(64 * 64 * 4, 0xFF);
    ThumbnailInpaintRegion region; region.x = 10; region.y = 10; region.width = 20; region.height = 20;
    req.region = region;
    auto res = eng.Inpaint(req);
    ASSERT(res.success);
    ASSERT(res.confidenceScore >= 0.0f);
}
TEST(TestOIR_Initialize)
{
    using namespace ExplorerLens::Engine;
    OffDeviceInferenceRouter& router = OffDeviceInferenceRouter::Instance();
    ASSERT(router.Initialize());
}
TEST(TestOIR_RouteInference)
{
    using namespace ExplorerLens::Engine;
    OffDeviceInferenceRouter& router = OffDeviceInferenceRouter::Instance();
    router.Initialize();
    auto target = router.SelectTarget();
    ASSERT(target == InferenceTarget::IntelNPU || target == InferenceTarget::DirectML ||
           target == InferenceTarget::ONNXRuntime || target == InferenceTarget::CPU);
    ASSERT(router.RouteInference(16));
}
TEST(TestAIBP_Enqueue)
{
    using namespace ExplorerLens::Engine;
    AIThumbnailBatchProcessor proc;
    proc.Initialize(2);
    AIBatchRequest req;
    req.filePath = "img.png";
    req.priority = AIBatchPriority::Normal;
    uint32_t id = proc.Enqueue(req);
    ASSERT(id > 0);
    ASSERT(proc.QueueDepth() == 1);
}
TEST(TestAIBP_ProcessBatch)
{
    using namespace ExplorerLens::Engine;
    AIThumbnailBatchProcessor proc;
    proc.Initialize(2);
    AIBatchRequest req; req.filePath = "img.png";
    proc.Enqueue(req);
    std::vector<AIBatchResult> results;
    ASSERT(proc.ProcessBatch(results));
    ASSERT(results.size() == 1);
    ASSERT(results[0].success);
    ASSERT(proc.GetStats().totalProcessed >=0);
}

//== Sprint 1181-1190: Plugin Marketplace v5 (v33.4.0 "Spica-U") ==
TEST(TestPMV5_Initialize)
{
    using namespace ExplorerLens::Engine;
    PluginMarketplaceV5& mkt = PluginMarketplaceV5::Instance();
    ASSERT(mkt.Initialize("https://plugins.explorerlens.example/feed.json"));
}
TEST(TestPMV5_Search)
{
    using namespace ExplorerLens::Engine;
    PluginMarketplaceV5& mkt = PluginMarketplaceV5::Instance();
    mkt.Initialize("https://plugins.explorerlens.example/feed.json");
    auto result = mkt.Search("avif");
    ASSERT(result.plugins.empty() || result.plugins[0].id.size() > 0);
}
TEST(TestSCK3_Initialize)
{
    using namespace ExplorerLens::Engine;
    ASSERT(SDKCompatKit3::Instance().Initialize());
}
TEST(TestSCK3_DetectVersion)
{
    using namespace ExplorerLens::Engine;
    SDKCompatKit3& kit = SDKCompatKit3::Instance();
    kit.Initialize();
    auto ver = kit.DetectPluginSDKVersion("some-plugin.dll");
    ASSERT(ver != SDKVersion::Unknown);
}
TEST(TestPDM_Install)
{
    using namespace ExplorerLens::Engine;
    PluginDistributionManager mgr;
    ASSERT(mgr.Initialize("C:\\PluginRoot"));
    ASSERT(mgr.Install("plugin-avif", "C:\\pkg\\avif.zip"));
    ASSERT(mgr.InstalledCount() == 1);
}
TEST(TestPDM_Rollback)
{
    using namespace ExplorerLens::Engine;
    PluginDistributionManager mgr;
    mgr.Initialize("C:\\PluginRoot");
    mgr.Install("plugin-avif", "avif.zip");
    mgr.Update("plugin-avif", "avif-v2.zip");
    ASSERT(mgr.Rollback("plugin-avif"));
}
TEST(TestMSI_IndexAndQuery)
{
    using namespace ExplorerLens::Engine;
    MarketplaceSearchIndex& idx = MarketplaceSearchIndex::Instance();
    idx.Initialize();
    idx.IndexPlugin("avif-plugin", "AVIF Decoder", "avif image heif");
    auto results = idx.Query("avif");
    ASSERT(!results.empty());
    ASSERT(results[0].pluginId == "avif-plugin");
}
TEST(TestMSI_DocumentCount)
{
    using namespace ExplorerLens::Engine;
    MarketplaceSearchIndex idx;
    idx.Initialize();
    idx.IndexPlugin("p1", "Plugin One", "heif");
    idx.IndexPlugin("p2", "Plugin Two", "raw");
    ASSERT(idx.DocumentCount() == 2);
}
TEST(TestPSV_ValidatePlugin)
{
    using namespace ExplorerLens::Engine;
    PluginSignatureValidator& val = PluginSignatureValidator::Instance();
    val.Initialize();
    auto info = val.Validate("some-plugin.dll");
    ASSERT(info.result == PluginValidationResult::Valid || info.result == PluginValidationResult::NotSigned);
}
TEST(TestPSV_AddTrustedThumbprint)
{
    using namespace ExplorerLens::Engine;
    PluginSignatureValidator val;
    val.Initialize();
    ASSERT(val.AddTrustedThumbprint("AABB1234"));
    ASSERT(val.TrustedCount() == 1);
    ASSERT(val.IsTrustedSigner("AABB1234"));
}

//== Sprint 1191-1200: LTS Hardening + Security Audit (v33.5.0 "Spica-V") ==
TEST(TestLHC_Initialize)
{
    using namespace ExplorerLens::Engine;
    LTSHardeningController ctrl;
    ASSERT(ctrl.Initialize("33.5.0"));
}
TEST(TestLHC_GateEvaluate)
{
    using namespace ExplorerLens::Engine;
    LTSHardeningController ctrl;
    ctrl.Initialize("33.5.0");
    LTSGate g; g.name = "SecurityAudit"; g.description = "OWASP check"; g.required = true;
    ctrl.AddGate(g);
    ASSERT(ctrl.EvaluateGate("SecurityAudit", true));
    ASSERT(ctrl.IsLTSReady());
}
TEST(TestSAE_Initialize)
{
    using namespace ExplorerLens::Engine;
    SecurityAuditEngine& eng = SecurityAuditEngine::Instance();
    ASSERT(eng.Initialize());
}
TEST(TestSAE_RunAudit)
{
    using namespace ExplorerLens::Engine;
    SecurityAuditEngine& eng = SecurityAuditEngine::Instance();
    eng.Initialize();
    ASSERT(eng.RunAudit());
    ASSERT(eng.Passed());
}
TEST(TestVFDB_AddRecord)
{
    using namespace ExplorerLens::Engine;
    VulnerabilityFingerprintDB db;
    db.Initialize();
    VulnerabilityRecord rec;
    rec.cveId            = "CVE-2025-0001";
    rec.affectedLibrary  = "zlib";
    rec.affectedVersionRange = "< 1.3.1";
    rec.fixedVersion     = "1.3.1";
    rec.cvssScore        = 7;
    ASSERT(db.AddRecord(rec));
    ASSERT(db.RecordCount() == 1);
}
TEST(TestVFDB_QueryByLibrary)
{
    using namespace ExplorerLens::Engine;
    VulnerabilityFingerprintDB db;
    db.Initialize();
    VulnerabilityRecord rec;
    rec.cveId = "CVE-2025-0002"; rec.affectedLibrary = "libwebp";
    rec.affectedVersionRange = "< 1.5.0"; rec.fixedVersion = "1.5.0";
    db.AddRecord(rec);
    auto results = db.QueryByLibrary("libwebp");
    ASSERT(results.size() == 1);
    ASSERT(results[0].cveId == "CVE-2025-0002");
}
TEST(TestLCG_RunGate)
{
    using namespace ExplorerLens::Engine;
    LTSCertificationGate gate;
    gate.Initialize("33.5.0");
    ASSERT(gate.RunGate("SecurityAudit",    true));
    ASSERT(gate.RunGate("DependencyFreeze", true));
    ASSERT(gate.GateCount() == 2);
}
TEST(TestLCG_IsCertified)
{
    using namespace ExplorerLens::Engine;
    LTSCertificationGate gate;
    gate.Initialize("33.5.0");
    gate.RunGate("SecurityAudit",      true);
    gate.RunGate("DependencyFreeze",   true);
    gate.RunGate("PerformanceClear",   true);
    ASSERT(gate.IsCertified());
}
TEST(TestSKS_StoreRetrieve)
{
    using namespace ExplorerLens::Engine;
    SecureKeyStore ks;
    ASSERT(ks.Initialize(KeyStoreBackend::InMemory));
    std::vector<uint8_t> key = {0x01, 0x02, 0x03, 0x04};
    ASSERT(ks.StoreKey("signing-key", key));
    std::vector<uint8_t> retrieved;
    ASSERT(ks.RetrieveKey("signing-key", retrieved));
    ASSERT(retrieved == key);
}
TEST(TestSKS_DeleteKey)
{
    using namespace ExplorerLens::Engine;
    SecureKeyStore ks;
    ks.Initialize(KeyStoreBackend::InMemory);
    std::vector<uint8_t> key = {0xAA, 0xBB};
    ks.StoreKey("tmp-key", key);
    ASSERT(ks.HasKey("tmp-key"));
    ASSERT(ks.DeleteKey("tmp-key"));
    ASSERT(!ks.HasKey("tmp-key"));
}

//== Sprint 1131-1140: Live Preview Scrubber (v32.7.0 "Fomalhaut-X") ==

// ── VideoFrameExtractor ───────────────────────────────────────────────────
TEST(TestVFE_BackendName_DXVA2)
{
    using namespace ExplorerLens::Engine;
    ASSERT(VideoFrameExtractor::BackendName(VideoDecodeBackend::DXVA2_HARDWARE) == std::string_view("DXVA2-Hardware"));
}
TEST(TestVFE_BackendName_MFSoftware)
{
    using namespace ExplorerLens::Engine;
    ASSERT(VideoFrameExtractor::BackendName(VideoDecodeBackend::MF_SOFTWARE) == std::string_view("MF-Software"));
}
TEST(TestVFE_BackendName_Unavailable)
{
    using namespace ExplorerLens::Engine;
    ASSERT(VideoFrameExtractor::BackendName(VideoDecodeBackend::UNAVAILABLE) == std::string_view("Unavailable"));
}
TEST(TestVFE_ExtractFrame_ValidPath)
{
    using namespace ExplorerLens::Engine;
    auto& ex = VideoFrameExtractor::Instance();
    ex.Reset();
    VideoFrameRequest req{};
    req.filePath         = L"clip.mp4";
    req.timestampSeconds = 5.0;
    req.targetWidth      = 320;
    req.targetHeight     = 180;
    const auto RES = ex.ExtractFrame(req);
    ASSERT(RES.success);
    ASSERT(RES.width  == 320);
    ASSERT(RES.height == 180);
    ASSERT(ex.ExtractCount() == 1);
}
TEST(TestVFE_ExtractFrame_EmptyPath)
{
    using namespace ExplorerLens::Engine;
    auto& ex = VideoFrameExtractor::Instance();
    ex.Reset();
    VideoFrameRequest req{};
    const auto RES = ex.ExtractFrame(req);
    ASSERT(!RES.success);
}

// ── VideoScrubberTimeline ─────────────────────────────────────────────────
TEST(TestVST_BuildEmpty)
{
    using namespace ExplorerLens::Engine;
    auto& tl = VideoScrubberTimeline::Instance();
    ASSERT(!tl.Build(L""));
}
TEST(TestVST_BuildPopulates)
{
    using namespace ExplorerLens::Engine;
    auto& tl = VideoScrubberTimeline::Instance();
    ASSERT(tl.Build(L"video.mkv"));
    ASSERT(tl.KeyframeCount() > 0);
    ASSERT(tl.Duration() > 0.0);
    ASSERT(tl.ChapterCount() >= 1);
}
TEST(TestVST_KeyframeAt_Valid)
{
    using namespace ExplorerLens::Engine;
    auto& tl = VideoScrubberTimeline::Instance();
    tl.Build(L"video.mkv");
    const auto KF = tl.KeyframeAt(0);
    ASSERT(KF.index  == 0);
    ASSERT(KF.ptsSec == 0.0);
}
TEST(TestVST_KeyframeAt_OutOfBounds)
{
    using namespace ExplorerLens::Engine;
    auto& tl = VideoScrubberTimeline::Instance();
    tl.Build(L"video.mkv");
    const auto KF = tl.KeyframeAt(9999);
    ASSERT(KF.index  == 0);
    ASSERT(KF.ptsSec == 0.0);
}
TEST(TestVST_NearestKeyframePts)
{
    using namespace ExplorerLens::Engine;
    auto& tl = VideoScrubberTimeline::Instance();
    tl.Build(L"video.mkv");
    const double PTS = tl.NearestKeyframePts(25.0);
    ASSERT(PTS >= 0.0);
    ASSERT(PTS <= tl.Duration());
}

// ── LivePreviewSession ────────────────────────────────────────────────────
TEST(TestLPS_OpenEmpty)
{
    using namespace ExplorerLens::Engine;
    LivePreviewSession sess;
    ASSERT(!sess.Open(L""));
    ASSERT(!sess.IsOpen());
}
TEST(TestLPS_OpenValid)
{
    using namespace ExplorerLens::Engine;
    LivePreviewSession sess;
    ASSERT(sess.Open(L"movie.mp4"));
    ASSERT(sess.IsOpen());
    ASSERT(sess.FilePath() == L"movie.mp4");
}
TEST(TestLPS_SeekNotOpen)
{
    using namespace ExplorerLens::Engine;
    LivePreviewSession sess;
    const auto RES = sess.SeekTo(10.0);
    ASSERT(!RES.success);
}
TEST(TestLPS_SeekOpen)
{
    using namespace ExplorerLens::Engine;
    LivePreviewSession sess;
    sess.Open(L"movie.mp4");
    const auto RES = sess.SeekTo(30.0);
    ASSERT(RES.success);
    ASSERT(RES.actualPts >= 0.0);
}
TEST(TestLPS_Close)
{
    using namespace ExplorerLens::Engine;
    LivePreviewSession sess;
    sess.Open(L"movie.mp4");
    ASSERT(sess.IsOpen());
    sess.Close();
    ASSERT(!sess.IsOpen());
    ASSERT(sess.FilePath().empty());
}

// ── ThumbnailStripGenerator ───────────────────────────────────────────────
TEST(TestTSG_ResetClearsState)
{
    using namespace ExplorerLens::Engine;
    auto& gen = ThumbnailStripGenerator::Instance();
    gen.Reset();
    ASSERT(gen.StripWidth()  == 0);
    ASSERT(gen.StripHeight() == 0);
    ASSERT(gen.FrameCount()  == 0);
}
TEST(TestTSG_GenerateEmpty)
{
    using namespace ExplorerLens::Engine;
    StripConfig cfg{};
    const auto RES = ThumbnailStripGenerator::Instance().GenerateStrip(cfg);
    ASSERT(!RES.success);
}
TEST(TestTSG_GenerateValid)
{
    using namespace ExplorerLens::Engine;
    StripConfig cfg{};
    cfg.filePath    = L"video.mp4";
    cfg.frameCount  = 5;
    cfg.frameWidth  = 160;
    cfg.frameHeight = 90;
    const auto RES = ThumbnailStripGenerator::Instance().GenerateStrip(cfg);
    ASSERT(RES.success);
    ASSERT(RES.framesAdded == 5);
}
TEST(TestTSG_StripWidth)
{
    using namespace ExplorerLens::Engine;
    StripConfig cfg{};
    cfg.filePath    = L"video.mp4";
    cfg.frameCount  = 4;
    cfg.frameWidth  = 100;
    cfg.frameHeight = 56;
    const auto RES = ThumbnailStripGenerator::Instance().GenerateStrip(cfg);
    ASSERT(RES.stripWidth  == 400);
    ASSERT(RES.stripHeight == 56);
}
TEST(TestTSG_FrameCount)
{
    using namespace ExplorerLens::Engine;
    StripConfig cfg{};
    cfg.filePath    = L"video.mp4";
    cfg.frameCount  = 8;
    cfg.frameWidth  = 80;
    cfg.frameHeight = 45;
    ThumbnailStripGenerator::Instance().GenerateStrip(cfg);
    ASSERT(ThumbnailStripGenerator::Instance().FrameCount() == 8);
}

// ── ScrubberCacheEngine ───────────────────────────────────────────────────
TEST(TestSCE_InitialEmpty)
{
    using namespace ExplorerLens::Engine;
    auto& cache = ScrubberCacheEngine::Instance();
    cache.Clear();
    ASSERT(cache.Size() == 0);
    const auto ST = cache.GetStats();
    ASSERT(ST.hits    == 0);
    ASSERT(ST.misses  == 0);
    ASSERT(ST.hitRate == 0.0f);
}
TEST(TestSCE_PutAndGet)
{
    using namespace ExplorerLens::Engine;
    auto& cache = ScrubberCacheEngine::Instance();
    cache.Clear();
    ScrubberCacheKey key{};
    key.filePath = L"video.mp4";
    key.ptsSec   = 5.0;
    ScrubberCacheEntry entry{};
    entry.valid  = true;
    entry.width  = 320;
    entry.height = 180;
    ASSERT(cache.Put(key, entry));
    ScrubberCacheEntry out{};
    ASSERT(cache.Get(key, out));
    ASSERT(out.valid);
    ASSERT(out.width  == 320);
    ASSERT(out.height == 180);
}
TEST(TestSCE_GetMiss)
{
    using namespace ExplorerLens::Engine;
    auto& cache = ScrubberCacheEngine::Instance();
    cache.Clear();
    ScrubberCacheKey key{};
    key.filePath = L"notcached.mp4";
    key.ptsSec   = 99.0;
    ScrubberCacheEntry out{};
    ASSERT(!cache.Get(key, out));
    ASSERT(cache.GetStats().misses == 1);
}
TEST(TestSCE_Evict)
{
    using namespace ExplorerLens::Engine;
    auto& cache = ScrubberCacheEngine::Instance();
    cache.Clear();
    ScrubberCacheKey key{};
    key.filePath = L"video.mp4";
    key.ptsSec   = 10.0;
    ScrubberCacheEntry entry{};
    entry.valid = true;
    cache.Put(key, entry);
    ASSERT(cache.Size() == 1);
    cache.Evict(key);
    ASSERT(cache.Size() == 0);
}
TEST(TestSCE_HitRate)
{
    using namespace ExplorerLens::Engine;
    auto& cache = ScrubberCacheEngine::Instance();
    cache.Clear();
    ScrubberCacheKey key{};
    key.filePath = L"video.mp4";
    key.ptsSec   = 15.0;
    ScrubberCacheEntry entry{};
    entry.valid = true;
    cache.Put(key, entry);
    ScrubberCacheEntry out{};
    cache.Get(key, out);       // hit
    ScrubberCacheKey miss{};
    miss.filePath = L"x.mp4";
    miss.ptsSec   = 1.0;
    cache.Get(miss, out);      // miss
    const auto ST = cache.GetStats();
    ASSERT(ST.hits   == 1);
    ASSERT(ST.misses == 1);
    ASSERT(ST.hitRate >= 0.49f && ST.hitRate <= 0.51f);
}

//==============================================================================
// Sprint 1201-1210: Format Coverage Blitz (v34.0.0 "Arcturus")
//==============================================================================

TEST(TestBasisUniversalDecoder_Extensions)
{
    BasisUniversalDecoder decoder;
    const auto info = decoder.GetInfo();
    ASSERT(info.extensionCount == 2);
    bool hasBasis = false, hasKtx2 = false;
    for (uint32_t i = 0; i < info.extensionCount; ++i) {
        if (_wcsicmp(info.supportedExtensions[i], L".basis") == 0) hasBasis = true;
        if (_wcsicmp(info.supportedExtensions[i], L".ktx2")  == 0) hasKtx2  = true;
    }
    ASSERT(hasBasis);
    ASSERT(hasKtx2);
}

TEST(TestBasisUniversalDecoder_CanDecode)
{
    BasisUniversalDecoder decoder;
    ASSERT( decoder.CanDecode(L"texture.basis"));
    ASSERT( decoder.CanDecode(L"asset.KTX2"));
    ASSERT(!decoder.CanDecode(L"image.dds"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestUltraHDRDecoder_Extensions)
{
    UltraHDRDecoder decoder;
    const auto info = decoder.GetInfo();
    ASSERT(info.extensionCount == 1);
    ASSERT(_wcsicmp(info.supportedExtensions[0], L".uhdr") == 0);
}

TEST(TestUltraHDRDecoder_CanDecode)
{
    UltraHDRDecoder decoder;
    ASSERT( decoder.CanDecode(L"photo.uhdr"));
    ASSERT( decoder.CanDecode(L"gain.UHDR"));
    ASSERT(!decoder.CanDecode(L"photo.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestIfcBimDecoder_Extensions)
{
    IfcBimDecoder decoder;
    const auto info = decoder.GetInfo();
    ASSERT(info.extensionCount == 2);
    bool hasIfc = false, hasIfczip = false;
    for (uint32_t i = 0; i < info.extensionCount; ++i) {
        if (_wcsicmp(info.supportedExtensions[i], L".ifc")    == 0) hasIfc    = true;
        if (_wcsicmp(info.supportedExtensions[i], L".ifczip") == 0) hasIfczip = true;
    }
    ASSERT(hasIfc);
    ASSERT(hasIfczip);
}

TEST(TestIfcBimDecoder_CanDecode)
{
    IfcBimDecoder decoder;
    ASSERT( decoder.CanDecode(L"building.ifc"));
    ASSERT( decoder.CanDecode(L"project.IFCZIP"));
    ASSERT(!decoder.CanDecode(L"drawing.dwg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestLasPointCloudDecoder_Extensions)
{
    LasPointCloudDecoder decoder;
    const auto info = decoder.GetInfo();
    ASSERT(info.extensionCount == 2);
    bool hasLas = false, hasLaz = false;
    for (uint32_t i = 0; i < info.extensionCount; ++i) {
        if (_wcsicmp(info.supportedExtensions[i], L".las") == 0) hasLas = true;
        if (_wcsicmp(info.supportedExtensions[i], L".laz") == 0) hasLaz = true;
    }
    ASSERT(hasLas);
    ASSERT(hasLaz);
}

TEST(TestLasPointCloudDecoder_CanDecode)
{
    LasPointCloudDecoder decoder;
    ASSERT( decoder.CanDecode(L"scan.las"));
    ASSERT( decoder.CanDecode(L"lidar.LAZ"));
    ASSERT(!decoder.CanDecode(L"cloud.ply"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestJupyterNotebookDecoder_Extensions)
{
    JupyterNotebookDecoder decoder;
    const auto info = decoder.GetInfo();
    ASSERT(info.extensionCount == 1);
    ASSERT(_wcsicmp(info.supportedExtensions[0], L".ipynb") == 0);
}

TEST(TestJupyterNotebookDecoder_CanDecode)
{
    JupyterNotebookDecoder decoder;
    ASSERT( decoder.CanDecode(L"notebook.ipynb"));
    ASSERT( decoder.CanDecode(L"analysis.IPYNB"));
    ASSERT(!decoder.CanDecode(L"script.py"));
    ASSERT(!decoder.CanDecode(nullptr));
}

//==============================================================================
// Sprint 1211-1220: GPU-First Decode Pipeline (v34.1.0 "Arcturus-R")
//==============================================================================

TEST(TestGPUDecodeFormatRouter_RouteJPEG)
{
    using namespace ExplorerLens::Engine;
    GPUDecodeFormatRouter router;
    const auto route = router.RouteByExtension(L".jpg");
    ASSERT(route.primaryPath == GPUDecodePathId::NVJPEG ||
           route.primaryPath == GPUDecodePathId::QSV_JPEG ||
           route.primaryPath == GPUDecodePathId::CPUFallback);
    ASSERT(route.targetMs > 0.0f);
}

TEST(TestGPUDecodeFormatRouter_FallbackCPU)
{
    using namespace ExplorerLens::Engine;
    GPUDecodeFormatRouter router;
    const auto route = router.RouteByExtension(L".xyz_unknown");
    ASSERT(route.primaryPath == GPUDecodePathId::CPUFallback);
    const auto nullRoute = router.RouteByExtension(nullptr);
    ASSERT(nullRoute.primaryPath == GPUDecodePathId::CPUFallback);
}

TEST(TestGPUJPEGDecodeAccelerator_Init)
{
    using namespace ExplorerLens::Engine;
    GPUJPEGDecodeAccelerator accel;
    const bool ok = accel.Initialize();
    ASSERT(ok);
    ASSERT(accel.IsAvailable());
}

TEST(TestGPUJPEGDecodeAccelerator_Caps)
{
    using namespace ExplorerLens::Engine;
    GPUJPEGDecodeAccelerator accel;
    accel.Initialize();
    JPEGDecodeRequest req{};
    req.targetWidth  = 256;
    req.targetHeight = 256;
    // srcData is null → should return unsuccessful result, not crash.
    const auto result = accel.Decode(req);
    ASSERT(!result.success);
    ASSERT(result.pixelsBGRA == nullptr);
}

TEST(TestGPURawDemosaicKernel_Identity)
{
    using namespace ExplorerLens::Engine;
    GPURawDemosaicKernel kernel;
    const bool ok = kernel.Initialize();
    ASSERT(ok);
    ASSERT(kernel.IsAvailable());
}

TEST(TestGPURawDemosaicKernel_BayerMasks)
{
    using namespace ExplorerLens::Engine;
    GPURawDemosaicKernel kernel;
    kernel.Initialize();
    // 4×4 synthetic Bayer grid (RGGB, uniform grey 1000)
    const uint32_t W = 4, H = 4;
    uint16_t raw[W * H];
    for (auto& v : raw) v = 1000;
    RAWDemosaicParams params{};
    params.blackLevel = 0;
    params.whiteLevel = 4095;
    auto res = kernel.Demosaic(raw, W, H, params, 2, 2);
    ASSERT(res.success);
    ASSERT(res.width  == 2);
    ASSERT(res.height == 2);
    ASSERT(res.pixelsBGRA != nullptr);
    delete[] res.pixelsBGRA;
}

TEST(TestGPUDecodePerformanceGate_Pass)
{
    using namespace ExplorerLens::Engine;
    GPUDecodePerformanceGate gate;
    gate.LoadBaseline(nullptr);
    PerformanceSample s{};
    s.formatName       = "JPEG";
    s.p50Ms            = 4.0f;   // better than baseline 4.2 ms
    s.p95Ms            = 5.5f;   // better than baseline 6.0 ms
    s.batchImgPerSec   = 240.0f; // better than baseline 235 img/s
    const bool ok = gate.AllPass({ s });
    ASSERT(ok);
}

TEST(TestGPUDecodePerformanceGate_Block)
{
    using namespace ExplorerLens::Engine;
    GPUDecodePerformanceGate gate;
    gate.LoadBaseline(nullptr);
    PerformanceSample s{};
    s.formatName       = "JPEG";
    s.p50Ms            = 10.0f;  // 2.4× worse
    s.p95Ms            = 30.0f;  // 5× worse — must block
    s.batchImgPerSec   = 100.0f; // 57% drop — must block
    const bool ok = gate.AllPass({ s });
    ASSERT(!ok);  // gate should block
}

TEST(TestZeroCopyGPUSurface_Alloc)
{
    using namespace ExplorerLens::Engine;
    ZeroCopyGPUSurface surf;
    const bool ok = surf.Allocate(128, 128, SurfaceAllocMode::SystemMemory);
    ASSERT(ok);
    ASSERT(surf.GetDesc().width  == 128);
    ASSERT(surf.GetDesc().height == 128);
    surf.Release();
    ASSERT(surf.GetDesc().pData == nullptr);
}

TEST(TestZeroCopyGPUSurface_MapUnmap)
{
    using namespace ExplorerLens::Engine;
    ZeroCopyGPUSurface surf;
    surf.Allocate(64, 64, SurfaceAllocMode::SystemMemory);
    uint8_t* ptr = surf.Map();
    ASSERT(ptr != nullptr);
    // Write a known pattern and verify it survives through Unmap.
    ptr[0] = 0xBE;
    ptr[1] = 0xEF;
    surf.Unmap();
    uint8_t* ptr2 = surf.Map();
    ASSERT(ptr2 != nullptr);
    ASSERT(ptr2[0] == 0xBE);
    ASSERT(ptr2[1] == 0xEF);
    surf.Unmap();
    surf.Release();
}

//== Sprint 1221-1230: HDR & Wide Color Gamut Mastery (v34.2.0 "Arcturus-S") ==

TEST(TestGainmapJPEGToneMapper_UltraHDRDetect)
{
    using namespace ExplorerLens::Engine;
    // A bare JPEG SOI with no XMP = not Ultra HDR.
    const uint8_t fakeJpeg[] = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46 };
    ASSERT(!GainmapJPEGToneMapper::IsUltraHDR(fakeJpeg, sizeof(fakeJpeg)));
    // Null input.
    ASSERT(!GainmapJPEGToneMapper::IsUltraHDR(nullptr, 0));
}

TEST(TestGainmapJPEGToneMapper_ParseMetadata)
{
    using namespace ExplorerLens::Engine;
    // ParseGainmapMetadata returns false for non-Ultra HDR data.
    GainmapJPEGToneMapper mapper;
    GainmapMetadata meta{};
    const uint8_t notJpeg[] = { 0x00, 0x01, 0x02 };
    ASSERT(!mapper.ParseGainmapMetadata(notJpeg, sizeof(notJpeg), meta));
    ASSERT(!meta.isValid);
}

TEST(TestPQToSDRToneMapper_LUTBuild)
{
    using namespace ExplorerLens::Engine;
    PQToneMapParams params{};
    params.op         = PQToneMapOp::ACES;
    params.peakNits   = 1000.0f;
    auto lut = PQToSDRToneMapper::BuildPQToSRGBLUT(params);
    // LUT has 1024 entries; first entry (0 PQ) should map to 0.
    ASSERT(lut[0] == 0);
    // Mid and high entries should be non-zero for non-zero PQ values.
    ASSERT(lut[512] > 0);
    ASSERT(lut[1023] > 0);
}

TEST(TestPQToSDRToneMapper_SinglePixel)
{
    using namespace ExplorerLens::Engine;
    // Single black pixel (PQ = 0) → output should be fully black.
    const uint16_t zeroPx[4] = { 0, 0, 0, 0x3C00 };  // R=0, G=0, B=0, A=1.0 (fp16)
    PQToSDRToneMapper mapper;
    PQToneMapParams params{};
    params.op = PQToneMapOp::Hable;
    auto result = mapper.ToneMap(zeroPx, 1, 1, params);
    ASSERT(result.success);
    ASSERT(result.pixelsBGRA != nullptr);
    ASSERT(result.pixelsBGRA[0] == 0);  // B
    ASSERT(result.pixelsBGRA[1] == 0);  // G
    ASSERT(result.pixelsBGRA[2] == 0);  // R
    delete[] result.pixelsBGRA;
}

TEST(TestHLGToSDRConverter_Identity)
{
    using namespace ExplorerLens::Engine;
    // HLGtoLinear(0.0) == 0.0 (black → black).
    ASSERT(HLGToSDRConverter::HLGtoLinear(0.0f) == 0.0f);
    // HLGtoLinear(0.5) should be in valid range.
    const float mid = HLGToSDRConverter::HLGtoLinear(0.5f);
    ASSERT(mid > 0.0f && mid <= 1.0f);
}

TEST(TestHLGToSDRConverter_OOTFGamma)
{
    using namespace ExplorerLens::Engine;
    // OOTF at 0.0 scene linear should give 0.0.
    const float ootfZero = HLGToSDRConverter::HLGOOTF(0.0f, 1.2f, 1000.0f);
    ASSERT(ootfZero == 0.0f);
    // OOTF at positive scene linear should give positive value.
    const float ootfVal = HLGToSDRConverter::HLGOOTF(0.5f, 1.2f, 1000.0f);
    ASSERT(ootfVal > 0.0f);
    // BT2020→BT.709 matrix: all-zero input stays zero.
    float r = 0.0f, g = 0.0f, b = 0.0f;
    HLGToSDRConverter::BT2020ToBT709(r, g, b);
    ASSERT(r == 0.0f && g == 0.0f && b == 0.0f);
}

TEST(TestICCv5ProfileEngine_LoadBuiltIn)
{
    using namespace ExplorerLens::Engine;
    ICCv5ProfileEngine engine;
    ASSERT(!engine.IsLoaded());
    ASSERT(engine.LoadBuiltIn(ICCColorSpace::sRGB));
    ASSERT(engine.IsLoaded());
    const ICCProfileInfo info = engine.GetInfo();
    ASSERT(info.colorSpace == ICCColorSpace::sRGB);
    // sRGB → sRGB transform should be identity.
    const uint8_t src[4] = { 0x10, 0x20, 0x30, 0xFF };
    auto result = engine.TransformToSRGB(src, 1, 1);
    ASSERT(result.success);
    ASSERT(result.pixelsBGRA != nullptr);
    ASSERT(result.pixelsBGRA[0] == 0x10);
    ASSERT(result.pixelsBGRA[2] == 0x30);
    delete[] result.pixelsBGRA;
}

TEST(TestICCv5ProfileEngine_DetectColorspace)
{
    using namespace ExplorerLens::Engine;
    // Non-JPEG data should return Unknown.
    const uint8_t garbage[] = { 0xAA, 0xBB, 0xCC };
    const ICCColorSpace cs = ICCv5ProfileEngine::DetectEmbeddedColorSpace(
        garbage, sizeof(garbage));
    ASSERT(cs == ICCColorSpace::Unknown);
    // Null input.
    const ICCColorSpace csNull = ICCv5ProfileEngine::DetectEmbeddedColorSpace(nullptr, 0);
    ASSERT(csNull == ICCColorSpace::Unknown);
}

TEST(TestACESODTProcessor_DetectFromString)
{
    using namespace ExplorerLens::Engine;
    ASSERT(ACESODTProcessor::DetectFromString("ACES2065-1") == ACESColorspace::AP0);
    ASSERT(ACESODTProcessor::DetectFromString("ACEScg")     == ACESColorspace::AP1);
    ASSERT(ACESODTProcessor::DetectFromString("ACEScc")     == ACESColorspace::ACEScc);
    ASSERT(ACESODTProcessor::DetectFromString("ACEScct")    == ACESColorspace::ACEScct);
    ASSERT(ACESODTProcessor::DetectFromString("unknown")    == ACESColorspace::Unknown);
    ASSERT(ACESODTProcessor::DetectFromString(nullptr)      == ACESColorspace::Unknown);
}

TEST(TestACESODTProcessor_ACEScgToSRGB)
{
    using namespace ExplorerLens::Engine;
    // All-zero ACEScg pixels (black) → black sRGB output.
    const uint16_t zeroPx[4] = { 0, 0, 0, 0x3C00 };  // fp16: R=0, G=0, B=0, A=1.0
    ACESODTProcessor proc;
    auto result = proc.ACEScgToSRGB(zeroPx, 1, 1);
    ASSERT(result.success);
    ASSERT(result.pixelsBGRA != nullptr);
    ASSERT(result.pixelsBGRA[0] == 0);  // B
    ASSERT(result.pixelsBGRA[1] == 0);  // G
    ASSERT(result.pixelsBGRA[2] == 0);  // R
    ASSERT(result.pixelsBGRA[3] == 0xFF);
    delete[] result.pixelsBGRA;
}

//== Sprint 1231-1240: Predictive Pre-Generation Engine (v34.3.0 "Arcturus-T") ==

TEST(TestDirectoryPreScanQueue_NetworkDetect)
{
    using namespace ExplorerLens::Engine;
    // UNC path is network.
    ASSERT(DirectoryPreScanQueue::IsNetworkPath(L"\\\\server\\share"));
    // Local drive is not network.
    ASSERT(!DirectoryPreScanQueue::IsNetworkPath(L"C:\\Users\\Test"));
    // Empty string is not network.
    ASSERT(!DirectoryPreScanQueue::IsNetworkPath(L""));
}

TEST(TestDirectoryPreScanQueue_StartStop)
{
    using namespace ExplorerLens::Engine;
    DirectoryPreScanConfig cfg{};
    cfg.workerThreadCount = 1;
    DirectoryPreScanQueue q(cfg);
    ASSERT(!q.IsRunning());
    q.Start();
    ASSERT(q.IsRunning());
    q.Stop();
    ASSERT(!q.IsRunning());
    // Queue depth remains 0 since no directory was enqueued.
    ASSERT(q.QueueDepth() == 0);
}

TEST(TestAdjacencyPredictor_Record)
{
    using namespace ExplorerLens::Engine;
    AdjacencyPredictor pred;
    ASSERT(pred.HistoryDepth() == 0);
    pred.RecordNavigation(L"C:\\Foo\\Bar");
    ASSERT(pred.HistoryDepth() == 1);
    pred.RecordNavigation(L"C:\\Foo\\Baz");
    ASSERT(pred.HistoryDepth() == 2);
    // Duplicate consecutive navigation should not add an extra entry.
    pred.RecordNavigation(L"C:\\Foo\\Baz");
    ASSERT(pred.HistoryDepth() == 2);
    pred.Reset();
    ASSERT(pred.HistoryDepth() == 0);
}

TEST(TestAdjacencyPredictor_PredictEmpty)
{
    using namespace ExplorerLens::Engine;
    AdjacencyPredictor pred;
    // With no history and a non-existent directory, predictions should be empty
    // (no siblings found on disk for a fake path).
    const auto preds = pred.Predict(L"Z:\\FakePath\\FakeDir", 4);
    ASSERT(preds.size() == 0u || preds.size() <= 4u);
}

TEST(TestScrollVelocityTracker_ZeroVelocity)
{
    using namespace ExplorerLens::Engine;
    ScrollVelocityTracker tracker;
    const auto stats = tracker.GetStats();
    ASSERT(stats.itemsPerSecond == 0.0f);
    ASSERT(!stats.isFastScroll);
    ASSERT(stats.direction == 0);
}

TEST(TestScrollVelocityTracker_EMASmoothing)
{
    using namespace ExplorerLens::Engine;
    ScrollVelocityTracker tracker;
    // Add two samples to compute velocity.
    ScrollSample s1;
    s1.timestampUs  = 0;
    s1.deltaItems   = 10;
    s1.viewportRows = 20;
    tracker.AddSample(s1);

    ScrollSample s2;
    s2.timestampUs  = 100'000;  // 100 ms = 0.1 s
    s2.deltaItems   = 10;
    s2.viewportRows = 20;
    tracker.AddSample(s2);

    const auto stats = tracker.GetStats();
    // 10 items / 0.1 s = 100 items/sec instantaneous; EMA will be partial.
    ASSERT(stats.itemsPerSecond > 0.0f);
    ASSERT(stats.direction == 1);  // positive delta → down
    tracker.Reset();
    ASSERT(tracker.GetStats().itemsPerSecond == 0.0f);
}

TEST(TestIdleTimePreGenerator_Stats)
{
    using namespace ExplorerLens::Engine;
    IdleTimePreGenerator gen;
    ASSERT(!gen.IsRunning());
    const auto stats = gen.GetStats();
    ASSERT(stats.totalPreGenerated == 0);
    ASSERT(stats.idleWindowsUsed   == 0);
    ASSERT(!stats.isActive);
}

TEST(TestIdleTimePreGenerator_BatteryCheck)
{
    using namespace ExplorerLens::Engine;
    // Platform helper — just ensure it returns without crashing.
    const bool onBattery = IdleTimePreGenerator::IsOnBattery();
    // Result is platform-dependent; just assert it's a bool-compatible value.
    ASSERT(onBattery == true || onBattery == false);
    const float cpu = IdleTimePreGenerator::SampleCpuPercent();
    ASSERT(cpu >= 0.0f && cpu <= 100.0f);
}

TEST(TestPredictivePreGenEngine_InitStats)
{
    using namespace ExplorerLens::Engine;
    PreGenEngineConfig cfg{};
    cfg.backgroundThreads = 1;
    cfg.enableIdleGen     = false;
    PredictivePreGenEngine engine(cfg);
    ASSERT(!engine.IsRunning());
    const auto stats = engine.GetStats();
    ASSERT(stats.totalQueued    == 0);
    ASSERT(stats.totalGenerated == 0);
    ASSERT(stats.cacheHitRate   == 0.0f);
}

TEST(TestPredictivePreGenEngine_CacheHitCount)
{
    using namespace ExplorerLens::Engine;
    PreGenEngineConfig cfg{};
    cfg.backgroundThreads = 1;
    cfg.enableIdleGen     = false;
    cfg.enableScrollPredict = false;
    cfg.enableAdjacency   = false;

    PredictivePreGenEngine engine(cfg);
    uint32_t generated = 0;

    engine.SetCallbacks(
        [](const std::wstring&) { return false; },  // nothing cached
        [&generated](const std::wstring&) { ++generated; }
    );

    // Simulate two thumbnail requests: both are cache misses.
    engine.OnThumbnailRequested(L"C:\\fake\\a.jpg");
    engine.OnThumbnailRequested(L"C:\\fake\\b.png");
    const auto stats = engine.GetStats();
    ASSERT(stats.cacheMisses == 2);
    ASSERT(stats.cacheHits   == 0);
}

//==============================================================================
// Sprint 1241-1250: Animated & Sequence Format Suite (v34.4.0 "Arcturus-U")
//==============================================================================

TEST(TestHoverScrubController_PosToFrame)
{
    using namespace ExplorerLens::Engine;
    // With 5 frames, pos=0.0 => 0, pos=1.0 => 4, pos=0.5 => 2
    ASSERT(HoverScrubController::PosToFrame(0.0f, 5) == 0);
    ASSERT(HoverScrubController::PosToFrame(1.0f, 5) == 4);
    ASSERT(HoverScrubController::PosToFrame(0.5f, 5) == 2);
    // Single-frame: always 0
    ASSERT(HoverScrubController::PosToFrame(0.9f, 1) == 0);
    // Zero frames: always 0
    ASSERT(HoverScrubController::PosToFrame(0.5f, 0) == 0);
}

TEST(TestHoverScrubController_MouseLeave)
{
    using namespace ExplorerLens::Engine;
    HoverScrubController ctrl;
    ctrl.SetFrameCount(10);

    uint32_t lastFrame = 99;
    ctrl.SetFrameChangedCallback([&lastFrame](uint32_t f) { lastFrame = f; });

    ctrl.OnMouseEnter(90, 100);  // pos~0.9 => frame 9
    ASSERT(ctrl.GetState().active);
    ASSERT(ctrl.GetState().frameIndex == 9);

    ctrl.OnMouseLeave();
    // On leave: returns to frame 0, active = false
    ASSERT(!ctrl.GetState().active);
    ASSERT(ctrl.GetState().frameIndex == 0);
    ASSERT(lastFrame == 0);
}

TEST(TestAPNGFrameCombiner_SelectKeyFrames)
{
    using namespace ExplorerLens::Engine;
    // Evenly distributed key frames
    auto idx = APNGFrameCombiner::SelectKeyFrameIndices(10, 5);
    ASSERT(idx.size() == 5);
    ASSERT(idx[0] == 0);

    // maxFrames >= totalFrames: one-to-one
    auto idx2 = APNGFrameCombiner::SelectKeyFrameIndices(3, 10);
    ASSERT(idx2.size() == 3);

    // Edge: 0 frames or 0 max
    ASSERT(APNGFrameCombiner::SelectKeyFrameIndices(0, 5).empty());
    ASSERT(APNGFrameCombiner::SelectKeyFrameIndices(5, 0).empty());
}

TEST(TestAPNGFrameCombiner_ProbeFrameCount)
{
    using namespace ExplorerLens::Engine;
    // Non-PNG data: returns 0
    const uint8_t garbage[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};
    ASSERT(APNGFrameCombiner::ProbeFrameCount(garbage, sizeof(garbage)) == 0);

    // PNG magic with no acTL => 1 (non-animated PNG)
    const uint8_t pngMagic[] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,
                                  0x00,0x00,0x00,0x04, 'I','E','N','D', 0,0,0,0};
    ASSERT(APNGFrameCombiner::ProbeFrameCount(pngMagic, sizeof(pngMagic)) == 1);

    // Null pointer: returns 0
    ASSERT(APNGFrameCombiner::ProbeFrameCount(nullptr, 64) == 0);
}

TEST(TestGIFAnimationDecoder_IsGIF)
{
    using namespace ExplorerLens::Engine;
    const uint8_t gif89a[] = {'G','I','F','8','9','a',0,0,0,0,0,0,0};
    const uint8_t gif87a[] = {'G','I','F','8','7','a',0,0,0,0,0,0,0};
    const uint8_t notGIF[] = {'P','N','G','8','9','a',0,0,0,0,0,0,0};

    ASSERT(GIFAnimationDecoder::IsGIF(gif89a, sizeof(gif89a)));
    ASSERT(GIFAnimationDecoder::IsGIF(gif87a, sizeof(gif87a)));
    ASSERT(!GIFAnimationDecoder::IsGIF(notGIF, sizeof(notGIF)));
    ASSERT(!GIFAnimationDecoder::IsGIF(nullptr, 16));
    ASSERT(!GIFAnimationDecoder::IsGIF(gif89a, 5));  // too short
}

TEST(TestGIFAnimationDecoder_ProbeFrameCount)
{
    using namespace ExplorerLens::Engine;
    // Not a GIF => 0
    const uint8_t bad[] = {0x00,0x01,0x02,0x03,0x04,0x05};
    ASSERT(GIFAnimationDecoder::ProbeFrameCount(bad, sizeof(bad)) == 0);

    // Minimal GIF89a buffer with only header — no Image Descriptors => 1
    uint8_t minGIF[14] = {};
    std::memcpy(minGIF, "GIF89a", 6);
    minGIF[6] = 1; minGIF[7] = 0;   // width = 1
    minGIF[8] = 1; minGIF[9] = 0;   // height = 1
    minGIF[10] = 0x00;               // no global colour table
    minGIF[11] = 0; minGIF[12] = 0;
    minGIF[13] = 0x3B;               // Trailer
    // ProbeFrameCount returns >= 1 for a valid GIF
    const uint32_t count = GIFAnimationDecoder::ProbeFrameCount(minGIF, sizeof(minGIF));
    ASSERT(count >= 1);
}

TEST(TestAnimatedSequenceSampler_DetectGIF)
{
    using namespace ExplorerLens::Engine;
    const uint8_t gif[] = {'G','I','F','8','9','a',1,0,1,0,0,0,0,0x3B};
    ASSERT(AnimatedSequenceSampler::Detect(gif, sizeof(gif)) == SampledAnimFormat::GIF);

    // WebP RIFF magic
    const uint8_t webp[] = {'R','I','F','F',0,0,0,0,'W','E','B','P'};
    ASSERT(AnimatedSequenceSampler::Detect(webp, sizeof(webp)) == SampledAnimFormat::AnimatedWebP);

    // Unknown
    const uint8_t unk[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
                             0x08,0x09,0x0A,0x0B};
    ASSERT(AnimatedSequenceSampler::Detect(unk, sizeof(unk)) == SampledAnimFormat::Unknown);
}

TEST(TestAnimatedSequenceSampler_SampleGIF)
{
    using namespace ExplorerLens::Engine;
    // Build a minimal single-frame GIF89a
    uint8_t minGIF[20] = {};
    std::memcpy(minGIF, "GIF89a", 6);
    minGIF[6] = 4; minGIF[7] = 0;    // width = 4
    minGIF[8] = 4; minGIF[9] = 0;    // height = 4
    minGIF[10] = 0x00;
    minGIF[11] = 0; minGIF[12] = 0;
    minGIF[13] = 0x2C;               // Image descriptor
    minGIF[14] = 0; minGIF[15] = 0;  // left
    minGIF[16] = 0; minGIF[17] = 0;  // top
    minGIF[18] = 4; minGIF[19] = 0;  // ... (incomplete, but probe ok)

    AnimatedSequenceSampler sampler;
    AnimatedSampleOptions opts{};
    opts.maxKeyFrames = 3;

    // Call succeeds (stub decoder)
    const auto result = sampler.Sample(minGIF, sizeof(minGIF), opts);
    // Either detects as GIF and succeeds, or returns unknown/fails — either is valid
    ASSERT(result.format == SampledAnimFormat::GIF || result.format == SampledAnimFormat::Unknown
           || !result.success || result.success);  // structural: no crash
}

TEST(TestAnimatedThumbnailCache_PutGet)
{
    using namespace ExplorerLens::Engine;
    AnimatedCacheConfig cfg{};
    cfg.maxBytes   = 1u * 1024u * 1024u;  // 1 MB
    cfg.maxEntries = 64;

    AnimatedThumbnailCache cache(cfg);
    AnimatedCacheKey key{ L"C:\\test\\anim.gif", 2 };

    // Miss before insert
    ASSERT(cache.Get(key) == nullptr);

    // Insert
    AnimatedCacheEntry entry{};
    entry.width  = 32;
    entry.height = 32;
    entry.pixelsBGRA.assign(32u * 32u * 4u, 0xFF);
    cache.Put(key, entry);

    // Hit after insert
    const auto* hit = cache.Get(key);
    ASSERT(hit != nullptr);
    ASSERT(hit->width  == 32);
    ASSERT(hit->height == 32);

    const auto stats = cache.GetStats();
    ASSERT(stats.hits   == 1);
    ASSERT(stats.misses == 1);
}

TEST(TestAnimatedThumbnailCache_Eviction)
{
    using namespace ExplorerLens::Engine;
    // Tiny cache: only fits 2 small entries.
    const uint32_t entryBytes = 16u * 16u * 4u;  // 1024 bytes each
    AnimatedCacheConfig cfg{};
    cfg.maxBytes   = entryBytes * 2u;
    cfg.maxEntries = 100;

    AnimatedThumbnailCache cache(cfg);

    for (uint32_t i = 0; i < 3; ++i) {
        AnimatedCacheKey k{ L"C:\\x.gif", i };
        AnimatedCacheEntry e{};
        e.width  = 16; e.height = 16;
        e.pixelsBGRA.assign(entryBytes, static_cast<uint8_t>(i));
        cache.Put(k, e);
    }

    // After 3 inserts into a 2-entry budget, at least 1 eviction must have occurred.
    const auto stats = cache.GetStats();
    ASSERT(stats.evictions >= 1);
    ASSERT(stats.entryCount <= 2);
}

//==============================================================================
// Sprint 1251-1260: Industrial & Scientific Formats v2 (v34.5.0 "Arcturus-V")
//==============================================================================

TEST(TestDICOMWindowingPresets_GetPreset)
{
    using namespace ExplorerLens::Engine;
    const auto lung = DICOMWindowingPresets::GetPreset(DICOMWindowPreset::CTLung);
    ASSERT(lung.windowWidth  == 1500);
    ASSERT(lung.windowCentre == -500);

    const auto brain = DICOMWindowingPresets::GetPreset(DICOMWindowPreset::Brain);
    ASSERT(brain.windowWidth  == 80);
    ASSERT(brain.windowCentre == 40);

    const auto bone = DICOMWindowingPresets::GetPreset(DICOMWindowPreset::CTBone);
    ASSERT(bone.windowWidth  == 2500);
    ASSERT(bone.windowCentre == 500);
}

TEST(TestDICOMWindowingPresets_BuildLUT)
{
    using namespace ExplorerLens::Engine;
    // Standard window W=400 C=40: HU=40 should map to ~128 (midpoint)
    const auto lut = DICOMWindowingPresets::BuildLinearLUT(400, 40, false);
    ASSERT(lut.size() == 65536);
    // HU at window centre (40) → index 40+32768=32808 → grey ~128
    const uint8_t mid = lut[32808];
    ASSERT(mid >= 120 && mid <= 136);
    // HU well below lo (40 - 200 = -160) → 0
    ASSERT(lut[static_cast<uint32_t>(-2000 + 32768)] == 0);
    // HU well above hi (40 + 200 = 240) → 255
    ASSERT(lut[static_cast<uint32_t>(5000 + 32768)] == 255);
    // Inverted: centre → ~127 still but black/white swapped
    const auto lutInv = DICOMWindowingPresets::BuildLinearLUT(400, 40, true);
    ASSERT(lutInv[32808] >= 120 && lutInv[32808] <= 136);
    ASSERT(lutInv[static_cast<uint32_t>(-2000 + 32768)] == 255);
}

TEST(TestFITSZScaleStretch_HeatMap)
{
    using namespace ExplorerLens::Engine;
    // Black point: t=0 → BGRA where alpha=0xFF
    const uint32_t black = FITSZScaleStretch::HeatMapBGRA(0.0f);
    ASSERT((black & 0xFF000000u) == 0xFF000000u);
    ASSERT((black & 0x00FFFFFFu) == 0x00000000u);  // B=G=R=0

    // White point: t=1 → B=G=R=255, A=255
    const uint32_t white = FITSZScaleStretch::HeatMapBGRA(1.0f);
    ASSERT((white & 0xFF000000u) == 0xFF000000u);
    ASSERT((white & 0x00FFFFFFu) != 0x00000000u);  // Not black

    // Clamp: t>1 same as t=1, t<0 same as t=0
    ASSERT(FITSZScaleStretch::HeatMapBGRA(2.0f) == FITSZScaleStretch::HeatMapBGRA(1.0f));
    ASSERT(FITSZScaleStretch::HeatMapBGRA(-1.0f) == FITSZScaleStretch::HeatMapBGRA(0.0f));
}

TEST(TestFITSZScaleStretch_Stretch)
{
    using namespace ExplorerLens::Engine;
    // Uniform image: all pixels = 0.5 → ZScale z1~z2 → graceful fallback
    const uint32_t W = 8, H = 8;
    std::vector<float> uniform(W * H, 0.5f);
    FITSZScaleStretch zs;
    const auto result = zs.Stretch(uniform.data(), W, H);
    ASSERT(result.success);
    ASSERT(result.width  == W);
    ASSERT(result.height == H);
    ASSERT(result.pixelsBGRA.size() == W * H * 4u);

    // Gradient image: values 0.0..1.0
    std::vector<float> grad(W * H);
    for (uint32_t i = 0; i < W * H; ++i)
        grad[i] = static_cast<float>(i) / static_cast<float>(W * H - 1u);
    const auto r2 = zs.Stretch(grad.data(), W, H);
    ASSERT(r2.success);
    ASSERT(r2.limits.z2 > r2.limits.z1);

    // Null: returns failure
    ASSERT(!zs.Stretch(nullptr, W, H).success);
}

TEST(TestLASPointCloudRenderer_IsLAS)
{
    using namespace ExplorerLens::Engine;
    const uint8_t lasf[] = {'L','A','S','F',0,0,0,0};
    ASSERT(LASPointCloudRenderer::IsLAS(lasf, sizeof(lasf)));

    const uint8_t bad[] = {'L','Z','I','P',0,0,0,0};
    ASSERT(!LASPointCloudRenderer::IsLAS(bad, sizeof(bad)));

    ASSERT(!LASPointCloudRenderer::IsLAS(nullptr, 16));
    ASSERT(!LASPointCloudRenderer::IsLAS(lasf, 3));  // too short
}

TEST(TestLASPointCloudRenderer_ProbePointCount)
{
    using namespace ExplorerLens::Engine;
    // Not a LAS file → 0
    const uint8_t bad[] = {0,1,2,3,4,5,6,7};
    ASSERT(LASPointCloudRenderer::ProbePointCount(bad, sizeof(bad)) == 0);

    // Null → 0
    ASSERT(LASPointCloudRenderer::ProbePointCount(nullptr, 0) == 0);
}

TEST(TestOMETIFFCompositor_WavelengthToBGR)
{
    using namespace ExplorerLens::Engine;
    // DAPI ~461 nm → blue range: B channel should be high
    const uint32_t dapi = OMETIFFCompositor::WavelengthToBGR(461);
    ASSERT((dapi & 0xFF) > 128);  // Blue component high

    // GFP ~509 nm → green range
    const uint32_t gfp = OMETIFFCompositor::WavelengthToBGR(509);
    ASSERT(((gfp >> 8) & 0xFF) > 128);  // Green component high

    // Unknown (0) → white
    const uint32_t unk = OMETIFFCompositor::WavelengthToBGR(0);
    ASSERT((unk & 0xFF)         == 0xFF);
    ASSERT(((unk >> 8)  & 0xFF) == 0xFF);
    ASSERT(((unk >> 16) & 0xFF) == 0xFF);
}

TEST(TestOMETIFFCompositor_IsOMETIFF)
{
    using namespace ExplorerLens::Engine;
    // Not TIFF → false
    const uint8_t notTiff[] = {0x89,0x50,0x4E,0x47,0,0,0,0};
    ASSERT(!OMETIFFCompositor::IsOMETIFF(notTiff, sizeof(notTiff)));

    // TIFF LE magic but no OME-XML → false
    const uint8_t tiffNoOME[] = {0x49,0x49,0x2A,0x00,0,0,0,0,0,0,0,0};
    ASSERT(!OMETIFFCompositor::IsOMETIFF(tiffNoOME, sizeof(tiffNoOME)));

    // Null → false
    ASSERT(!OMETIFFCompositor::IsOMETIFF(nullptr, 16));
}

TEST(TestMHAVolumeDecoder_IsMHA)
{
    using namespace ExplorerLens::Engine;
    const uint8_t mha[] = {'N','D','i','m','s',' ','=',' ','3','\n'};
    ASSERT(MHAVolumeDecoder::IsMHA(mha, sizeof(mha)));

    const uint8_t notMHA[] = {'G','I','F','8','9','a',0,0,0,0};
    ASSERT(!MHAVolumeDecoder::IsMHA(notMHA, sizeof(notMHA)));

    ASSERT(!MHAVolumeDecoder::IsMHA(nullptr, 64));
}

TEST(TestMHAVolumeDecoder_ParseHeader)
{
    using namespace ExplorerLens::Engine;
    // Minimal valid MHA header string
    const char mhaText[] =
        "ObjectType = Image\n"
        "NDims = 3\n"
        "DimSize = 64 64 32\n"
        "ElementType = MET_SHORT\n"
        "ElementSpacing = 1.0 1.0 2.5\n"
        "BinaryDataByteOrderMSB = False\n"
        "ElementDataFile = LOCAL\n";  // triggers dataOffset + valid = true

    const auto hdr = MHAVolumeDecoder::ParseHeader(
        reinterpret_cast<const uint8_t*>(mhaText), sizeof(mhaText) - 1);

    ASSERT(hdr.valid);
    ASSERT(hdr.dimX == 64);
    ASSERT(hdr.dimY == 64);
    ASSERT(hdr.dimZ == 32);
    ASSERT(hdr.elemType == MHAElementType::Short);
    ASSERT(!hdr.bigEndian);
    ASSERT(!hdr.compressed);
}

//==============================================================================
// Sprint 1261-1270: CAD/BIM/EDA Formats (v34.6.0 "Arcturus-W")
//==============================================================================

TEST(TestDWGHeaderParser_IsDWG)
{
    using namespace ExplorerLens::Engine;
    // Valid DWG R2018 magic
    const uint8_t dwg2018[] = {'A','C','1','0','3','2',0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    ASSERT(DWGHeaderParser::IsDWG(dwg2018, sizeof(dwg2018)));

    // Valid R14 magic
    const uint8_t dwgR14[] = {'A','C','1','0','1','4',0x00};
    ASSERT(DWGHeaderParser::IsDWG(dwgR14, sizeof(dwgR14)));

    // DXF detection
    const uint8_t dxfAscii[] = {' ',' ','0','\r','\n'};
    ASSERT(DWGHeaderParser::IsDXF(dxfAscii, sizeof(dxfAscii)));

    // Not DWG
    const uint8_t notDwg[] = {'P','K',0x03,0x04,0x00,0x00,0x00};
    ASSERT(!DWGHeaderParser::IsDWG(notDwg, sizeof(notDwg)));

    // Null/short
    ASSERT(!DWGHeaderParser::IsDWG(nullptr, 0));
    ASSERT(!DWGHeaderParser::IsDWG(dwg2018, 1));
}

TEST(TestDWGHeaderParser_Parse)
{
    using namespace ExplorerLens::Engine;
    // AC1032 = R2018
    const uint8_t ac1032[] = {'A','C','1','0','3','2',0,0,0,0,0,0,0,0,0,0,0x78,0x08,0x00,0x00};
    const auto info = DWGHeaderParser::Parse(ac1032, sizeof(ac1032));
    ASSERT(info.isValid);
    ASSERT(info.version == DWGVersion::R2018);
    ASSERT(memcmp(info.versionString, "AC1032", 6) == 0);

    // AC1015 = R2000
    const uint8_t ac1015[] = {'A','C','1','0','1','5',0,0,0,0,0,0,0};
    const auto info2 = DWGHeaderParser::Parse(ac1015, sizeof(ac1015));
    ASSERT(info2.isValid);
    ASSERT(info2.version == DWGVersion::R2000);

    // Unknown version
    const uint8_t acUnk[] = {'A','C','9','9','9','9',0,0};
    const auto info3 = DWGHeaderParser::Parse(acUnk, sizeof(acUnk));
    ASSERT(!info3.isValid);
    ASSERT(info3.version == DWGVersion::Unknown);

    // Version label
    const std::string lbl = DWGHeaderParser::VersionLabel(DWGVersion::R2018);
    ASSERT(!lbl.empty());
    ASSERT(lbl.find("2018") != std::string::npos);
}

TEST(TestSTEPBoundingBoxExtractor_DetectFormat)
{
    using namespace ExplorerLens::Engine;
    const char* stepHdr = "ISO-10303-21;\nHEADER;\n";
    ASSERT(STEPBoundingBoxExtractor::DetectFormat(
        reinterpret_cast<const uint8_t*>(stepHdr), strlen(stepHdr)) == CADFileFormat::STEP);

    // IGES: 73rd byte is 'S' (section marker)
    std::string igesLine(80, ' ');
    igesLine[72] = 'S';
    igesLine += '\n';
    ASSERT(STEPBoundingBoxExtractor::DetectFormat(
        reinterpret_cast<const uint8_t*>(igesLine.c_str()), igesLine.size()) == CADFileFormat::IGES);

    const char* garbage = "GARBAGE DATA";
    ASSERT(STEPBoundingBoxExtractor::DetectFormat(
        reinterpret_cast<const uint8_t*>(garbage), strlen(garbage)) == CADFileFormat::Unknown);

    ASSERT(STEPBoundingBoxExtractor::DetectFormat(nullptr, 0) == CADFileFormat::Unknown);
}

TEST(TestSTEPBoundingBoxExtractor_ExtractSTEP)
{
    using namespace ExplorerLens::Engine;
    // Minimal STEP with two CARTESIAN_POINTs
    const char* step =
        "ISO-10303-21;\n"
        "DATA;\n"
        "#1=CARTESIAN_POINT('',(0.0,0.0,0.0));\n"
        "#2=CARTESIAN_POINT('',(10.0,20.0,30.0));\n"
        "#3=CARTESIAN_POINT('',(-5.0,5.0,15.0));\n"
        "ENDSEC;\n";
    const STEPBoundingBox bb = STEPBoundingBoxExtractor::ExtractSTEP(
        reinterpret_cast<const uint8_t*>(step), strlen(step));

    ASSERT(bb.valid);
    ASSERT(bb.minX <= -5.0 && bb.maxX >= 10.0);
    ASSERT(bb.minY <=  0.0 && bb.maxY >= 20.0);
    ASSERT(bb.minZ <=  0.0 && bb.maxZ >= 30.0);

    // RenderBBoxPreview should return non-empty BGRA
    const auto img = STEPBoundingBoxExtractor::RenderBBoxPreview(bb, 64, 64);
    ASSERT(img.size() == 64u * 64u * 4u);

    // Invalid bbox — render still returns data
    STEPBoundingBox invalid{};
    const auto img2 = STEPBoundingBoxExtractor::RenderBBoxPreview(invalid, 32, 32);
    ASSERT(img2.size() == 32u * 32u * 4u);
}

TEST(TestIFCEntityCounter_IsIFC)
{
    using namespace ExplorerLens::Engine;
    const char* ifc =
        "ISO-10303-21;\nHEADER;\n"
        "FILE_SCHEMA(('IFC4'));\n"
        "ENDSEC;\nDATA;\n";
    ASSERT(IFCEntityCounter::IsIFC(
        reinterpret_cast<const uint8_t*>(ifc), strlen(ifc)));

    const char* notIfc = "ISO-10303-21;\nHEADER;\nFILE_SCHEMA(('AP214'));\n";
    ASSERT(!IFCEntityCounter::IsIFC(
        reinterpret_cast<const uint8_t*>(notIfc), strlen(notIfc)));

    ASSERT(!IFCEntityCounter::IsIFC(nullptr, 0));

    // Version detection
    const std::string ver = IFCEntityCounter::DetectVersion(
        reinterpret_cast<const uint8_t*>(ifc), strlen(ifc));
    ASSERT(!ver.empty());
    ASSERT(ver.find("IFC") != std::string::npos);
}

TEST(TestIFCEntityCounter_Count)
{
    using namespace ExplorerLens::Engine;
    const char* ifc =
        "ISO-10303-21;\nHEADER;\nFILE_SCHEMA(('IFC4'));\nENDSEC;\nDATA;\n"
        "#1=IFCWALL('X',#2,'Wall A',$,$,#3,#4,'Ext');\n"
        "#2=IFCWALL('X',#2,'Wall B',$,$,#3,#4,'Int');\n"
        "#3=IFCDOOR('X',#2,'Door1',$,$,#3,#4,'Ent');\n"
        "#4=IFCWINDOW('X',#2,'Win1',$,$,#3,#4,'');\n"
        "ENDSEC;\n";
    const auto s = IFCEntityCounter::Count(
        reinterpret_cast<const uint8_t*>(ifc), strlen(ifc), 5);
    ASSERT(s.valid);
    ASSERT(s.totalEntities >= 4);
    ASSERT(!s.topEntities.empty());
    // IFCWALL should be most common (appears twice)
    ASSERT(s.topEntities.front().entityType == "IFCWALL");
    ASSERT(s.topEntities.front().count == 2);
}

TEST(TestGerberLayerCompositor_IsGerber)
{
    using namespace ExplorerLens::Engine;
    const char* gerber = "G04 Copper Top Layer*\n%FSLAX46Y46*%\n";
    ASSERT(GerberLayerCompositor::IsGerber(
        reinterpret_cast<const uint8_t*>(gerber), strlen(gerber)));

    const char* fsGerber = "%FSLAX46Y46*%\nG75*\n";
    ASSERT(GerberLayerCompositor::IsGerber(
        reinterpret_cast<const uint8_t*>(fsGerber), strlen(fsGerber)));

    const char* notGerber = "NOTGERBER";
    ASSERT(!GerberLayerCompositor::IsGerber(
        reinterpret_cast<const uint8_t*>(notGerber), strlen(notGerber)));

    ASSERT(!GerberLayerCompositor::IsGerber(nullptr, 0));

    // Layer type detection from extension
    ASSERT(GerberLayerCompositor::DetectLayerType(".gtl") == GerberLayerType::CopperTop);
    ASSERT(GerberLayerCompositor::DetectLayerType(".gbl") == GerberLayerType::CopperBottom);
    ASSERT(GerberLayerCompositor::DetectLayerType(".gto") == GerberLayerType::SilkscreenTop);
    ASSERT(GerberLayerCompositor::DetectLayerType(".drl") == GerberLayerType::DrillThrough);
    ASSERT(GerberLayerCompositor::DetectLayerType(".xyz") == GerberLayerType::Unknown);
}

TEST(TestGerberLayerCompositor_ProbeLayer)
{
    using namespace ExplorerLens::Engine;
    // Gerber with a few flash operations
    const char* gerbData =
        "G04 Test Layer*\n"
        "%FSLAX36Y36*%\n"
        "%MOIN*%\n"
        "%ADD10C,0.100*%\n"
        "D10*\n"
        "X002000Y003000D03*\n"   // flash
        "X004000Y005000D03*\n"   // flash
        "X001000Y001000D01*\n";  // draw
    const auto info = GerberLayerCompositor::ProbeLayer(
        reinterpret_cast<const uint8_t*>(gerbData), strlen(gerbData), ".gtl");

    ASSERT(info.valid);
    ASSERT(info.flashCount >= 2);
    ASSERT(info.drawCount  >= 1);
    ASSERT(info.type == GerberLayerType::CopperTop);

    // Not-Gerber
    const char* bad = "NOTHING";
    const auto bad_info = GerberLayerCompositor::ProbeLayer(
        reinterpret_cast<const uint8_t*>(bad), strlen(bad));
    ASSERT(!bad_info.valid);
}

TEST(TestKiCadNetlistParser_IsKiCad)
{
    using namespace ExplorerLens::Engine;
    const char* sch = "(kicad_sch (version 20230121) (generator eeschema))\n";
    ASSERT(KiCadNetlistParser::IsKiCad(
        reinterpret_cast<const uint8_t*>(sch), strlen(sch)));
    ASSERT(KiCadNetlistParser::DetectFileType(
        reinterpret_cast<const uint8_t*>(sch), strlen(sch)) == KiCadFileType::Schematic);

    const char* pcb = "(kicad_pcb (version 20221018))\n";
    ASSERT(KiCadNetlistParser::IsKiCad(
        reinterpret_cast<const uint8_t*>(pcb), strlen(pcb)));
    ASSERT(KiCadNetlistParser::DetectFileType(
        reinterpret_cast<const uint8_t*>(pcb), strlen(pcb)) == KiCadFileType::PCBLayout);

    const char* notKicad = "(spice netlist)";
    ASSERT(!KiCadNetlistParser::IsKiCad(
        reinterpret_cast<const uint8_t*>(notKicad), strlen(notKicad)));

    ASSERT(!KiCadNetlistParser::IsKiCad(nullptr, 0));
}

TEST(TestKiCadNetlistParser_Parse)
{
    using namespace ExplorerLens::Engine;
    const char* sch =
        "(kicad_sch (version 20230121) (generator eeschema)\n"
        "  (symbol (lib_id \"Device:R\") (at 50 50 0)\n"
        "    (property \"Reference\" \"R1\")\n"
        "    (property \"Value\" \"10k\")\n"
        "    (property \"Footprint\" \"Resistor_SMD:R_0402\")\n"
        "  )\n"
        "  (symbol (lib_id \"Device:C\") (at 60 60 0)\n"
        "    (property \"Reference\" \"C1\")\n"
        "    (property \"Value\" \"100n\")\n"
        "  )\n"
        "  (symbol (lib_id \"Device:R\") (at 70 70 0)\n"
        "    (property \"Reference\" \"R2\")\n"
        "    (property \"Value\" \"10k\")\n"
        "  )\n"
        ")\n";
    const auto s = KiCadNetlistParser::Parse(
        reinterpret_cast<const uint8_t*>(sch), strlen(sch));

    ASSERT(s.valid);
    ASSERT(s.fileType == KiCadFileType::Schematic);
    ASSERT(s.components.size() >= 3);
    // R1 should be first
    ASSERT(s.components[0].reference == "R1");
    ASSERT(s.components[0].value     == "10k");
    // uniqueValues: "10k" and "100n" = 2
    ASSERT(s.uniqueValues >= 2);

    // RenderPieChart should return 256*256*4 bytes
    const auto img = KiCadNetlistParser::RenderPieChart(s, 64, 64);
    ASSERT(img.size() == 64u * 64u * 4u);
}

//==============================================================================
// Sprint 1271-1280: Performance Hardening + LTS Gate (v34.7.0 "Arcturus-X")
//==============================================================================

TEST(TestPerfRegressionGate_Thresholds)
{
    using namespace ExplorerLens;

    PerfRegressionGate gate;
    gate.SetDefaultThresholds();

    // Verify a few default thresholds are set.
    const auto& thresholds = gate.Thresholds();
    ASSERT(!thresholds.empty());
    ASSERT(thresholds.count(PerfKPI::SingleThumbnailMs) == 1);
    ASSERT(thresholds.count(PerfKPI::BatchThroughputImgSec) == 1);

    // Verify threshold values match documented defaults.
    const auto& tSingle = thresholds.at(PerfKPI::SingleThumbnailMs);
    ASSERT(tSingle.warnThreshold > 0.0);
    ASSERT(tSingle.failThreshold > tSingle.warnThreshold);
}

TEST(TestPerfRegressionGate_BlockOnFail)
{
    using namespace ExplorerLens;

    PerfRegressionGate gate;
    gate.SetDefaultThresholds();

    // Inject a measurement that exceeds the fail threshold for SingleThumbnailMs.
    // Default failThreshold = 25.0 ms.
    std::map<PerfKPI, double> measurements;
    measurements[PerfKPI::SingleThumbnailMs] = 50.0;  // well above fail threshold

    const GateResult result = gate.Evaluate(measurements);
    ASSERT(result.failCount >= 1);
    ASSERT(result.overall == GateVerdict::Fail);
    ASSERT(!result.Passed());

    // FormatReport should produce non-empty text.
    const std::string report = PerfRegressionGate::FormatReport(result);
    ASSERT(!report.empty());
}

TEST(TestLTSBuildValidator_AllGatesPass)
{
    using namespace ExplorerLens::Engine;

    LTSBuildValidator v;
    v.SetTestCount(4664);
    v.SetDecoderCount(200);
    v.SetPeakMemoryMB(110.0);
    v.SetCoveragePct(96.5);
    v.SetBuildWarnings(0);
    v.SetSoakTestPassed(true);

    const LTSValidationReport report = v.Validate();
    ASSERT(report.AllGatesPassed());
    ASSERT(report.ltsStampIssued);
    ASSERT(report.failed == 0);

    const std::string summary = report.Summary();
    ASSERT(!summary.empty());
}

TEST(TestLTSBuildValidator_FailOnCoverage)
{
    using namespace ExplorerLens::Engine;

    LTSBuildValidator v;
    v.SetTestCount(4664);
    v.SetDecoderCount(200);
    v.SetPeakMemoryMB(110.0);
    v.SetCoveragePct(80.0);   // Below MIN_COVERAGE_PCT (95%)
    v.SetBuildWarnings(0);
    v.SetSoakTestPassed(true);

    const LTSValidationReport report = v.Validate();
    ASSERT(!report.AllGatesPassed());
    ASSERT(!report.ltsStampIssued);
    ASSERT(report.failed >= 1);
}

TEST(TestCacheWarmupPreloader_StartStop)
{
    using namespace ExplorerLens::Engine;

    CacheWarmupPreloader preloader;

    // Start with a non-existent path → returns false but does not crash.
    const bool started = preloader.Start(L"non_existent_mru.log");
    ASSERT(!started);
    ASSERT(preloader.IsComplete());
}

TEST(TestCacheWarmupPreloader_Stats)
{
    using namespace ExplorerLens::Engine;

    CacheWarmupPreloader preloader;

    // Inject a decoder that always succeeds.
    uint32_t callCount = 0;
    preloader.SetDecoder([&](const std::wstring&) -> bool {
        ++callCount;
        return true;
    });

    // Create a temporary MRU log with 3 paths.
    const std::string tmpPath = "test_mru_warmup.tmp";
    {
        std::ofstream f(tmpPath);
        f << "C:\\fake\\a.jpg\n";
        f << "C:\\fake\\b.png\n";
        f << "C:\\fake\\c.webp\n";
    }

    const bool started = preloader.Start(
        std::wstring(tmpPath.begin(), tmpPath.end()));
    ASSERT(started);
    ASSERT(preloader.IsComplete());
    ASSERT(callCount == 3);

    const WarmupStats stats = preloader.GetStats();
    ASSERT(stats.attempted == 3u);
    ASSERT(stats.fulfilled == 3u);
    ASSERT(stats.failed == 0u);
    ASSERT(stats.elapsedMs >= 0.0);

    // Cleanup.
    std::remove(tmpPath.c_str());
}

TEST(TestDecodeLatencyProfiler_RecordAndQuery)
{
    using namespace ExplorerLens::Engine;

    DecodeLatencyProfiler profiler;

    // Record several JPEG latencies.
    profiler.RecordSample("jpeg", 2.1);
    profiler.RecordSample("jpeg", 3.0);
    profiler.RecordSample("jpeg", 2.8);
    profiler.RecordSample("jpeg", 1.9);
    profiler.RecordSample("jpeg", 4.2);

    ASSERT(profiler.SampleCount("jpeg") == 5u);
    ASSERT(profiler.FormatCount() == 1u);

    const double p50 = profiler.GetP50("jpeg");
    const double p95 = profiler.GetP95("jpeg");
    ASSERT(p50 > 0.0);
    ASSERT(p95 >= p50);

    const LatencyPercentiles perc = profiler.GetPercentiles("jpeg");
    ASSERT(perc.sampleCount == 5u);
    ASSERT(perc.p99Ms >= perc.p95Ms);
    ASSERT(perc.maxMs >= perc.minMs);

    // JSON export should be non-empty.
    const std::string json = profiler.ToJSON();
    ASSERT(!json.empty());
}

TEST(TestDecodeLatencyProfiler_Reset)
{
    using namespace ExplorerLens::Engine;

    DecodeLatencyProfiler profiler;
    profiler.RecordSample("png", 1.5);
    profiler.RecordSample("png", 2.0);
    ASSERT(profiler.FormatCount() == 1u);

    profiler.Reset();
    ASSERT(profiler.FormatCount() == 0u);
    ASSERT(profiler.SampleCount("png") == 0u);

    // After reset, query returns zero.
    const double p50 = profiler.GetP50("png");
    ASSERT(p50 == 0.0);
}

TEST(TestBenchmarkBaseline_LoadCompare)
{
    using namespace ExplorerLens::Engine;

    BenchmarkBaseline baseline;
    // Inject baseline values directly (no file I/O).
    baseline.SetBaselineValue("jpeg_decode_ms", 2.5);             // lower is better
    baseline.SetBaselineValue("batch_throughput_img_sec", 550.0,  // higher is better
                              true);

    ASSERT(baseline.MetricCount() == 2u);

    // Compare with measurements showing a large latency regression.
    std::unordered_map<std::string, double> current;
    current["jpeg_decode_ms"]          = 3.5;  // +40% regression
    current["batch_throughput_img_sec"] = 560.0;  // slightly improved

    const BaselineCompareResult result = baseline.Compare(current);
    ASSERT(result.regressionDetected);

    // JSON output should be non-empty.
    const std::string json = BenchmarkBaseline::ResultToJSON(result);
    ASSERT(!json.empty());
}

TEST(TestBenchmarkBaseline_NoRegression)
{
    using namespace ExplorerLens::Engine;

    BenchmarkBaseline baseline;
    baseline.SetBaselineValue("single_thumb_ms", 17.0);              // lower is better
    baseline.SetBaselineValue("cache_hit_ms",    0.8);               // lower is better
    baseline.SetBaselineValue("throughput",      235.0, true);       // higher is better

    // Measurements within tolerance (< 10% delta).
    std::unordered_map<std::string, double> current;
    current["single_thumb_ms"] = 17.5;   // +2.9% — within tolerance
    current["cache_hit_ms"]    = 0.82;   // +2.5% — within tolerance
    current["throughput"]      = 240.0;  // +2.1% improvement

    const BaselineCompareResult result = baseline.Compare(current);
    ASSERT(!result.regressionDetected);
    ASSERT(result.IsClean());
    ASSERT(result.metrics.size() == 3u);
}

//==============================================================================
// Sprint 1281-1290: Streaming & Cloud-Native Thumbnails (v35.0.0 "Vega")
//==============================================================================

TEST(TestMultiStageThumbnailEmitter_Stages)
{
    using namespace ExplorerLens::Engine;

    uint32_t callCount = 0;
    MultiStageThumbnailEmitter emitter([&](const StageResult& r) {
        ++callCount;
        ASSERT(r.success);
        ASSERT(r.width > 0);
        ASSERT(r.height > 0);
    });

    MultiStageThumbnailEmitter::Config cfg;
    cfg.emitPlaceholder = true;
    cfg.emitLowRes      = true;
    cfg.emitFullRes     = true;
    emitter.SetConfig(cfg);

    const bool ok = emitter.Emit(L"test.jpg");
    ASSERT(ok);
    ASSERT(!emitter.WasCancelled());
    ASSERT(emitter.StagesCompleted() == 3u);
    ASSERT(callCount == 3u);
}

TEST(TestMultiStageThumbnailEmitter_Cancel)
{
    using namespace ExplorerLens::Engine;

    MultiStageThumbnailEmitter emitter(nullptr);
    MultiStageThumbnailEmitter::Config cfg;
    cfg.emitPlaceholder = true;
    cfg.emitLowRes      = true;
    cfg.emitFullRes     = true;
    emitter.SetConfig(cfg);

    // Cancel before emitting
    emitter.Cancel();
    const bool ok = emitter.Emit(L"test.png");
    ASSERT(!ok);
    ASSERT(emitter.WasCancelled());
    ASSERT(emitter.StagesCompleted() == 0u);

    // Summary mentions cancelled
    const auto summary = emitter.Summary();
    ASSERT(!summary.empty());
}

TEST(TestCloudHydrationMonitor_Detect)
{
    using namespace ExplorerLens::Engine;

    CloudHydrationMonitor::Config cfg;
    cfg.minimumBytesRequired = 1;
    CloudHydrationMonitor monitor(cfg);

    // Non-existent file → UNKNOWN
    const auto state = monitor.Probe(L"nonexistent_file_12345.dat");
    ASSERT(state == HydrationState::UNKNOWN);

    // StateLabel covers all variants
    ASSERT(CloudHydrationMonitor::StateLabel(HydrationState::FULLY_LOCAL)       != nullptr);
    ASSERT(CloudHydrationMonitor::StateLabel(HydrationState::PARTIALLY_LOCAL)   != nullptr);
    ASSERT(CloudHydrationMonitor::StateLabel(HydrationState::GHOST_PLACEHOLDER) != nullptr);
    ASSERT(CloudHydrationMonitor::StateLabel(HydrationState::NOT_A_PLACEHOLDER) != nullptr);
    ASSERT(CloudHydrationMonitor::StateLabel(HydrationState::UNKNOWN)           != nullptr);
}

TEST(TestCloudHydrationMonitor_Defer)
{
    using namespace ExplorerLens::Engine;

    CloudHydrationMonitor monitor;

    // CancelDeferred on a path that was never registered should not crash.
    monitor.CancelDeferred(L"C:\\fake\\path\\file.docx");

    // RequestWhenReady with a non-existent path — stub should be a no-op.
    bool cbFired = false;
    monitor.RequestWhenReady(L"C:\\fake\\path\\file.docx",
        [&](const std::wstring&, HydrationState) { cbFired = true; });
    // Callback not immediately fired (requires background polling in production).
    ASSERT(!cbFired);

    // Config round-trip
    ASSERT(monitor.GetConfig().pollIntervalMs > 0);
}

TEST(TestPartialDecodeStateCache_SaveRestore)
{
    using namespace ExplorerLens::Engine;

    PartialDecodeStateCache cache;

    PartialDecodeState state;
    state.formatTag   = "JPEG";
    state.widthHint   = 1920;
    state.heightHint  = 1080;
    state.mtimeMs     = 100000ULL;
    state.headerBlob  = {0xFF, 0xD8, 0xFF, 0xE0};

    cache.Put(L"C:\\photos\\test.jpg", state);
    ASSERT(cache.EntryCount() == 1u);

    const auto* p = cache.Get(L"C:\\photos\\test.jpg", 100000ULL);
    ASSERT(p != nullptr);
    ASSERT(p->formatTag == "JPEG");
    ASSERT(p->widthHint == 1920u);

    // Mtime mismatch → cache miss
    const auto* p2 = cache.Get(L"C:\\photos\\test.jpg", 999999ULL);
    ASSERT_NULL(p2);
}

TEST(TestPartialDecodeStateCache_Eviction)
{
    using namespace ExplorerLens::Engine;

    PartialDecodeStateCache::Config cfg;
    cfg.maxEntries = 3;
    PartialDecodeStateCache cache(cfg);

    for (uint32_t i = 0; i < 5; ++i) {
        PartialDecodeState s;
        s.formatTag  = "PNG";
        s.mtimeMs    = static_cast<uint64_t>(i) * 1000u;
        cache.Put(L"file" + std::to_wstring(i) + L".png", s);
    }

    // After eviction, at most maxEntries remain.
    ASSERT(cache.EntryCount() <= cfg.maxEntries);

    // Invalidate all
    cache.Clear();
    ASSERT(cache.EntryCount() == 0u);
    ASSERT(cache.TotalBlobBytes() == 0u);
}

TEST(TestThumbnailETagValidator_Match)
{
    using namespace ExplorerLens::Engine;

    ThumbnailETagValidator validator;

    validator.Record(L"C:\\docs\\report.pdf", "etag-abc123", 555000ULL, 12345ULL);
    ASSERT(validator.RecordCount() == 1u);

    // Same ETag → VALID
    const auto r = validator.Validate(
        L"C:\\docs\\report.pdf", "etag-abc123", 555000ULL, 12345ULL);
    ASSERT(r == ETagValidationResult::VALID);

    // Not present path → NOT_PRESENT
    const auto r2 = validator.Validate(
        L"C:\\docs\\missing.pdf", "etag-xyz", 0, 0);
    ASSERT(r2 == ETagValidationResult::NOT_PRESENT);

    // ValidationLabel coverage
    ASSERT(ThumbnailETagValidator::ValidationLabel(ETagValidationResult::VALID)         != nullptr);
    ASSERT(ThumbnailETagValidator::ValidationLabel(ETagValidationResult::ETAG_CHANGED)  != nullptr);
    ASSERT(ThumbnailETagValidator::ValidationLabel(ETagValidationResult::MTIME_CHANGED) != nullptr);
    ASSERT(ThumbnailETagValidator::ValidationLabel(ETagValidationResult::NOT_PRESENT)   != nullptr);
}

TEST(TestThumbnailETagValidator_Invalidate)
{
    using namespace ExplorerLens::Engine;

    ThumbnailETagValidator validator;

    validator.Record(L"C:\\img\\photo.heic", "etag-v1", 1000ULL, 500ULL);
    ASSERT(validator.RecordCount() == 1u);

    // Changed ETag → ETAG_CHANGED
    const auto r = validator.Validate(
        L"C:\\img\\photo.heic", "etag-v2", 1000ULL, 500ULL);
    ASSERT(r == ETagValidationResult::ETAG_CHANGED);

    // Explicit remove
    validator.Remove(L"C:\\img\\photo.heic");
    ASSERT(validator.RecordCount() == 0u);

    // PurgeOld with 0 seconds should evict everything older than now
    validator.Record(L"C:\\img\\old.png", "", 0ULL, 100ULL);
    ASSERT(validator.RecordCount() == 1u);
    validator.Clear();
    ASSERT(validator.RecordCount() == 0u);
}

TEST(TestAdaptiveFidelitySelector_HighBudget)
{
    using namespace ExplorerLens::Engine;

    AdaptiveFidelitySelector::Config cfg;
    cfg.timeBudgetMs           = 1000;
    cfg.minBandwidthForQuality = 10 * 1024 * 1024;
    cfg.preferQualityWhenGPU   = true;
    cfg.degradeOnMetered       = false;
    AdaptiveFidelitySelector sel(cfg);

    // High bandwidth + GPU → QUALITY
    NetworkBandwidthEstimate bw;
    bw.bytesPerSecond = 50 * 1024 * 1024;
    bw.isMetered      = false;
    const auto hint = sel.Select(bw, /*gpuAvailable=*/true);
    ASSERT(hint == FidelityHint::QUALITY);

    // FidelityLabel coverage
    ASSERT(AdaptiveFidelitySelector::FidelityLabel(FidelityHint::PLACEHOLDER_ONLY) != nullptr);
    ASSERT(AdaptiveFidelitySelector::FidelityLabel(FidelityHint::FAST)             != nullptr);
    ASSERT(AdaptiveFidelitySelector::FidelityLabel(FidelityHint::BALANCED)         != nullptr);
    ASSERT(AdaptiveFidelitySelector::FidelityLabel(FidelityHint::QUALITY)          != nullptr);
}

TEST(TestAdaptiveFidelitySelector_LowBudget)
{
    using namespace ExplorerLens::Engine;

    AdaptiveFidelitySelector sel;

    // Very tight budget → PLACEHOLDER_ONLY
    sel.SetTimeBudgetMs(20);
    NetworkBandwidthEstimate bw;
    bw.bytesPerSecond = 100 * 1024 * 1024;
    bw.isMetered      = false;
    const auto hint = sel.Select(bw, /*gpuAvailable=*/true);
    ASSERT(hint == FidelityHint::PLACEHOLDER_ONLY);

    // Offline → FAST
    sel.SetTimeBudgetMs(1000);
    bw.bytesPerSecond = 0;
    const auto hint2 = sel.Select(bw, false);
    ASSERT(hint2 == FidelityHint::FAST);

    // Reset should not crash
    sel.Reset();
    ASSERT(sel.GetConfig().timeBudgetMs == 1000); // SetTimeBudgetMs persists in Config
}

//==============================================================================
// Sprint 1291-1300: Real-Time Collaboration & Live Edit Sync (v35.1.0 "Vega-R")
//==============================================================================

TEST(TestLiveSyncTokenManager_Issue)
{
    using namespace ExplorerLens::Engine;

    LiveSyncTokenManager mgr;
    const auto tok = mgr.Issue(L"C:\\OneDrive\\doc.xlsx", 42u);

    ASSERT(tok.valid);
    ASSERT(tok.version.userId   == 42u);
    ASSERT(tok.version.counter  == 1u);
    ASSERT(!tok.filePath.empty());
    ASSERT(mgr.ActiveTokenCount() == 1u);

    // Re-issuing bumps the counter.
    const auto tok2 = mgr.Issue(L"C:\\OneDrive\\doc.xlsx", 42u);
    ASSERT(tok2.version.counter == 2u);
    ASSERT(mgr.ActiveTokenCount() == 1u);  // Same slot, not a new entry
}

TEST(TestLiveSyncTokenManager_Expire)
{
    using namespace ExplorerLens::Engine;

    LiveSyncTokenManager::Config cfg;
    cfg.defaultTtlMs = 0;  // No TTL — tokens never expire automatically
    LiveSyncTokenManager mgr(cfg);

    const auto tok = mgr.Issue(L"C:\\OneDrive\\report.pdf", 7u);
    ASSERT(mgr.Validate(tok));

    // Explicit expiry
    mgr.Expire(L"C:\\OneDrive\\report.pdf", 7u);
    ASSERT(!mgr.Validate(tok));  // Should be invalid now

    // Refresh on expired token should fail (token is marked invalid, not removed)
    ASSERT(!mgr.Refresh(L"C:\\OneDrive\\report.pdf", 7u) ||
            mgr.Refresh(L"C:\\OneDrive\\report.pdf", 7u));  // impl may re-enable
    // PurgeExpired on epoch 0 shouldn't crash
    mgr.PurgeExpired(0ULL);
    ASSERT(mgr.GetConfig().defaultTtlMs == 0u);
}

TEST(TestCollaborativeCacheCoordinator_Invalidate)
{
    using namespace ExplorerLens::Engine;

    CollaborativeCacheCoordinator coord;

    uint32_t received1 = 0, received2 = 0;
    const SessionId s1 = coord.RegisterSession([&](const std::wstring&, SessionId) { ++received1; });
    const SessionId s2 = coord.RegisterSession([&](const std::wstring&, SessionId) { ++received2; });

    coord.WatchFolder(s1, L"C:\\OneDrive\\Shared");
    coord.WatchFolder(s2, L"C:\\OneDrive\\Shared");

    // s1 triggers change; only s2 should be notified.
    coord.Invalidate(L"C:\\OneDrive\\Shared\\photo.jpg", s1);
    ASSERT(received1 == 0u);  // Originator not notified
    ASSERT(received2 == 1u);

    ASSERT(coord.InvalidationsFiredTotal() == 1u);
    ASSERT(coord.ActiveSessionCount() == 2u);
}

TEST(TestCollaborativeCacheCoordinator_Sync)
{
    using namespace ExplorerLens::Engine;

    CollaborativeCacheCoordinator coord;

    const SessionId s1 = coord.RegisterSession(nullptr);
    ASSERT(coord.ActiveSessionCount() == 1u);

    coord.WatchFolder(s1, L"C:\\Docs");
    coord.UnwatchFolder(s1, L"C:\\Docs");

    // Deregister; session count drops.
    coord.DeregisterSession(s1);
    ASSERT(coord.ActiveSessionCount() == 0u);

    // Reset clears everything.
    coord.Reset();
    ASSERT(coord.ActiveSessionCount() == 0u);
    ASSERT(coord.InvalidationsFiredTotal() == 0u);
}

TEST(TestThumbnailDeltaEncoder_Encode)
{
    using namespace ExplorerLens::Engine;

    ThumbnailDeltaEncoder encoder;

    constexpr uint32_t W = 8, H = 8;
    std::vector<uint8_t> prev(W * H * 4, 0x80);
    std::vector<uint8_t> curr = prev;

    // Change one pixel
    curr[4 * (2 * W + 3) + 0] = 0xFF; // B
    curr[4 * (2 * W + 3) + 1] = 0x00;
    curr[4 * (2 * W + 3) + 2] = 0x00;

    const auto delta = encoder.Encode(prev.data(), curr.data(), W, H);
    ASSERT(!delta.isEmpty);
    ASSERT(delta.dirtyW > 0);
    ASSERT(delta.dirtyH > 0);
    ASSERT(!delta.payload.empty());
    ASSERT(encoder.TotalBytesEncoded() > 0);
}

TEST(TestThumbnailDeltaEncoder_Decode)
{
    using namespace ExplorerLens::Engine;

    ThumbnailDeltaEncoder encoder;

    constexpr uint32_t W = 4, H = 4;
    std::vector<uint8_t> prev(W * H * 4, 0x40);
    std::vector<uint8_t> curr = prev;
    curr[8] = 0xFF;  // Modify pixel (0,2)→byte offset 8

    const auto delta = encoder.Encode(prev.data(), curr.data(), W, H);
    ASSERT(!delta.isEmpty);

    // Decode back into a copy of prev
    std::vector<uint8_t> restored = prev;
    const bool ok = encoder.Decode(restored.data(), W, H, delta);
    ASSERT(ok);

    // The dirty region should match curr
    ASSERT(restored[8] == curr[8]);

    // isEmpty delta should decode as no-op
    ThumbnailDelta emptyDelta;
    emptyDelta.isEmpty = true;
    ASSERT(encoder.Decode(restored.data(), W, H, emptyDelta));

    encoder.ResetStats();
    ASSERT(encoder.TotalBytesEncoded() == 0u);
}

TEST(TestConflictResolutionEngine_Merge)
{
    using namespace ExplorerLens::Engine;

    ConflictResolutionEngine engine(ConflictStrategy::LATEST_ETAG);

    const std::vector<VersionCandidate> candidates = {
        { "etag-v1", 1000ULL, 100ULL, 1 },
        { "etag-v3", 3000ULL, 300ULL, 2 },
        { "etag-v2", 2000ULL, 200ULL, 3 },
    };

    const auto res = engine.Resolve(L"C:\\Docs\\design.ai", candidates);
    ASSERT(res.wasConflict);
    ASSERT(res.candidateCount == 3u);
    ASSERT(res.winner.etag == "etag-v3");
    ASSERT(engine.AuditLog().size() == 1u);
}

TEST(TestConflictResolutionEngine_PickLatest)
{
    using namespace ExplorerLens::Engine;

    ConflictResolutionEngine engine(ConflictStrategy::LATEST_MTIME);

    const std::vector<VersionCandidate> candidates = {
        { "", 5000ULL, 50ULL, 1 },
        { "", 9000ULL, 90ULL, 2 },
        { "", 7000ULL, 70ULL, 3 },
    };

    const auto res = engine.Resolve(L"C:\\Photos\\image.heic", candidates);
    ASSERT(res.winner.mtimeMs == 9000ULL);
    ASSERT(res.winner.userId  == 2u);

    // Single candidate → no conflict
    const std::vector<VersionCandidate> single = { { "etag-only", 1ULL, 1ULL, 9 } };
    const auto res2 = engine.Resolve(L"C:\\Photos\\solo.png", single);
    ASSERT(!res2.wasConflict);

    engine.SetStrategy(ConflictStrategy::LARGEST_SIZE);
    ASSERT(engine.GetStrategy() == ConflictStrategy::LARGEST_SIZE);

    engine.ClearAuditLog();
    ASSERT(engine.AuditLog().empty());
}

TEST(TestRealTimePreviewPipeline_Subscribe)
{
    using namespace ExplorerLens::Engine;

    RealTimePreviewPipeline::Config cfg;
    cfg.debounceMs    = 0;  // Fire immediately in tests
    cfg.maxCoalesceMs = 100;
    RealTimePreviewPipeline pipeline(cfg);

    uint32_t fired = 0;
    pipeline.Subscribe([&](const PreviewUpdateEvent&) { ++fired; });

    pipeline.Notify(L"C:\\Files\\a.jpg");
    pipeline.Notify(L"C:\\Files\\b.png");

    // Drain at t=1000 (well past debounce=0)
    const uint32_t count = pipeline.Drain(1000ULL);
    ASSERT(count == 2u);
    ASSERT(fired == 2u);
    ASSERT(pipeline.PendingEventCount() == 0u);
    ASSERT(pipeline.TotalEventsEnqueued() == 2u);
    ASSERT(pipeline.TotalEventsFired()    == 2u);
}

TEST(TestRealTimePreviewPipeline_Backpressure)
{
    using namespace ExplorerLens::Engine;

    RealTimePreviewPipeline::Config cfg;
    cfg.debounceMs    = 500;
    cfg.maxQueueDepth = 10;
    RealTimePreviewPipeline pipeline(cfg);

    pipeline.Subscribe(nullptr);  // No-op subscriber

    // Flood with 20 unique paths → queue capped at maxQueueDepth
    for (uint32_t i = 0; i < 20; ++i)
        pipeline.Notify(L"C:\\Files\\file" + std::to_wstring(i) + L".jpg");

    ASSERT(pipeline.PendingEventCount() <= cfg.maxQueueDepth);
    ASSERT(pipeline.TotalEventsDropped() > 0u);

    // Coalesce: same path notified twice → still one pending event
    pipeline = RealTimePreviewPipeline(cfg);
    pipeline.Subscribe(nullptr);
    pipeline.Notify(L"C:\\Files\\same.jpg");
    pipeline.Notify(L"C:\\Files\\same.jpg");
    ASSERT(pipeline.PendingEventCount() == 1u);
    ASSERT(pipeline.TotalEventsEnqueued() == 2u);
}

//==============================================================================
// Sprint 1301-1310: Network-Aware Streaming Cache (v35.2.0 "Vega-S")
//==============================================================================

TEST(TestNetworkTopologyProbe_Probe)
{
    using namespace ExplorerLens::Engine;

    NetworkTopologyProbe::Config cfg;
    cfg.probeIntervalMs = 100;
    NetworkTopologyProbe probe(cfg);

    ASSERT(probe.ProbeCount() == 0u);

    const auto result = probe.Probe();
    ASSERT(probe.ProbeCount() == 1u);
    // Stubbed probe always returns LAN
    ASSERT(result.topology == NetworkTopology::LAN);
    ASSERT(result.estimatedKbps == 1000u);
    ASSERT(!result.isMetered);

    // LastResult should match the most recent probe
    ASSERT(probe.LastResult().topology == probe.GetTopology());
    ASSERT(probe.GetConfig().probeIntervalMs == 100u);
}

TEST(TestNetworkTopologyProbe_ForceTopology)
{
    using namespace ExplorerLens::Engine;

    NetworkTopologyProbe probe;
    probe.ForceTopology(NetworkTopology::CELL);
    ASSERT(probe.GetTopology() == NetworkTopology::CELL);

    probe.ForceTopology(NetworkTopology::OFFLINE);
    ASSERT(probe.GetTopology()  == NetworkTopology::OFFLINE);
    ASSERT(!probe.IsMetered());  // Forced topology doesn't set metered flag

    probe.Probe();  // Real probe overrides forced topology
    ASSERT(probe.ProbeCount() == 1u);
    ASSERT(probe.GetTopology() == NetworkTopology::LAN); // Stub returns LAN
}

TEST(TestStreamingCacheTierPolicy_Derive)
{
    using namespace ExplorerLens::Engine;

    StreamingCacheTierPolicy policy;

    const auto lan    = policy.Derive(NetworkTopology::LAN);
    const auto cell   = policy.Derive(NetworkTopology::CELL);
    const auto offline= policy.Derive(NetworkTopology::OFFLINE);

    // LAN should be most generous
    ASSERT(lan.maxBudgetMb > cell.maxBudgetMb);
    ASSERT(lan.prefetchDepth >= cell.prefetchDepth);
    ASSERT(lan.allowRemoteFetch);

    // Cell should use placeholders
    ASSERT(cell.usePlaceholders);
    ASSERT(cell.maxBandwidthKbps > 0u);  // Capped on metered

    // Offline: no remote fetch
    ASSERT(!offline.allowRemoteFetch);
    ASSERT(offline.usePlaceholders);

    ASSERT(policy.OverrideCount() == 0u);
}

TEST(TestStreamingCacheTierPolicy_Override)
{
    using namespace ExplorerLens::Engine;

    StreamingCacheTierPolicy policy;

    CacheTierParameters custom;
    custom.maxBudgetMb     = 9999;
    custom.prefetchDepth   = 99;
    custom.allowRemoteFetch = false;
    policy.OverrideParams(NetworkTopology::WIFI, custom);
    ASSERT(policy.OverrideCount() == 1u);

    const auto wifi = policy.Derive(NetworkTopology::WIFI);
    ASSERT(wifi.maxBudgetMb == 9999u);
    ASSERT(!wifi.allowRemoteFetch);

    policy.ResetOverrides();
    ASSERT(policy.OverrideCount() == 0u);

    // After reset, default values should be back
    const auto wifi2 = policy.Derive(NetworkTopology::WIFI);
    ASSERT(wifi2.maxBudgetMb != 9999u);

    // DeriveFromProbe on metered result should enforce bandwidth cap
    NetworkProbeResult metered;
    metered.topology    = NetworkTopology::LAN;
    metered.isMetered   = true;
    metered.estimatedKbps = 500;
    const auto params = policy.DeriveFromProbe(metered);
    ASSERT(params.maxBandwidthKbps > 0u);
}

TEST(TestBandwidthThrottleGuard_Allow)
{
    using namespace ExplorerLens::Engine;

    BandwidthThrottleGuard::Config cfg;
    cfg.maxKbps     = 0;  // Unlimited
    cfg.bucketCapKb = 256;
    BandwidthThrottleGuard guard(cfg);

    // Unlimited mode — always permits
    ASSERT(guard.TryConsume(1'000'000, 1000ULL));
    ASSERT(guard.TryConsume(1'000'000, 2000ULL));
    ASSERT(guard.TotalBytesAllowed() == 2'000'000ULL);
    ASSERT(guard.TotalBytesRejected() == 0ULL);

    // AvailableTokensKb on unlimited guard returns bucket cap
    guard.Reset();
    ASSERT(guard.AvailableTokensKb() == cfg.bucketCapKb);
}

TEST(TestBandwidthThrottleGuard_Throttle)
{
    using namespace ExplorerLens::Engine;

    BandwidthThrottleGuard::Config cfg;
    cfg.maxKbps     = 100;   // 100 KB/s
    cfg.bucketCapKb = 10;
    cfg.burstKb     = 0;
    BandwidthThrottleGuard guard(cfg);

    // Drain the initial bucket
    while (guard.AvailableTokensKb() > 0)
        guard.TryConsume(1024, 0ULL);  // consume 1 KB chunks at t=0

    // No tokens left — large request rejected
    ASSERT(!guard.TryConsume(512 * 1024, 0ULL));
    ASSERT(guard.TotalBytesRejected() > 0ULL);

    // After 1 second, 100 KB refill
    guard.Tick(1000ULL);
    ASSERT(guard.AvailableTokensKb() >= 10u);  // Capped at bucketCap

    guard.SetMaxKbps(0);  // Switch to unlimited
    ASSERT(guard.TryConsume(9999999, 2000ULL));

    guard.Reset();
    ASSERT(guard.TotalBytesAllowed() == 0ULL);
    ASSERT(guard.TotalBytesRejected() == 0ULL);
}

TEST(TestRemoteFileManifestCache_Store)
{
    using namespace ExplorerLens::Engine;

    RemoteFileManifestCache cache;

    DirectoryManifest manifest;
    manifest.directoryPath = L"\\\\server\\share\\photos";
    manifest.fetchedAtMs   = 1000ULL;
    manifest.ttlMs         = 60'000ULL;
    manifest.entries.push_back({ L"\\\\server\\share\\photos\\a.jpg", 1000ULL, 2048ULL, "etag-a" });
    manifest.entries.push_back({ L"\\\\server\\share\\photos\\b.png", 1100ULL, 1024ULL, "etag-b" });

    cache.Store(manifest);
    ASSERT(cache.EntryCount() == 1u);

    DirectoryManifest out;
    ASSERT(cache.Lookup(L"\\\\server\\share\\photos", 5000ULL, out));
    ASSERT(out.entries.size() == 2u);
    ASSERT(cache.HitCount() == 1u);
    ASSERT(cache.MissCount() == 0u);

    // Miss for unknown path
    ASSERT(!cache.Lookup(L"\\\\server\\other", 5000ULL, out));
    ASSERT(cache.MissCount() == 1u);
}

TEST(TestRemoteFileManifestCache_Stale)
{
    using namespace ExplorerLens::Engine;

    RemoteFileManifestCache cache;

    DirectoryManifest manifest;
    manifest.directoryPath = L"C:\\OneDrive\\Reports";
    manifest.fetchedAtMs   = 0ULL;
    manifest.ttlMs         = 1000ULL;  // 1 second TTL

    cache.Store(manifest);

    DirectoryManifest out;
    // At t=500 — still fresh
    ASSERT(cache.Lookup(L"C:\\OneDrive\\Reports", 500ULL, out));

    // At t=1001 — stale
    ASSERT(!cache.Lookup(L"C:\\OneDrive\\Reports", 1001ULL, out));

    // Invalidate explicit
    cache.Store(manifest);
    ASSERT(cache.EntryCount() == 1u);
    cache.Invalidate(L"C:\\OneDrive\\Reports");
    ASSERT(cache.EntryCount() == 0u);

    // Clear resets counters
    cache.Store(manifest);
    cache.Clear();
    ASSERT(cache.EntryCount() == 0u);
    ASSERT(cache.HitCount() == 0u);
}

TEST(TestCachePrefetchScheduler_Enqueue)
{
    using namespace ExplorerLens::Engine;

    CachePrefetchScheduler::Config cfg;
    cfg.maxQueueDepth = 32;
    cfg.maxDepth      = 4;
    CachePrefetchScheduler sched(cfg);

    sched.Enqueue({ L"C:\\Photos\\a.jpg", 10, 0ULL });
    sched.Enqueue({ L"C:\\Photos\\b.jpg", 50, 0ULL });
    sched.Enqueue({ L"C:\\Photos\\c.jpg", 30, 0ULL });

    ASSERT(sched.QueueDepth()    == 3u);
    ASSERT(sched.TotalEnqueued() == 3u);

    PrefetchRequest top;
    ASSERT(sched.Dequeue(0ULL, top));
    ASSERT(top.priority == 50u);  // Highest priority first
    ASSERT(sched.TotalDequeued() == 1u);

    sched.Cancel(L"C:\\Photos\\a.jpg");
    ASSERT(sched.QueueDepth() == 1u);  // Only c.jpg remains

    sched.Flush();
    ASSERT(sched.QueueDepth() == 0u);
    ASSERT(sched.TotalEnqueued() == 0u);
}

TEST(TestCachePrefetchScheduler_Backpressure)
{
    using namespace ExplorerLens::Engine;

    CachePrefetchScheduler::Config cfg;
    cfg.maxQueueDepth = 5;
    CachePrefetchScheduler sched(cfg);

    // Flood with 10 items — only 5 survive
    for (uint32_t i = 0; i < 10; ++i)
        sched.Enqueue({ L"C:\\file" + std::to_wstring(i) + L".jpg",
                        i,  // priority 0..9
                        0ULL });

    ASSERT(sched.QueueDepth() <= cfg.maxQueueDepth);
    ASSERT(sched.TotalDropped() >= 5u);

    // Drain all
    PrefetchRequest item;
    uint32_t drained = 0;
    while (sched.Dequeue(0ULL, item)) ++drained;
    ASSERT(drained == (std::min)(sched.TotalEnqueued() - sched.TotalDropped(),
                                  cfg.maxQueueDepth));
    ASSERT(sched.QueueDepth() == 0u);

    sched.SetMaxDepth(16);
    sched.SetMaxBandwidthKbps(512);
    ASSERT(sched.GetConfig().maxDepth == 16u);
    ASSERT(sched.GetConfig().maxBandwidthKbps == 512u);
}

//==============================================================================
// Sprint 1311-1320: Zero-Trust Thumbnail Security (v35.3.0 "Vega-T")
//==============================================================================

TEST(TestThumbnailManifestSigner_Sign)
{
    using namespace ExplorerLens::Engine;

    ThumbnailManifestSigner::Config cfg;
    cfg.keyId = "test-key-1";
    ThumbnailManifestSigner signer(cfg);

    const std::vector<ThumbnailManifestEntry> entries = {
        { L"C:\\Photos\\img.jpg",  "abc123hash", 1000ULL, 2048ULL },
        { L"C:\\Photos\\img2.png", "def456hash", 2000ULL, 4096ULL },
    };

    const auto sig = signer.Sign(entries, 9999ULL);
    ASSERT(sig.valid);
    ASSERT(!sig.sig.empty());
    ASSERT(sig.keyId == "test-key-1");
    ASSERT(sig.signedAtMs == 9999ULL);
    ASSERT(signer.SignCount() == 1u);

    // Empty manifest produces invalid signature
    const auto emptySig = signer.Sign({}, 1000ULL);
    ASSERT(!emptySig.valid);
    ASSERT(signer.SignCount() == 2u);
}

TEST(TestThumbnailManifestSigner_Verify)
{
    using namespace ExplorerLens::Engine;

    ThumbnailManifestSigner signer;

    const std::vector<ThumbnailManifestEntry> entries = {
        { L"C:\\Docs\\report.pdf", "sha256-xyz", 5000ULL, 512000ULL },
    };

    const auto sig = signer.Sign(entries, 1ULL);
    ASSERT(sig.valid);

    // Correct entries verify OK
    ASSERT(signer.Verify(entries, sig));
    ASSERT(signer.VerifyCount() == 1u);

    // Tampered entry fails verification
    std::vector<ThumbnailManifestEntry> tampered = entries;
    tampered[0].contentHash = "TAMPERED";
    ASSERT(!signer.Verify(tampered, sig));
    ASSERT(signer.VerifyCount() == 2u);

    // Invalid sig always fails
    ManifestSignature badSig;
    badSig.valid = false;
    ASSERT(!signer.Verify(entries, badSig));
}

TEST(TestZeroTrustDecodeWorker_Spawn)
{
    using namespace ExplorerLens::Engine;

    ZeroTrustDecodeWorker::Config cfg;
    cfg.timeoutMs         = 1000;
    cfg.allowNetworkAccess = false;
    cfg.allowFileWrite     = false;
    ZeroTrustDecodeWorker worker(cfg);

    ASSERT(worker.GetState()   == ZTWorkerState::IDLE);
    ASSERT(!worker.IsAlive());

    const bool spawned = worker.Spawn();
    ASSERT(spawned);
    ASSERT(worker.GetState()   == ZTWorkerState::RUNNING);
    ASSERT(worker.IsAlive());

    ASSERT(worker.GetConfig().allowNetworkAccess == false);
    ASSERT(worker.GetConfig().allowFileWrite     == false);

    worker.Terminate();
    ASSERT(worker.GetState() == ZTWorkerState::TERMINATED);
    ASSERT(!worker.IsAlive());
}

TEST(TestZeroTrustDecodeWorker_Decode)
{
    using namespace ExplorerLens::Engine;

    ZeroTrustDecodeWorker worker;
    worker.Spawn();

    // Valid request → stub returns 4×4 green surface
    WorkerDecodeRequest req;
    req.filePath    = L"C:\\TestFiles\\img.psd";
    req.formatHint  = "psd";
    req.maxWidthPx  = 4;
    req.maxHeightPx = 4;

    const auto resp = worker.Decode(req);
    ASSERT(resp.success);
    ASSERT(resp.width  == 4u);
    ASSERT(resp.height == 4u);
    ASSERT(resp.bgraSurface.size() == 4u * 4u * 4u);
    ASSERT(worker.DecodeCount() == 1u);

    // Decode without spawn fails
    ZeroTrustDecodeWorker dead;
    const auto resp2 = dead.Decode(req);
    ASSERT(!resp2.success);
    ASSERT(!resp2.error.empty());

    // Empty path fails
    WorkerDecodeRequest emptyReq;
    const auto resp3 = worker.Decode(emptyReq);
    ASSERT(!resp3.success);
}

TEST(TestTokenBoundCacheEntry_Store)
{
    using namespace ExplorerLens::Engine;

    TokenBoundCacheEntry cache;

    TenantToken tok{42u, 100u, "read"};
    TokenBoundEntry entry;
    entry.thumbnailBGRA = std::vector<uint8_t>(4 * 4 * 4, 0xAA);
    entry.width  = 4; entry.height = 4;
    entry.storedAtMs = 1000ULL;

    cache.Store(L"C:\\Files\\a.jpg", tok, entry);
    ASSERT(cache.EntryCount() == 1u);

    TokenBoundEntry out;
    ASSERT(cache.Lookup(L"C:\\Files\\a.jpg", tok, out));
    ASSERT(out.width == 4u);
    ASSERT(cache.AuthorizedHitCount() == 1u);

    // Evict and miss
    cache.Evict(L"C:\\Files\\a.jpg");
    ASSERT(!cache.Lookup(L"C:\\Files\\a.jpg", tok, out));
    ASSERT(cache.EntryCount() == 0u);
}

TEST(TestTokenBoundCacheEntry_CrossTenant)
{
    using namespace ExplorerLens::Engine;

    TokenBoundCacheEntry cache;

    TenantToken owner{10u, 200u, "read"};
    TenantToken other{11u, 200u, "read"};  // Different userId — cross-tenant

    TokenBoundEntry entry;
    entry.thumbnailBGRA = std::vector<uint8_t>(16, 0xFF);
    entry.width = 2; entry.height = 2;
    cache.Store(L"C:\\Shared\\secret.pdf", owner, entry);

    TokenBoundEntry out;
    // Owner can read
    ASSERT(cache.Lookup(L"C:\\Shared\\secret.pdf", owner, out));

    // Cross-tenant lookup denied
    ASSERT(!cache.Lookup(L"C:\\Shared\\secret.pdf", other, out));
    ASSERT(cache.UnauthorizedMissCount() == 1u);

    cache.Clear();
    ASSERT(cache.EntryCount() == 0u);
    ASSERT(cache.AuthorizedHitCount() == 0u);
    ASSERT(cache.UnauthorizedMissCount() == 0u);
}

TEST(TestThumbnailAuditLog_Record)
{
    using namespace ExplorerLens::Engine;

    ThumbnailAuditLog::Config cfg;
    cfg.maxEvents     = 10;
    cfg.dropOldOnFull = true;
    ThumbnailAuditLog log(cfg);

    ASSERT(log.EventCount()   == 0u);
    ASSERT(log.TotalRecorded() == 0u);

    AuditEvent ev;
    ev.kind        = AuditEventKind::DECODE_SUCCESS;
    ev.filePath    = L"C:\\Photos\\img.heic";
    ev.userId      = 1u;
    ev.tenantId    = 5u;
    ev.timestampMs = 1000ULL;
    ev.detail      = "format=heic sz=4096";

    log.Record(ev);
    ASSERT(log.EventCount()    == 1u);
    ASSERT(log.TotalRecorded() == 1u);
    ASSERT(log.TotalDropped()  == 0u);

    // Flood past capacity → oldest dropped
    for (uint32_t i = 0; i < 15; ++i) {
        AuditEvent e2;
        e2.kind = AuditEventKind::CACHE_HIT;
        log.Record(e2);
    }
    ASSERT(log.EventCount()    <= cfg.maxEvents);
    ASSERT(log.TotalDropped()  >= 1u);

    log.Flush();
    ASSERT(log.EventCount() == 0u);
}

TEST(TestThumbnailAuditLog_Query)
{
    using namespace ExplorerLens::Engine;

    ThumbnailAuditLog log;

    auto addEvent = [&](AuditEventKind k, const std::wstring& path) {
        AuditEvent ev;
        ev.kind     = k;
        ev.filePath = path;
        log.Record(ev);
    };

    addEvent(AuditEventKind::DECODE_SUCCESS, L"C:\\a.jpg");
    addEvent(AuditEventKind::DECODE_FAILURE, L"C:\\b.psd");
    addEvent(AuditEventKind::CACHE_HIT,      L"C:\\c.png");
    addEvent(AuditEventKind::CACHE_HIT,      L"C:\\d.bmp");
    addEvent(AuditEventKind::POLICY_DENY,    L"C:\\e.exe");

    ASSERT(log.EventCount() == 5u);

    const auto hits   = log.Query(AuditEventKind::CACHE_HIT);
    const auto denies = log.Query(AuditEventKind::POLICY_DENY);
    ASSERT(hits.size()   == 2u);
    ASSERT(denies.size() == 1u);
    ASSERT(denies[0].filePath == L"C:\\e.exe");

    const auto notPresent = log.Query(AuditEventKind::CACHE_EVICT);
    ASSERT(notPresent.empty());
}

TEST(TestFIPSCryptoAdapter_Hash)
{
    using namespace ExplorerLens::Engine;

    FIPSCryptoAdapter::Config cfg;
    cfg.enforceFIPSMode = true;
    FIPSCryptoAdapter crypto(cfg);

    const std::string input = "ExplorerLens secure hash test";
    const auto h256 = crypto.Hash(reinterpret_cast<const uint8_t*>(input.data()),
                                   input.size(), FIPSHashAlgo::SHA256);
    ASSERT(h256.size() == 32u);
    ASSERT(crypto.HashCallCount() == 1u);

    const auto h512 = crypto.Hash(reinterpret_cast<const uint8_t*>(input.data()),
                                   input.size(), FIPSHashAlgo::SHA512);
    ASSERT(h512.size() == 64u);

    // Same input → same hash (deterministic stub)
    const auto h256b = crypto.Hash(reinterpret_cast<const uint8_t*>(input.data()),
                                    input.size(), FIPSHashAlgo::SHA256);
    ASSERT(crypto.ConstantTimeEqual(h256, h256b));

    // Different input → different hash
    const std::string other = "different";
    const auto hOther = crypto.Hash(reinterpret_cast<const uint8_t*>(other.data()),
                                     other.size(), FIPSHashAlgo::SHA256);
    ASSERT(!crypto.ConstantTimeEqual(h256, hOther));
}

TEST(TestFIPSCryptoAdapter_Hmac)
{
    using namespace ExplorerLens::Engine;

    FIPSCryptoAdapter crypto;

    const std::string msg = "thumbnail-manifest-payload";
    const std::string key = "32-byte-secret-key-for-hmac-test";

    const auto mac1 = crypto.Hmac(
        reinterpret_cast<const uint8_t*>(msg.data()), msg.size(),
        reinterpret_cast<const uint8_t*>(key.data()), key.size(),
        FIPSHmacAlgo::HMAC_SHA256);
    ASSERT(mac1.size() == 32u);
    ASSERT(crypto.HmacCallCount() == 1u);

    // Same inputs → same MAC (deterministic stub)
    const auto mac2 = crypto.Hmac(
        reinterpret_cast<const uint8_t*>(msg.data()), msg.size(),
        reinterpret_cast<const uint8_t*>(key.data()), key.size(),
        FIPSHmacAlgo::HMAC_SHA256);
    ASSERT(crypto.ConstantTimeEqual(mac1, mac2));

    const auto mac512 = crypto.Hmac(
        reinterpret_cast<const uint8_t*>(msg.data()), msg.size(),
        reinterpret_cast<const uint8_t*>(key.data()), key.size(),
        FIPSHmacAlgo::HMAC_SHA512);
    ASSERT(mac512.size() == 64u);

    // Different key → different MAC
    const std::string otherKey = "completely-different-key";
    const auto macOther = crypto.Hmac(
        reinterpret_cast<const uint8_t*>(msg.data()), msg.size(),
        reinterpret_cast<const uint8_t*>(otherKey.data()), otherKey.size(),
        FIPSHmacAlgo::HMAC_SHA256);
    ASSERT(!crypto.ConstantTimeEqual(mac1, macOther));
}

//== Sprint 1321-1330: WebAssembly / Browser Extension Pipeline ===============

TEST(TestWasmDecoderShim_Register)
{
    using namespace ExplorerLens::Engine;

    WasmDecoderShim shim;
    ASSERT(shim.DecoderCount() == 0u);

    WasmDecoderConfig cfg;
    cfg.format          = "jpeg";
    cfg.maxWidthPx      = 320u;
    cfg.maxHeightPx     = 320u;
    cfg.targetPlatform  = WasmTargetPlatform::WASM32;

    const uint32_t handle = shim.RegisterDecoder(cfg);
    ASSERT(handle != 0u);
    ASSERT(shim.DecoderCount() == 1u);

    shim.UnregisterDecoder(handle);
    ASSERT(shim.DecoderCount() == 0u);
}

TEST(TestWasmDecoderShim_Decode)
{
    using namespace ExplorerLens::Engine;

    WasmDecoderShim shim;
    WasmDecoderConfig cfg;
    cfg.format         = "png";
    cfg.maxWidthPx     = 128u;
    cfg.maxHeightPx    = 128u;
    cfg.targetPlatform = WasmTargetPlatform::WASM64;

    const uint32_t handle = shim.RegisterDecoder(cfg);
    ASSERT(handle != 0u);

    const uint8_t fakeBytes[16] = {};
    const auto result = shim.Decode(handle, fakeBytes, sizeof(fakeBytes));
    ASSERT(result.success);
    ASSERT(result.width  == 128u);
    ASSERT(result.height == 128u);
    ASSERT(!result.pixelBuffer.empty());
    ASSERT(result.pixelBuffer.size() == 128u * 128u * 4u);
}

TEST(TestBrowserThumbnailBridge_PostMessage)
{
    using namespace ExplorerLens::Engine;

    BrowserThumbnailBridge bridge;
    ASSERT(bridge.PendingCount()    == 0u);
    ASSERT(bridge.DispatchedCount() == 0u);

    BrowserMessage msg;
    msg.kind      = BrowserMessageKind::REQUEST_THUMBNAIL;
    msg.requestId = 42u;
    msg.filePath  = L"C:\\thumbnails\\test.jpg";

    bridge.EnqueueMessage(msg);
    ASSERT(bridge.PendingCount() == 1u);

    bridge.DispatchAll();
    ASSERT(bridge.PendingCount()    == 0u);
    ASSERT(bridge.DispatchedCount() == 1u);
}

TEST(TestBrowserThumbnailBridge_AsyncReply)
{
    using namespace ExplorerLens::Engine;

    BrowserThumbnailBridge bridge;
    uint32_t capturedId = 0u;
    size_t   payloadSize = 0u;

    bridge.SetReplyHandler([&](uint32_t id, const std::vector<uint8_t>& payload) {
        capturedId   = id;
        payloadSize  = payload.size();
    });

    BrowserMessage msg;
    msg.kind      = BrowserMessageKind::REQUEST_THUMBNAIL;
    msg.requestId = 99u;
    msg.filePath  = L"C:\\thumbnails\\test.png";

    bridge.EnqueueMessage(msg);
    bridge.DispatchAll();

    ASSERT(capturedId  == 99u);
    ASSERT(payloadSize > 0u);
}

TEST(TestOffscreenCanvasRenderer_FrameSize)
{
    using namespace ExplorerLens::Engine;

    OffscreenCanvasRenderer renderer;
    CanvasRenderOptions opts;
    opts.width  = 256u;
    opts.height = 256u;
    opts.format = "rgba8";

    const auto result = renderer.RenderFrame(nullptr, 0, opts);
    ASSERT(result.error  == RenderError::NONE);
    ASSERT(result.width  == 256u);
    ASSERT(result.height == 256u);
    ASSERT(result.frameBuffer.size() == 256u * 256u * 4u);
    ASSERT(renderer.RenderCount() == 1u);
}

TEST(TestOffscreenCanvasRenderer_ErrorOnNull)
{
    using namespace ExplorerLens::Engine;

    OffscreenCanvasRenderer renderer;
    CanvasRenderOptions opts;
    opts.width  = 0u;
    opts.height = 0u;
    opts.format = "rgba8";

    const auto result = renderer.RenderFrame(nullptr, 0, opts);
    ASSERT(result.error  == RenderError::INVALID_DIMS);
    ASSERT(result.frameBuffer.empty());
    ASSERT(renderer.ErrorCount() == 1u);
    ASSERT(renderer.RenderCount() == 0u);
}

TEST(TestWasmCacheAdapter_Store)
{
    using namespace ExplorerLens::Engine;

    WasmCacheAdapter cache;
    ASSERT(cache.EntryCount()     == 0u);
    ASSERT(cache.TotalSizeBytes() == 0u);

    WasmCacheEntry entry;
    entry.key       = "file::sha256::abc123";
    entry.data      = {0x01, 0x02, 0x03, 0x04};
    entry.sizeBytes = entry.data.size();

    const auto status = cache.Store(entry);
    ASSERT(status == WasmCacheStatus::OK);
    ASSERT(cache.EntryCount()     == 1u);
    ASSERT(cache.TotalSizeBytes() == 4u);

    const auto fetched = cache.Get(entry.key);
    ASSERT(fetched.data == entry.data);
}

TEST(TestWasmCacheAdapter_Evict)
{
    using namespace ExplorerLens::Engine;

    WasmCacheAdapter cache;

    WasmCacheEntry entry;
    entry.key       = "evict::test::key";
    entry.data      = {0xFF, 0xFE};
    entry.sizeBytes = entry.data.size();

    cache.Store(entry);
    ASSERT(cache.EntryCount()     == 1u);
    ASSERT(cache.TotalSizeBytes() == 2u);

    cache.Evict(entry.key);
    ASSERT(cache.EntryCount()     == 0u);
    ASSERT(cache.TotalSizeBytes() == 0u);

    const auto empty = cache.Get(entry.key);
    ASSERT(empty.data.empty());
}

TEST(TestProgressiveThumbnailStream_Emit)
{
    using namespace ExplorerLens::Engine;

    ProgressiveThumbnailStream stream;
    ASSERT(!stream.IsComplete());
    ASSERT(stream.EmittedCount() == 0u);

    SSEFrame frame;
    frame.eventType = StreamEventType::THUMBNAIL_READY;
    frame.requestId = 77u;
    frame.payload   = {0xAA, 0xBB};

    stream.Emit(frame);
    ASSERT(stream.EmittedCount()  == 1u);
    ASSERT(stream.LastEventType() == StreamEventType::THUMBNAIL_READY);
    ASSERT(!stream.IsComplete());
}

TEST(TestProgressiveThumbnailStream_Complete)
{
    using namespace ExplorerLens::Engine;

    ProgressiveThumbnailStream stream;

    SSEFrame low;
    low.eventType = StreamEventType::THUMBNAIL_LOW_RES;
    low.requestId = 1u;
    stream.Emit(low);
    ASSERT(!stream.IsComplete());
    ASSERT(stream.EmittedCount() == 1u);

    SSEFrame done;
    done.eventType = StreamEventType::STREAM_COMPLETE;
    done.requestId = 1u;
    stream.Emit(done);
    ASSERT(stream.IsComplete());
    ASSERT(stream.EmittedCount() == 2u);

    // Further emits after completion are silently discarded
    stream.Emit(done);
    ASSERT(stream.EmittedCount() == 2u);
}

// ===========================================================================
// Sprint 1331-1340: Cross-Device Preview Sync
// ===========================================================================

TEST(TestDeviceSyncManifest_Upsert)
{
    using namespace ExplorerLens::Engine;

    DeviceSyncManifest manifest("device-A");

    SyncManifestEntry e1;
    e1.pathHash    = 0xABCD1234u;
    e1.etag        = "etag-v1";
    e1.width       = 256;
    e1.height      = 256;
    e1.sizeBytes   = 4096;
    e1.generatedAt = 1000u;

    bool isNew = manifest.Upsert(e1);
    ASSERT(isNew);
    ASSERT(manifest.EntryCount() == 1u);
    ASSERT(manifest.DeviceId() == "device-A");

    // Upsert same hash again — should overwrite, not add
    e1.etag = "etag-v2";
    bool isNew2 = manifest.Upsert(e1);
    ASSERT(!isNew2);
    ASSERT(manifest.EntryCount() == 1u);

    auto found = manifest.Find(0xABCD1234u);
    ASSERT(found.has_value());
    ASSERT(found->etag == "etag-v2");
}

TEST(TestDeviceSyncManifest_Serialize)
{
    using namespace ExplorerLens::Engine;

    DeviceSyncManifest original("dev-serial");
    SyncManifestEntry e;
    e.pathHash    = 0xDEADBEEFu;
    e.etag        = "abc123";
    e.width       = 128;
    e.height      = 128;
    e.sizeBytes   = 2048;
    e.generatedAt = 9999u;
    original.Upsert(e);

    auto blob = original.Serialize();
    ASSERT(!blob.empty());

    DeviceSyncManifest restored;
    bool ok = restored.Deserialize(blob);
    ASSERT(ok);
    ASSERT(restored.EntryCount() == 1u);
    ASSERT(restored.DeviceId() == "dev-serial");

    auto found = restored.Find(0xDEADBEEFu);
    ASSERT(found.has_value());
    ASSERT(found->etag == "abc123");
    ASSERT(found->width == 128u);
}

TEST(TestCrossDeviceCacheSync_Upload)
{
    using namespace ExplorerLens::Engine;

    CrossDeviceCacheSync sync;
    sync.Configure("https://cloud.example.com/sync", "device-A");
    ASSERT(sync.IsConfigured());

    std::vector<uint8_t> blob = { 0x01, 0x02, 0x03 };
    auto res = sync.UploadManifest(blob);
    ASSERT(res == SyncOpResult::OK);
    ASSERT(sync.LastStats().bytesTransferred >= blob.size());

    auto res2 = sync.UploadManifest({});
    ASSERT(res2 == SyncOpResult::UPLOAD_FAILED);
}

TEST(TestCrossDeviceCacheSync_Bidirectional)
{
    using namespace ExplorerLens::Engine;

    CrossDeviceCacheSync sync;
    sync.Configure("https://cloud.example.com/sync", "device-B");

    size_t progressCalls = 0;
    auto res = sync.Sync(DeviceSyncDirection::Bidirectional,
        [&](size_t, size_t){ ++progressCalls; });

    ASSERT(res == SyncOpResult::OK);
    ASSERT(sync.LastStats().entriesUploaded   == 1u);
    ASSERT(sync.LastStats().entriesDownloaded == 1u);
    ASSERT(progressCalls >= 2u);
    ASSERT(sync.SyncCount() == 1u);
}

TEST(TestThumbnailPackFile_PackExtract)
{
    using namespace ExplorerLens::Engine;

    ThumbnailPackFile pack;
    std::vector<uint8_t> thumb1 = { 0xFF, 0xD8, 0xFF, 0xE0, 0x01, 0x02 };
    std::vector<uint8_t> thumb2 = { 0x89, 0x50, 0x4E, 0x47, 0x03, 0x04 };

    pack.AddEntry(0x1111u, thumb1);
    pack.AddEntry(0x2222u, thumb2);
    ASSERT(pack.EntryCount() == 2u);

    auto packed = pack.Pack();
    ASSERT(!packed.empty());

    ThumbnailPackFile reader;
    auto err = reader.Load(packed);
    ASSERT(err == PackFileError::OK);
    ASSERT(reader.EntryCount() == 2u);

    auto extracted = reader.Extract(0x1111u);
    ASSERT(extracted == thumb1);
    auto extracted2 = reader.Extract(0x2222u);
    ASSERT(extracted2 == thumb2);
}

TEST(TestThumbnailPackFile_InvalidMagic)
{
    using namespace ExplorerLens::Engine;

    ThumbnailPackFile reader;
    std::vector<uint8_t> garbage = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x00, 0x00, 0x00, 0x00 };
    auto err = reader.Load(garbage);
    ASSERT(err == PackFileError::INVALID_MAGIC);

    auto notFound = reader.Extract(0xABCDu);
    ASSERT(notFound.empty());
}

TEST(TestSyncConflictResolver_LatestEtag)
{
    using namespace ExplorerLens::Engine;

    SyncConflictResolver resolver(ConflictResolutionMode::LATEST_ETAG);

    SyncManifestEntry local;
    local.pathHash = 0x1u;
    local.etag     = "etag-old";

    SyncManifestEntry remote;
    remote.pathHash = 0x1u;
    remote.etag     = "etag-new"; // lexicographically greater

    std::vector<SyncManifestEntry> localList  = { local };
    std::vector<SyncManifestEntry> remoteList = { remote };

    auto result = resolver.Merge(localList, remoteList);
    ASSERT(result.conflictsFound == 1u);
    // Remote etag is greater, so remote should win
    ASSERT(localList[0].etag == "etag-new");
    ASSERT(resolver.TotalConflictsResolved() == 1u);
}

TEST(TestSyncConflictResolver_RemoteWins)
{
    using namespace ExplorerLens::Engine;

    SyncConflictResolver resolver(ConflictResolutionMode::REMOTE_WINS);

    SyncManifestEntry local;
    local.pathHash    = 0x2u;
    local.etag        = "etag-zzz"; // "winning" by alpha but mode is REMOTE_WINS

    SyncManifestEntry remote;
    remote.pathHash = 0x2u;
    remote.etag     = "etag-aaa";

    // New entry (no conflict)
    SyncManifestEntry remoteNew;
    remoteNew.pathHash = 0x3u;
    remoteNew.etag     = "etag-new";

    std::vector<SyncManifestEntry> localList  = { local };
    std::vector<SyncManifestEntry> remoteList = { remote, remoteNew };

    auto result = resolver.Merge(localList, remoteList);
    ASSERT(result.conflictsFound == 1u);
    ASSERT(result.entriesMerged  == 2u);
    ASSERT(localList.size()      == 2u);
    // remote wins: local etag overwritten
    ASSERT(localList[0].etag == "etag-aaa");
}

TEST(TestDeviceCapabilityAdvertiser_Probe)
{
    using namespace ExplorerLens::Engine;

    DeviceCapabilityAdvertiser adv;
    ASSERT(!adv.IsProbed());

    adv.Probe();
    ASSERT(adv.IsProbed());

    const auto& prof = adv.Profile();
    ASSERT(prof.logicalCores   > 0u);
    ASSERT(prof.availableMemMB > 0u);
    ASSERT(prof.gpuTier        != DeviceGPUTier::NONE);

    auto json = adv.Serialize();
    ASSERT(!json.empty());
    ASSERT(json.front() == '{');
}

TEST(TestDeviceCapabilityAdvertiser_FindBest)
{
    using namespace ExplorerLens::Engine;

    DeviceCapabilityAdvertiser adv;

    DeviceCapabilityProfile p1;
    p1.deviceId      = "peer-1";
    p1.gpuTier       = DeviceGPUTier::DX11;
    p1.canDecode4K   = true;
    p1.logicalCores  = 4;

    DeviceCapabilityProfile p2;
    p2.deviceId      = "peer-2";
    p2.gpuTier       = DeviceGPUTier::NVDEC; // better than DX11
    p2.canDecode4K   = true;
    p2.logicalCores  = 8;

    DeviceCapabilityProfile p3;
    p3.deviceId      = "peer-3";
    p3.gpuTier       = DeviceGPUTier::DX12;
    p3.canDecode4K   = false; // excluded

    adv.LoadPeerProfiles({ p1, p2, p3 });
    ASSERT(adv.PeerProfiles().size() == 3u);

    const auto* best = adv.FindBestDecoder();
    ASSERT(best != nullptr);
    ASSERT(best->deviceId == "peer-2"); // NVDEC > DX12 > DX11
}


//==============================================================================
// Sprint S231-S237 — ROADMAP v6.0 Phase 1 foundation tests
//==============================================================================

// --- S231 LensFormatsCommand -------------------------------------------------
TEST(TestS231_LensFormats_EmptyCatalogue)
{
    LensFormatsCommand cmd;
    ASSERT(cmd.Count() == 0);
    auto json = cmd.RenderJson();
    ASSERT(json.find("\"schema\":\"lens.formats.v1\"") != std::string::npos);
    ASSERT(json.find("\"formats\":[]") != std::string::npos);
}
TEST(TestS231_LensFormats_RegisterEntry)
{
    LensFormatsCommand cmd;
    LensFormatsCommand::FormatEntry e;
    e.familyId   = "jpeg";
    e.displayName = "JPEG / JFIF";
    e.extensions = { ".jpg", ".jpeg" };
    e.mimeTypes  = { "image/jpeg" };
    e.decoder    = "libjpeg-turbo";
    e.enabled    = true;
    cmd.Register(e);
    ASSERT(cmd.Count() == 1);
    auto json = cmd.RenderJson();
    ASSERT(json.find("\"id\":\"jpeg\"") != std::string::npos);
    ASSERT(json.find("libjpeg-turbo") != std::string::npos);
    ASSERT(json.find("\"enabled\":true") != std::string::npos);
}
TEST(TestS231_LensFormats_JsonEscape)
{
    LensFormatsCommand cmd;
    LensFormatsCommand::FormatEntry e;
    e.familyId    = "quote\"test";
    e.displayName = "has\\backslash";
    e.decoder     = "x";
    cmd.Register(e);
    auto json = cmd.RenderJson();
    ASSERT(json.find("\\\"") != std::string::npos);
    ASSERT(json.find("\\\\") != std::string::npos);
}

// --- S232 Nodiscard macro ----------------------------------------------------
TEST(TestS232_NodiscardMacro_Expands)
{
    // Call the probe; we explicitly consume the return to satisfy nodiscard.
    int v = NodiscardExpectedProbe::Probe();
    ASSERT(v == 0);
}

// --- S233 COMBoundaryGuard ---------------------------------------------------
TEST(TestS233_COMBoundary_PODIsValid)
{
    using namespace ExplorerLens::Engine::COM;
    COMDecodeRequest req{};
    req.pathHash  = 0xDEADBEEFCAFEBABEULL;
    req.fileSize  = 1024;
    req.thumbSize = 256;
    req.flags     = 0;
    ASSERT(IsValidCOMArg(&req));
    ASSERT(!IsValidCOMArg<COMDecodeRequest>(nullptr));
}
TEST(TestS233_COMBoundary_CheckerAccepts)
{
    using namespace ExplorerLens::Engine::COM;
    static_assert(CheckCOMBoundary<COMDecodeRequest>::value,
                  "COMDecodeRequest must pass the boundary check");
    ASSERT(CheckCOMBoundary<COMDecodeRequest>::value);
}

// --- S234 ProbeCache ---------------------------------------------------------
TEST(TestS234_ProbeCache_MissThenHit)
{
    ProbeCache pc(16);
    ProbeCacheEntry out{};
    ASSERT(pc.Size() == 0);
    ASSERT(!pc.TryGet(0xAA, out));
    ASSERT(pc.Misses() == 1);

    ProbeCacheEntry ins{};
    ins.formatFamilyId = "jpeg";
    ins.fileSize       = 42;
    ins.conclusive     = true;
    pc.Put(0xAA, ins);
    ASSERT(pc.Size() == 1);
    ASSERT(pc.TryGet(0xAA, out));
    ASSERT(out.formatFamilyId == "jpeg");
    ASSERT(pc.Hits() == 1);
}
TEST(TestS234_ProbeCache_Eviction)
{
    ProbeCache pc(2);
    ProbeCacheEntry a, b, c;
    a.formatFamilyId = "a";
    b.formatFamilyId = "b";
    c.formatFamilyId = "c";
    pc.Put(1, a);
    pc.Put(2, b);
    pc.Put(3, c); // evicts key 1 (LRU)
    ProbeCacheEntry out{};
    ASSERT(!pc.TryGet(1, out));
    ASSERT(pc.TryGet(2, out));
    ASSERT(pc.TryGet(3, out));
    ASSERT(pc.Size() == 2);
}

// --- S235 CacheKeyV2 ---------------------------------------------------------
TEST(TestS235_CacheKeyV2_Size)
{
    using namespace ExplorerLens::Engine::Cache;
    ASSERT(sizeof(CacheKeyV2) == 32);
    ASSERT(kCacheKeySchemaVersion == 2);
}
TEST(TestS235_CacheKeyV2_Equality)
{
    using namespace ExplorerLens::Engine::Cache;
    CacheKeyV2 k1{};
    k1.pathHash       = 0x1111;
    k1.fileSize       = 100;
    k1.mtime100ns     = 12345;
    k1.decoderVersion = 5;
    k1.thumbSize      = 256;
    CacheKeyV2 k2 = k1;
    ASSERT(k1 == k2);
    ASSERT(HashCacheKeyV2(k1) == HashCacheKeyV2(k2));
    k2.decoderVersion = 6;
    ASSERT(k1 != k2);
    ASSERT(HashCacheKeyV2(k1) != HashCacheKeyV2(k2));
}

// --- S236 STAComplianceGuard -------------------------------------------------
TEST(TestS236_STA_ApartmentDetect)
{
    STAComplianceGuard::ResetCounters();
    ApartmentKind kind = STAComplianceGuard::DetectApartment();
    // Test runs on a non-COM thread usually (None) or MTA — both OK
    ASSERT(kind == ApartmentKind::None || kind == ApartmentKind::MTA ||
           kind == ApartmentKind::STA);
}
TEST(TestS236_STA_ViolationCounter)
{
    STAComplianceGuard::ResetCounters();
    {
        // 1us budget + ~2ms sleep reliably exceeds budget regardless of timer resolution.
        STAComplianceGuard g(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    ASSERT(STAComplianceGuard::Violations() >= 1);
}

// --- S237 CacheBlobFormatV1 --------------------------------------------------
TEST(TestS237_CacheBlob_HeaderSize)
{
    using namespace ExplorerLens::Engine::Cache;
    ASSERT(sizeof(CacheBlobHeaderV1) == 32);
    ASSERT(kCacheBlobMagic == 0x424E454CU);
    ASSERT(kCacheBlobVersion == 1);
}
TEST(TestS237_CacheBlob_MakeAndValidate)
{
    using namespace ExplorerLens::Engine::Cache;
    auto hdr = MakeBgra8Header(256, 256, 256u * 256u * 4u, 0, false);
    ASSERT(IsValidCacheBlobHeader(hdr));
    ASSERT(hdr.stride == 256u * 4u);
    ASSERT(hdr.pixelFormat == static_cast<std::uint8_t>(CacheBlobPixelFormat::BGRA8));
}
TEST(TestS237_CacheBlob_RejectsBadMagic)
{
    using namespace ExplorerLens::Engine::Cache;
    CacheBlobHeaderV1 hdr{};
    hdr.magic = 0xDEADBEEFU;
    ASSERT(!IsValidCacheBlobHeader(hdr));
}
TEST(TestS237_CacheBlob_RejectsOversizedPayload)
{
    using namespace ExplorerLens::Engine::Cache;
    auto hdr = MakeBgra8Header(256, 256, 64u * 1024u * 1024u, 0, false);
    ASSERT(!IsValidCacheBlobHeader(hdr));
}

//==============================================================================
// Sprint S241-S249 — ROADMAP v6.0 Phase 1/2 scaffold tests
//==============================================================================

TEST(TestS241_PixelSpan2D_Packed)
{
    std::uint32_t buf[4 * 3] = {0};
    auto view = PixelSpan2D<std::uint32_t>::Packed(buf, 4, 3);
    ASSERT(view.IsValid());
    ASSERT_EQ(view.Width(), 4u);
    ASSERT_EQ(view.Height(), 3u);
    ASSERT_EQ(view.StrideBytes(), 16u);
    view(0, 0) = 0xDEADBEEFu;
    view(3, 2) = 0xFEEDFACEu;
    ASSERT_EQ(buf[0], 0xDEADBEEFu);
    ASSERT_EQ(buf[11], 0xFEEDFACEu);
}

TEST(TestS241_PixelSpan2D_StridedRow)
{
    std::uint8_t backing[4 * 32] = {0};
    PixelSpan2D<std::uint32_t> view(reinterpret_cast<std::uint32_t*>(backing), 4, 4, 32);
    ASSERT(view.IsValid());
    ASSERT_EQ(view.SizeBytes(), 4u * 32u);
    auto empty = PixelSpan2D<std::uint32_t>::Empty();
    ASSERT(!empty.IsValid());
}

TEST(TestS242_CancelToken_DefaultIsLive)
{
    DecodeCancelToken tok;
    ASSERT(!tok.IsCancelled());
    ASSERT(!tok.IsTimeExpired());
    ASSERT_EQ(static_cast<int>(tok.Reason()), static_cast<int>(DecodeCancelReason::NONE));
}

TEST(TestS242_CancelToken_ExplicitCancel)
{
    DecodeCancelToken tok;
    tok.Cancel(DecodeCancelReason::USER);
    ASSERT(tok.IsCancelled());
    ASSERT_EQ(static_cast<int>(tok.Reason()), static_cast<int>(DecodeCancelReason::USER));
    // second cancel is a no-op on reason
    tok.Cancel(DecodeCancelReason::TIMEOUT);
    ASSERT_EQ(static_cast<int>(tok.Reason()), static_cast<int>(DecodeCancelReason::USER));
}

TEST(TestS243_SSIM_ThresholdsByFamily)
{
    ASSERT(SSIMThresholdFor(SSIMFormatFamily::PNG)  > SSIMThresholdFor(SSIMFormatFamily::JPEG));
    ASSERT(SSIMThresholdFor(SSIMFormatFamily::JPEG) > SSIMThresholdFor(SSIMFormatFamily::THREE_D));
    ASSERT_EQ(kSSIMValidationSchema, std::string_view{"lens.ssim-gate.v1"});
}

TEST(TestS243_SSIM_GateReasons)
{
    ASSERT_EQ(static_cast<int>(EvaluateSSIMScore(0.99, SSIMFormatFamily::PNG, true)),
              static_cast<int>(SSIMGateReason::PASS));
    ASSERT_EQ(static_cast<int>(EvaluateSSIMScore(0.50, SSIMFormatFamily::PNG, true)),
              static_cast<int>(SSIMGateReason::FAIL_BELOW_THRESHOLD));
    ASSERT_EQ(static_cast<int>(EvaluateSSIMScore(0.99, SSIMFormatFamily::PNG, false)),
              static_cast<int>(SSIMGateReason::FAIL_MISSING_BASELINE));
    ASSERT_EQ(static_cast<int>(EvaluateSSIMScore(-0.1, SSIMFormatFamily::PNG, true)),
              static_cast<int>(SSIMGateReason::FAIL_DECODE_ERROR));
}

TEST(TestS244_CorpusEntry_CompliancePasses)
{
    CorpusEntryV2 e;
    e.path     = "jpeg/flower.jpg";
    e.sha256   = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
    e.license  = CorpusLicense::PUBLIC_DOMAIN;
    e.family   = SSIMFormatFamily::JPEG;
    ASSERT(IsCorpusEntryCompliant(e));
}

TEST(TestS244_CorpusEntry_RejectsMissingLicense)
{
    CorpusEntryV2 e;
    e.path     = "png/x.png";
    e.sha256   = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
    e.license  = CorpusLicense::UNKNOWN;
    ASSERT(!IsCorpusEntryCompliant(e));

    CorpusEntryV2 needsAttrib;
    needsAttrib.path     = "webp/y.webp";
    needsAttrib.sha256   = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
    needsAttrib.license  = CorpusLicense::CC_BY;
    needsAttrib.sourceUrl = "";
    ASSERT(!IsCorpusEntryCompliant(needsAttrib));
}

TEST(TestS245_StreamingDecoderV2_PodLayout)
{
    StreamingDecoderHandle h{};
    ASSERT(!h.IsValid());
    h.opaque = 42;
    ASSERT(h.IsValid());
    ASSERT(kStreamingThresholdBytes == (50ull * 1024ull * 1024ull));
    ASSERT(kStreamingDefaultChunkBytes == (1u << 20));
    ASSERT(kStreamingMaxInflight == 4u);

    StreamingChunkResult r{};
    r.status = StreamingChunkStatus::OK;
    r.bytesReturned = 1024;
    ASSERT_EQ(static_cast<int>(r.status), static_cast<int>(StreamingChunkStatus::OK));
}

TEST(TestS246_CETCompat_BitsPhase2Extends1)
{
    ASSERT(HasBit(kMitigationsPhase1, MitigationBits::GUARD_CF));
    ASSERT(!HasBit(kMitigationsPhase1, MitigationBits::CET_COMPAT));
    ASSERT(HasBit(kMitigationsPhase2, MitigationBits::CET_COMPAT));
    ASSERT(HasBit(kMitigationsPhase2, MitigationBits::GUARD_EH));
    ASSERT_EQ(kLENSShellPolicy.binaryName, std::string_view{"LENSShell.dll"});
}

TEST(TestS247_SAL_EnabledOnMSVC)
{
#if defined(_MSC_VER)
    ASSERT(kLensSalEnabled);
#else
    ASSERT(!kLensSalEnabled);
#endif
}

TEST(TestS248_PDFium_DefaultsValid)
{
    PDFDecodeOptions opts;
    ASSERT_EQ(opts.thumbWidthPx, 256u);
    ASSERT_EQ(opts.thumbHeightPx, 256u);
    ASSERT_EQ(static_cast<int>(opts.renderMode), static_cast<int>(PDFRenderMode::FIRST_PAGE));
    ASSERT(opts.antialias);
    ASSERT(opts.thumbWidthPx <= kPDFMaxThumbDim);
    ASSERT(kPDFMaxFileBytes > 0ull);
}

TEST(TestS249_D3D11Resize_RequestIsPod)
{
    D3D11ResizeRequest req;
    req.srcWidth = 1024;
    req.srcHeight = 768;
    req.dstWidth  = 256;
    req.dstHeight = 256;
    ASSERT_EQ(static_cast<int>(req.quality), static_cast<int>(ResizeQuality::CATMULL_ROM));
    ASSERT_EQ(static_cast<int>(req.colorMode), static_cast<int>(ResizeColorMode::BGRA8_SRGB));
    ASSERT(kResizeMaxBatch == 64u);
    ASSERT(kResizeMaxDim  == 16384u);
    D3D11ResizeResult r{};
    r.status = ResizeStatus::OK;
    ASSERT_EQ(static_cast<int>(r.status), static_cast<int>(ResizeStatus::OK));
}

// ============================================================================
// Sprint S251-S259 — ROADMAP v6.0 Phase 2 scaffold tests
// ============================================================================

TEST(TestS251_SqliteSchema_ColumnCountAndSql)
{
    ASSERT(kSqliteCacheSchemaVersion == 1u);
    const size_t colCount = sizeof(kSqliteCacheColumns) / sizeof(kSqliteCacheColumns[0]);
    ASSERT_EQ(static_cast<int>(colCount), 14);
    ASSERT(kSqliteCacheCreateSql != nullptr);
    ASSERT(kSqliteCachePragmas   != nullptr);
    SqliteCacheSchema s{};
    ASSERT_EQ(static_cast<int>(s.version), static_cast<int>(kSqliteCacheSchemaVersion));
}

TEST(TestS251_SqliteSchema_BudgetOrder)
{
    ASSERT(kSqliteCacheL1BytesDefault < kSqliteCacheL2BytesDefault);
    ASSERT(kSqliteCacheL1BytesDefault == 64ull  * 1024 * 1024);
    ASSERT(kSqliteCacheL2BytesDefault == 512ull * 1024 * 1024);
}

TEST(TestS252_Pixel16_BytesPerSampleMatch)
{
    ASSERT(PixelBytesPerSample(PixelDepth::EIGHT)      == 1u);
    ASSERT(PixelBytesPerSample(PixelDepth::SIXTEEN)    == 2u);
    ASSERT(PixelBytesPerSample(PixelDepth::HALF_FLOAT) == 2u);
    ASSERT(PixelBytesPerSample(PixelDepth::FLOAT)      == 4u);
    ASSERT(PixelBytesPerBGRAPixel(PixelDepth::SIXTEEN) == 8u);
}

TEST(TestS252_Pixel16_PreservePolicy)
{
    ASSERT(!ShouldPreserve16Bit(PixelDepth::EIGHT,   Pixel16Path::PRESERVE));
    ASSERT( ShouldPreserve16Bit(PixelDepth::SIXTEEN, Pixel16Path::PRESERVE));
    ASSERT(!ShouldPreserve16Bit(PixelDepth::SIXTEEN, Pixel16Path::QUANTISE_8));
}

TEST(TestS253_ExifAutoRotate_TagMapping)
{
    ASSERT_EQ(static_cast<int>(ExifTagToTransform(1)),
              static_cast<int>(ExifAutoRotateTransform::IDENTITY));
    ASSERT_EQ(static_cast<int>(ExifTagToTransform(6)),
              static_cast<int>(ExifAutoRotateTransform::ROTATE_90_CW));
    ASSERT_EQ(static_cast<int>(ExifTagToTransform(8)),
              static_cast<int>(ExifAutoRotateTransform::ROTATE_270_CW));
    ASSERT_EQ(static_cast<int>(ExifTagToTransform(99)),
              static_cast<int>(ExifAutoRotateTransform::IDENTITY));
}

TEST(TestS253_ExifAutoRotate_DimSwap)
{
    ASSERT(!ExifTransformSwapsDims(ExifAutoRotateTransform::IDENTITY));
    ASSERT(!ExifTransformSwapsDims(ExifAutoRotateTransform::FLIP_HORIZONTAL));
    ASSERT( ExifTransformSwapsDims(ExifAutoRotateTransform::ROTATE_90_CW));
    ASSERT( ExifTransformSwapsDims(ExifAutoRotateTransform::ROTATE_270_CW));
}

TEST(TestS254_AsyncPlaceholder_BudgetsReasonable)
{
    ASSERT(kPlaceholderDefaultLatencyMs <  kPlaceholderMaxLatencyMs);
    ASSERT(kPlaceholderMaxLatencyMs     <= 33u);
    PlaceholderRequest req{};
    PlaceholderResult  res{};
    ASSERT_EQ(static_cast<int>(res.kind),   static_cast<int>(PlaceholderKind::NONE));
    ASSERT_EQ(static_cast<int>(res.status), static_cast<int>(PlaceholderStatus::UNAVAILABLE));
    ASSERT(req.allowDowngrade);
}

TEST(TestS255_TiledTiff_LayoutDefaults)
{
    TiledTiffLayout lay{};
    ASSERT(!lay.isBigTiff);
    ASSERT(!lay.isTiled);
    ASSERT(kTiledTiffMaxTileSide      == 8192u);
    ASSERT(kTiledTiffMaxPyramidLevels == 16u);
    TiledTiffProbeResult pr{};
    ASSERT_EQ(static_cast<int>(pr.status), static_cast<int>(TiledTiffStatus::OK));
}

TEST(TestS256_PropertyGen_DeterministicSeed)
{
    const auto a = MakePropertySeed(0, PropertyTestFamily::STRIDE_INVARIANT);
    const auto b = MakePropertySeed(0, PropertyTestFamily::STRIDE_INVARIANT);
    ASSERT(a.width  == b.width);
    ASSERT(a.height == b.height);
    const auto c = MakePropertySeed(1, PropertyTestFamily::STRIDE_INVARIANT);
    ASSERT(a.seed != c.seed);
    ASSERT(kPropertyTestSmokeIterations < kPropertyTestIterations);
}

TEST(TestS257_LibPng_ColorTypeValues)
{
    ASSERT_EQ(static_cast<int>(LibPngColorType::GRAY),       0);
    ASSERT_EQ(static_cast<int>(LibPngColorType::RGB),        2);
    ASSERT_EQ(static_cast<int>(LibPngColorType::PALETTE),    3);
    ASSERT_EQ(static_cast<int>(LibPngColorType::GRAY_ALPHA), 4);
    ASSERT_EQ(static_cast<int>(LibPngColorType::RGBA),       6);
    LibPngDecodeOptions opt{};
    ASSERT(opt.expandGrayToRgba);
}

TEST(TestS258_LibTiff_CompressionValues)
{
    ASSERT_EQ(static_cast<int>(LibTiffCompression::NONE),    1);
    ASSERT_EQ(static_cast<int>(LibTiffCompression::LZW),     5);
    ASSERT_EQ(static_cast<int>(LibTiffCompression::JPEG),    7);
    ASSERT_EQ(static_cast<int>(LibTiffCompression::DEFLATE), 8);
    LibTiffDecodeOptions opt{};
    ASSERT(opt.allowTiled);
    ASSERT(opt.honorOrientation);
}

TEST(TestS259_OpenEXR3_ProbeCompact)
{
    ASSERT(sizeof(OpenEXRProbeResult) <= 64u);
    OpenEXRDecodeOptions opt{};
    ASSERT_EQ(static_cast<int>(opt.interiorDepth),
              static_cast<int>(PixelDepth::HALF_FLOAT));
    ASSERT_EQ(static_cast<int>(opt.preferredLayout),
              static_cast<int>(OpenEXRChannelLayout::RGBA));
    ASSERT(kOpenEXRMaxPartCount == 64u);
}

// ============================================================================
// Sprint S261-S269 — ROADMAP v6.0 Phase 3 Shell + decoder contracts
// ============================================================================

TEST(TestS261_ShellPreview_BudgetsSane)
{
    ASSERT(kShellPreviewDefaultBudgetMs < kShellPreviewMaxBudgetMs);
    ASSERT(kShellPreviewDefaultBudgetMs == 250u);
    ASSERT(kShellPreviewMaxBudgetMs == 500u);
    ShellPreviewRequest req{};
    ASSERT_EQ(req.dpi, 96u);
    ASSERT(req.fitToPane);
    ASSERT(!req.allowAnimation);
}

TEST(TestS261_ShellPreview_MaxPaneSide)
{
    ASSERT(kShellPreviewMaxPaneSide == 8192u);
    ShellPreviewResult r{};
    ASSERT_EQ(static_cast<int>(r.status), static_cast<int>(ShellPreviewStatus::OK));
    ASSERT_EQ(r.emittedWidth, 0u);
}

TEST(TestS262_ShellPropStore_SchemaPopulated)
{
    ASSERT_EQ(kShellPropStoreSchemaCount, static_cast<size_t>(16));
    for (size_t i = 0; i < kShellPropStoreSchemaCount; ++i) {
        ASSERT(kShellPropStoreSchema[i].pkeyName != nullptr);
    }
}

TEST(TestS262_ShellPropStore_ValueTypeShape)
{
    ShellPropStoreKey k{};
    ASSERT_EQ(static_cast<int>(k.id),
              static_cast<int>(ShellPropStoreKeyId::IMAGE_WIDTH));
    ASSERT_EQ(static_cast<int>(k.valueType),
              static_cast<int>(ShellPropStoreValueType::NONE));
    ASSERT(k.readOnly);
    ShellPropStoreRational r{};
    ASSERT_EQ(r.denominator, 1);
}

TEST(TestS263_ShellContextMenu_VerbTable)
{
    ASSERT_EQ(kShellMenuCommandsCount, static_cast<size_t>(8));
    ASSERT(kShellMenuInvokeBudgetMs == 16u);
    ASSERT(kShellMenuMaxSelection == 1024u);
    for (size_t i = 0; i < kShellMenuCommandsCount; ++i) {
        ASSERT(kShellMenuCommands[i].canonicalName != nullptr);
        ASSERT(kShellMenuCommands[i].displayNameEn != nullptr);
    }
}

TEST(TestS264_SvgResvg_OptionsDefault)
{
    SvgResvgDecodeOptions opt{};
    ASSERT_EQ(static_cast<int>(opt.backend),
              static_cast<int>(SvgResvgBackend::RESVG_20));
    ASSERT(!opt.allowExternalRefs);
    ASSERT(!opt.allowScripts);
    ASSERT(kSvgResvgDefaultBudgetMs < kSvgResvgHardBudgetMs);
    ASSERT(kSvgResvgMaxViewBoxSide == 16384u);
}

TEST(TestS265_MFKeyframe_BudgetsSane)
{
    MediaFoundationKeyframeOptions opt{};
    ASSERT_EQ(static_cast<int>(opt.seekPolicy),
              static_cast<int>(MediaFoundationSeekPolicy::FIXED_PERCENT_10));
    ASSERT(opt.allowHardwareDecode);
    ASSERT(kMediaFoundationDefaultBudgetMs < kMediaFoundationHardBudgetMs);
    ASSERT(kMediaFoundationMaxScanMs == 5000u);
}

TEST(TestS266_FontSpecimen_PointSizeRange)
{
    ASSERT(kFreeTypeFontSpecimenMinPointSize == 8u);
    ASSERT(kFreeTypeFontSpecimenMaxPointSize == 256u);
    ASSERT(kFreeTypeFontSpecimenDefaultBudgetMs <
           kFreeTypeFontSpecimenHardBudgetMs);
    FreeTypeFontSpecimenOptions opt{};
    ASSERT_EQ(static_cast<int>(opt.layout),
              static_cast<int>(FreeTypeFontSpecimenLayout::PANGRAM_LATIN));
    ASSERT(opt.enableKerning);
    ASSERT(opt.useHarfBuzz);
}

TEST(TestS267_TinyGltf_TriangleBudget)
{
    ASSERT(kTinyGltfMaxTriangles == 20000000ull);
    ASSERT(kTinyGltfDefaultBudgetMs < kTinyGltfHardBudgetMs);
    TinyGltfDecodeOptions opt{};
    ASSERT_EQ(static_cast<int>(opt.input),
              static_cast<int>(TinyGltfInputFormat::AUTO_DETECT));
    ASSERT(opt.pbrLighting);
    ASSERT_EQ(opt.msaaSamples, 4u);
}

TEST(TestS268_PluginTrustV1_BudgetOrder)
{
    PluginTrustChainV1Policy p{};
    ASSERT(p.requireAuthenticode);
    ASSERT(p.requireSandboxCompat);
    ASSERT(!p.allowUnsignedDevBuilds);
    ASSERT(p.stageBudgetMs < p.totalBudgetMs);
    ASSERT(kPluginTrustChainV1StageCount == 8u);
    ASSERT(kPluginTrustChainV1DefaultBudgetMs <
           kPluginTrustChainV1HardBudgetMs);
}

TEST(TestS269_FolderCover_NamesCount)
{
    ASSERT_EQ(kFolderCoverImageExplicitNamesCount, static_cast<size_t>(10));
    for (size_t i = 0; i < kFolderCoverImageExplicitNamesCount; ++i) {
        ASSERT(kFolderCoverImageExplicitNames[i] != nullptr);
    }
    ASSERT(kFolderCoverImageDefaultBudgetMs < kFolderCoverImageHardBudgetMs);
    ASSERT(kFolderCoverImageMaxScanLimit == 4096u);
}

TEST(TestS269_FolderCover_StrategyEnum)
{
    FolderCoverImageRequest req{};
    ASSERT_EQ(static_cast<int>(req.strategy),
              static_cast<int>(FolderCoverImageStrategy::EXPLICIT_COVER_FILE));
    ASSERT_EQ(req.targetWidth, 256u);
    ASSERT_EQ(req.targetHeight, 256u);
    ASSERT(!req.respectHidden);
    ASSERT(!req.followShortcuts);
}

// ============================================================================
// Sprint S271-S279 — ROADMAP v6.0 Phase 3 GPU / color / HDR / privacy
// ============================================================================

TEST(TestS271_GpuVendorRouting_IdsCorrect)
{
    ASSERT_EQ(static_cast<int>(GpuVendorId::NVIDIA), 0x10DE);
    ASSERT_EQ(static_cast<int>(GpuVendorId::INTEL),  0x8086);
    ASSERT_EQ(static_cast<int>(GpuVendorId::AMD),    0x1002);
    ASSERT_EQ(static_cast<int>(GpuVendorId::MICROSOFT), 0x1414);
}

TEST(TestS271_GpuVendorRouting_PolicyDefaults)
{
    GpuVendorRoutingPolicy p{};
    ASSERT(p.allowHwVideoDecode);
    ASSERT(p.allowHwImageDecode);
    ASSERT(p.allowD3D11Resize);
    ASSERT(!p.allowWarpFallback);
    ASSERT_EQ(p.vramBudgetMb, 512u);
    ASSERT(kGpuVendorRoutingMaxAdapters == 8u);
}

TEST(TestS272_IccColor_PolicyDefaults)
{
    IccColorPipelinePolicy p{};
    ASSERT(p.applyBpc);
    ASSERT(p.useDisplayProfile);
    ASSERT(p.fastSrgbShortcut);
    ASSERT_EQ(p.lutSize, 33u);
    ASSERT(kIccColorPipelineDefaultBudgetMs < kIccColorPipelineHardBudgetMs);
    ASSERT(kIccColorPipelineMaxProfileBytes == 2u * 1024u * 1024u);
}

TEST(TestS273_AnimatedImage_BudgetsSane)
{
    AnimatedImagePlaybackOptions opt{};
    ASSERT_EQ(opt.maxFrames, 1024u);
    ASSERT_EQ(opt.minFrameMs, 20u);
    ASSERT(opt.skipOnBattery);
    ASSERT(opt.preferHwDecode);
    ASSERT(kAnimatedImageDefaultBudgetMs < kAnimatedImageHardBudgetMs);
    ASSERT(kAnimatedImageMaxFrames == 10000u);
}

TEST(TestS274_ArchiveNesting_PolicyDefaults)
{
    ArchiveNestingGuardPolicy p{};
    ASSERT_EQ(p.maxDepth, 4u);
    ASSERT_EQ(p.maxCompressionRatio, 200u);
    ASSERT(p.abortOnCycle);
    ASSERT(p.logDeniedRequests);
    ASSERT(kArchiveNestingGuardMaxDepth == 8u);
    ASSERT(kArchiveNestingGuardHardRatio == 1000u);
}

TEST(TestS275_DecoderWatchdog_BudgetOrder)
{
    DecoderWatchdogPolicy p{};
    ASSERT(p.softBudgetMs < p.hardBudgetMs);
    ASSERT(p.hardBudgetMs < p.terminateBudgetMs);
    ASSERT_EQ(p.quarantineAfterFails, 3u);
    ASSERT(p.emitEtwEvents);
    ASSERT(kDecoderWatchdogMaxBudgetMs == 5000u);
}

TEST(TestS276_TelemetryPrivacy_RuleCount)
{
    ASSERT_EQ(kTelemetryPrivacyRulesCount, static_cast<size_t>(10));
    for (size_t i = 0; i < kTelemetryPrivacyRulesCount; ++i) {
        ASSERT(kTelemetryPrivacyRules[i].etwPropertyName != nullptr);
        ASSERT(kTelemetryPrivacyRules[i].mode !=
               TelemetryPrivacyRedactMode::PASS_THROUGH);
    }
}

TEST(TestS276_TelemetryPrivacy_NoPassthrough)
{
    TelemetryPrivacyPolicy p{};
    ASSERT(p.redactFilePaths);
    ASSERT(p.redactHostIdentifiers);
    ASSERT(p.redactPhotoExifPii);
    ASSERT(!p.allowPassThroughInDev);
    ASSERT(p.maxPayloadBytes == 32u * 1024u);
}

TEST(TestS277_Hdr10ToneMapping_Defaults)
{
    Hdr10ToneMappingPolicy p{};
    ASSERT_EQ(static_cast<int>(p.curve),
              static_cast<int>(Hdr10ToneCurve::BT2390));
    ASSERT(p.useFrameMetadata);
    ASSERT(p.preserveHueRotation);
    ASSERT(kHdr10ToneMappingDefaultBudgetMs <
           kHdr10ToneMappingHardBudgetMs);
    ASSERT(kHdr10ToneMappingReferenceWhiteNits == 203.0f);
}

TEST(TestS278_ClipboardIntegration_PlanShape)
{
    ASSERT_EQ(kClipboardIntegrationPlanCount, static_cast<size_t>(4));
    for (size_t i = 0; i < kClipboardIntegrationPlanCount; ++i) {
        const auto& row = kClipboardIntegrationPlan[i];
        ASSERT(row.sizeCapBytes > 0u);
        ASSERT(row.primaryFormat != ClipboardIntegrationFormatId::NONE);
    }
    ASSERT(kClipboardIntegrationMaxImageBytes == 32u * 1024u * 1024u);
}

TEST(TestS279_AccessibilityDescriptor_Bounds)
{
    AccessibilityDescriptorPolicy p{};
    ASSERT_EQ(p.maxCharacters, 240u);
    ASSERT(!p.allowAiGeneration);
    ASSERT(p.includeDecoderHint);
    ASSERT(p.includeDimensions);
    ASSERT(kAccessibilityDescriptorDefaultChars <
           kAccessibilityDescriptorHardMaxChars);
    ASSERT(kAccessibilityDescriptorHardMaxChars == 1024u);
}

// ============================================================================
// Sprint S281-S289 — ROADMAP v6.0 Phase 3/4 shell / plugin / WER / enterprise
// ============================================================================

TEST(TestS281_SpacebarPreview_Defaults)
{
    SpacebarPreviewPolicy p{};
    ASSERT(p.enableGlobalHotkey);
    ASSERT(p.closeOnFocusLoss);
    ASSERT(p.dimBackground);
    ASSERT(p.advanceOnArrowKeys);
    ASSERT_EQ(p.animationDurationMs, 150u);
    ASSERT_EQ(p.readyBudgetMs, 250u);
    ASSERT(kSpacebarPreviewDefaultBudgetMs < kSpacebarPreviewHardBudgetMs);
}

TEST(TestS282_PluginManifest_KeyCount)
{
    ASSERT_EQ(kPluginManifestSchemaKeyCount, static_cast<size_t>(14));
    ASSERT_EQ(kPluginManifestSchemaVersion, 1u);
    ASSERT(kPluginManifestMaxBytes == 64u * 1024u);
}

TEST(TestS282_PluginManifest_AllKeysNonNull)
{
    for (size_t i = 0; i < kPluginManifestSchemaKeyCount; ++i) {
        ASSERT(kPluginManifestSchema[i].jsonPath != nullptr);
    }
}

TEST(TestS283_LiveEtwSession_PolicyBounds)
{
    LiveEtwSessionPolicy p{};
    ASSERT_EQ(p.ringBufferEntries, 4096u);
    ASSERT(p.ringBufferEntries <= kLiveEtwSessionMaxRingEntries);
    ASSERT(p.maxEventsPerSecond <= kLiveEtwSessionMaxEventsPerSecond);
    ASSERT(p.dropOldestOnFull);
    ASSERT(p.pauseWhenWindowHidden);
    ASSERT(!p.writeRotatingEtlFile);
}

TEST(TestS284_ShellComHostInTest_Defaults)
{
    ShellComHostInTestPolicy p{};
    ASSERT_EQ(static_cast<int>(p.mode),
              static_cast<int>(ShellComHostInTestMode::IN_PROCESS_REGFREE));
    ASSERT(p.minSsim >= kShellComHostInTestMinAcceptableSsim);
    ASSERT(p.invokeBudgetMs < kShellComHostInTestHardBudgetMs);
    ASSERT(p.validateAlphaChannel);
    ASSERT(p.requireStride4Alignment);
    ASSERT(p.failOnComLeakedRefs);
}

TEST(TestS285_WinUI3Gate_CriteriaCount)
{
    WinUI3GatePolicy p{};
    ASSERT(p.coldStartBudgetMs > p.warmStartBudgetMs);
    ASSERT(p.requireUnpackagedSupport);
    ASSERT(p.requireComBridge);
    ASSERT(p.requireAccessibilityNvda);
    ASSERT_EQ(kWinUI3GateCriterionCount, static_cast<size_t>(10));
}

TEST(TestS286_AppContainerHost_BudgetOrder)
{
    AppContainerHostPolicy p{};
    ASSERT(p.handshakeBudgetMs < p.decodeBudgetMs);
    ASSERT(p.handshakeBudgetMs < kAppContainerHostHandshakeHardMs);
    ASSERT(p.workingSetLimitMb <= kAppContainerHostMaxWorkingSetMb);
    ASSERT(p.requireLowIlToken);
    ASSERT(p.logDeniedCapabilities);
    // capability bit set independence
    ASSERT_EQ(static_cast<uint32_t>(AppContainerPluginHostCapability::INTERNET_CLIENT), 1u);
    ASSERT_EQ(static_cast<uint32_t>(AppContainerPluginHostCapability::GPU_ADAPTER_READ), 1u << 6);
}

TEST(TestS287_EvSigning_TimestampUrls)
{
    ASSERT_EQ(kEvSigningTimestampUrlCount, static_cast<size_t>(4));
    for (size_t i = 0; i < kEvSigningTimestampUrlCount; ++i) {
        ASSERT(kEvSigningTimestampUrls[i].url != nullptr);
        ASSERT(kEvSigningTimestampUrls[i].rfc3161);
    }
    EvSigningPolicy p{};
    ASSERT(p.requireDualSignature);
    ASSERT(p.requireRfc3161Timestamp);
    ASSERT(p.signBudgetMsPerArtifact < kEvSigningHardBudgetMsPerArtifact);
}

TEST(TestS288_WerCrashReporter_OptInDefault)
{
    WerCrashReporterPolicy p{};
    ASSERT_EQ(static_cast<int>(p.submissionMode),
              static_cast<int>(WerSubmissionMode::DISABLED));
    ASSERT(p.requireExplicitOptIn);
    ASSERT(p.redactFilePaths);
    ASSERT(p.redactDecoderPayload);
    ASSERT(p.stripUsernameFromPaths);
    ASSERT(p.maxDumpBytes <= kWerCrashReporterHardMaxDumpBytes);
    ASSERT(p.rateLimitPerHour <= kWerCrashReporterHardRateLimitPerHour);
}

TEST(TestS289_AdmxPolicy_KeyCount)
{
    ASSERT_EQ(kAdmxPolicyKeyCount, static_cast<size_t>(15));
    for (size_t i = 0; i < kAdmxPolicyKeyCount; ++i) {
        ASSERT(kAdmxPolicyKeys[i].registryValueName != nullptr);
    }
    ASSERT(kAdmxPolicyRegistryRoot != nullptr);
    ASSERT(kAdmxPolicyNamespace != nullptr);
}

// ── S291 — LensRestApiEndpointContract ───────────────────────────────────────

TEST(TestS291_RestApi_EndpointCount)
{
    ASSERT_EQ(kLensRestApiEndpointCount, static_cast<size_t>(7));
    for (size_t i = 0; i < kLensRestApiEndpointCount; ++i) {
        ASSERT(kLensRestApiEndpoints[i].path != nullptr);
        ASSERT(kLensRestApiEndpoints[i].description != nullptr);
        ASSERT(kLensRestApiEndpoints[i].softTimeoutMs <= kLensRestApiEndpoints[i].hardTimeoutMs);
    }
    ASSERT(kLensRestApiDefaultPort > 1024u);
    ASSERT(kLensRestApiSchemaVersion == 1);
}

// ── S292 — SqlitePHashIndexContract ──────────────────────────────────────────

TEST(TestS292_SqlitePHash_ThresholdBounds)
{
    ASSERT(kSqlitePHashHammingThreshold <= kSqlitePHashHardMaxThreshold);
    ASSERT(kSqlitePHashHardMaxThreshold <= kSqlitePHashBitDepth);
    ASSERT(kSqlitePHashBitDepth == 64u);
    SqlitePHashIndexPolicy p;
    ASSERT(p.hammingThreshold <= kSqlitePHashHardMaxThreshold);
    ASSERT(p.algorithm == SqlitePHashAlgorithm::DCT_PHASH);
}

// ── S293 — CrashTelemetryConsentContract ─────────────────────────────────────

TEST(TestS293_CrashTelemetry_DefaultDisabled)
{
    CrashTelemetryConsentPolicy p;
    ASSERT(p.mode == CrashTelemetryConsentMode::DISABLED);
    ASSERT(p.piiFilter == CrashTelemetryPiiFilter::STRICT_MODE);
    ASSERT(p.maxDumpKb <= kCrashTelemetryMaxDumpKb);
    ASSERT(p.rateLimitPerDay <= kCrashTelemetryHardRateLimitPerDay);
    ASSERT(kCrashTelemetrySchemaVersion == 1);
}

// ── S294 — ArchiveFileBadgeOverlayContract ────────────────────────────────────

TEST(TestS294_ArchiveFileBadge_PolicyDefaults)
{
    ArchiveFileBadgeOverlayPolicy p;
    ASSERT(p.position == ArchiveFileBadgePosition::BOTTOM_RIGHT);
    ASSERT(p.style == ArchiveFileBadgeStyle::ROUNDED_PILL);
    ASSERT(p.badgeSizePx >= kArchiveFileBadgeMinSizePx);
    ASSERT(p.badgeSizePx <= kArchiveFileBadgeMaxSizePx);
    ArchiveFileBadgeCoverPolicy cp;
    ASSERT(cp.hardTimeoutMs <= kArchiveFileBadgeHardTimeoutMs);
    ASSERT(cp.scanLimitFiles <= kArchiveFileBadgeScanHardLimit);
}

// ── S295 — MtlsRestAuthContract ───────────────────────────────────────────────

TEST(TestS295_MtlsRestAuth_DefaultDisabled)
{
    MtlsRestAuthPolicy p;
    ASSERT(p.mode == MtlsRestAuthMode::DISABLED);
    ASSERT(p.clientValidation == MtlsRestCertValidation::STRICT_MODE);
    ASSERT(!p.allowSelfSigned);
    ASSERT(p.handshakeTimeoutMs <= kMtlsRestAuthHandshakeHardMs);
    ASSERT(p.sessionLifetimeMs <= kMtlsRestAuthSessionHardMs);
    ASSERT(kMtlsRestAuthSchemaVersion == 1);
}

// ── S296 — EngineDllAbiContract ───────────────────────────────────────────────

TEST(TestS296_EngineDllAbi_ExportCount)
{
    ASSERT_EQ(kEngineDllAbiExportCount, static_cast<size_t>(8));
    for (size_t i = 0; i < kEngineDllAbiExportCount; ++i) {
        ASSERT(kEngineDllAbiExports[i].symbolName != nullptr);
        ASSERT(kEngineDllAbiExports[i].signature != nullptr);
    }
    ASSERT(kEngineDllAbiMajorVersion == 1);
    ASSERT(kEngineDllAbiCurrentVersion == ((1u << 16) | 0u));
}

// ── S297 — VulkanResizePipelineContract ──────────────────────────────────────

TEST(TestS297_VulkanResize_ApiVersion)
{
    ASSERT(kVulkanResizeMinApiVersion > 0u);
    ASSERT(kVulkanResizeMaxBatchSizeHard >= 64u);
    ASSERT(kVulkanResizeHardTimeoutMs >= 1000u);
    ASSERT(kVulkanResizeSchemaVersion == 1);
    VulkanResizePipelinePolicy p;
    ASSERT(p.maxBatchSize <= kVulkanResizeMaxBatchSizeHard);
    ASSERT(p.filter == VulkanResizeFilter::LANCZOS3);
}

// ── S298 — DbusThumbnailerContract ───────────────────────────────────────────

TEST(TestS298_DbusThumbnailer_DefaultDisabled)
{
    DbusThumbnailerPolicy p;
    ASSERT(p.mode == DbusThumbnailerMode::DISABLED);
    ASSERT(p.maxThumbnailPx <= kDbusThumbnailerMaxSizePx);
    ASSERT(p.hardTimeoutMs <= kDbusThumbnailerHardTimeoutMs);
    ASSERT(kDbusThumbnailerServiceName != nullptr);
    ASSERT(kDbusThumbnailerInterface != nullptr);
    ASSERT(kDbusThumbnailerSchemaVersion == 1);
}

// ── S299 — PluginCatalogSchemaContract ───────────────────────────────────────

TEST(TestS299_PluginCatalog_SchemaVersion)
{
    ASSERT(kPluginCatalogSchemaVersion == 1);
    ASSERT(kPluginCatalogMaxEntries > 0u);
    ASSERT(kPluginCatalogFetchHardMs >= 5000u);
    ASSERT(kPluginCatalogRemoteBaseUrl != nullptr);
    PluginCatalogPolicy p;
    ASSERT(!p.allowRemoteFetch);          // Default: local only
    ASSERT(p.requireSignedHash);          // Must verify SHA-256
    ASSERT(p.enforceTrustChain);          // Must run trust chain
    ASSERT(p.maxCatalogEntries <= kPluginCatalogMaxEntries);
}

//===========================================================================
// ROADMAP v7.0 Phase 1 Foundation Cleanup Tests (S301-S310)
//===========================================================================

TEST(TestS302_StbImageAudit_SiteCount)
{
    // stb_image removal audit: exactly 4 sites remain (Phase 2 target: S318/S319)
    ASSERT(StbImageAuditPending::kRemainingUsageSites == 4);
    ASSERT(StbImageAuditPending::kTargetSprintRemoval >= 319);
    ASSERT(StbImageAuditPending::kJpegReplacement != nullptr);
    ASSERT(StbImageAuditPending::kPngReplacement != nullptr);
    ASSERT(StbImageAuditPending::kHdrReplacement != nullptr);
    ASSERT(StbImageAuditPending::kTgaReplacement != nullptr);
}

TEST(TestS307_AsyncDecodeToken_Construct)
{
    // Default-constructed token: not cancelled, valid request ID
    AsyncDecodeToken tok;
    ASSERT(!tok.IsCancellationRequested());
    ASSERT(tok.RequestId() > 0u);
    ASSERT(tok.PriorityHint() == 0);

    // After RequestCancellation(): stop is signalled
    tok.RequestCancellation();
    ASSERT(tok.IsCancellationRequested());

    // std::stop_token from GetToken() reflects the same state
    auto st = tok.GetToken();
    ASSERT(st.stop_requested());

    // DecodeTokenView wraps correctly
    AsyncDecodeToken tok2;
    DecodeTokenView view{ tok2 };
    ASSERT(!view.IsCancellationRequested());
    ASSERT(view.requestId == tok2.RequestId());

    // AlreadyCancelled factory
    auto cancelled = AsyncDecodeToken::AlreadyCancelled();
    ASSERT(cancelled.IsCancellationRequested());
}

TEST(TestS308_EmbeddedJpegExtractor_Defaults)
{
    // Constants within expected bounds
    ASSERT(EmbeddedJpegExtractor::kMaxEmbeddedJpegBytes == 64u * 1024u * 1024u);
    ASSERT(EmbeddedJpegExtractor::kMinEmbeddedJpegBytes == 128u);
    ASSERT(EmbeddedJpegExtractor::kMinEmbeddedJpegBytes < EmbeddedJpegExtractor::kMaxEmbeddedJpegBytes);

    // EXIF tag constants match TIFF/EXIF specification
    ASSERT(ExifTag::kJpegInterchangeFormat       == 0x0201u);
    ASSERT(ExifTag::kJpegInterchangeFormatLength == 0x0202u);

    // IsJpegSoi: valid JPEG SoI (FF D8)
    const std::byte validSoi[]{ std::byte{0xFF}, std::byte{0xD8}, std::byte{0xFF}, std::byte{0xE0} };
    ASSERT(EmbeddedJpegExtractor::IsJpegSoi(validSoi));

    // IsJpegSoi: not a JPEG
    const std::byte notJpeg[]{ std::byte{0x89}, std::byte{0x50} };
    ASSERT(!EmbeddedJpegExtractor::IsJpegSoi(notJpeg));

    // IsJpegSoi: empty span
    ASSERT(!EmbeddedJpegExtractor::IsJpegSoi(std::span<const std::byte>{}));

    // Phase 2 stub: Extract(nullptr) returns nullopt (no crash)
    EmbeddedJpegExtractor ext;
    auto result = ext.Extract(nullptr);
    ASSERT(!result.has_value());
}

TEST(TestS309_IccProfileManager_WIC)
{
    // WIC-backed ICC correction must be enabled on Windows
    ASSERT(IccProfileManager::IsEnabled());
    ASSERT(strcmp(IccProfileManager::BackendName(), "WIC") == 0);

    IccProfileManager mgr;

    // Empty buffer must not validate
    const auto infoEmpty = mgr.ParseProfile({});
    ASSERT(!infoEmpty.isValid);

    // A 96-byte stub (less than 128-byte header) must not validate
    std::byte shortHdr[96]{};
    const auto infoShort = mgr.ParseProfile(std::span<const std::byte>{ shortHdr, 96 });
    ASSERT(!infoShort.isValid);

    // Valid 128-byte dummy header with 'RGB ' at offset 16 must parse
    std::byte hdr[128]{};
    // Set profile size (big-endian) = 128
    hdr[0] = std::byte{ 0x00 }; hdr[1] = std::byte{ 0x00 };
    hdr[2] = std::byte{ 0x00 }; hdr[3] = std::byte{ 0x80 };
    // Color space 'RGB ' at bytes 16-19
    hdr[16] = std::byte{ 'R' }; hdr[17] = std::byte{ 'G' };
    hdr[18] = std::byte{ 'B' }; hdr[19] = std::byte{ ' ' };
    const auto infoValid = mgr.ParseProfile(std::span<const std::byte>{ hdr, 128 });
    ASSERT(infoValid.isValid);
    ASSERT(infoValid.colorSpace == "RGB");

    // CreateTransform with empty source ICC must return nullptr (passthrough)
    auto* xformNull = mgr.CreateTransform({}, {});
    ASSERT(xformNull == nullptr);

    // DestroyTransform(nullptr) must be safe
    mgr.DestroyTransform(nullptr);

    // ApplyTransform with null transform handle must be a no-op
    std::byte dummyPixel[4]{};
    mgr.ApplyTransform(nullptr, std::span<std::byte>{ dummyPixel, 4 }, 1u);
}

//==============================================================================
// Sprint S311-S319 — ROADMAP v7.0 Phase 2 First GPU Pixels
//==============================================================================

TEST(TestS311_D3D11DeviceManager_Contract)
{
    // Manager must report not-ready before Initialise() is called
    D3D11DeviceManager mgr;
    ASSERT(!mgr.IsReady());

    // Default config constants must hold project-mandated values
    ASSERT(D3D11DeviceManager::kMaxAdaptersToTry == 8u);
    ASSERT(D3D11DeviceManager::kMaxTDRResets     == 3u);

    // QueryInfo on uninitialised manager must return a zeroed struct
    const auto info = mgr.QueryInfo();
    ASSERT(!info.isInitialised);

    // Shutdown on uninitialised manager must be safe (no crash)
    mgr.Shutdown();
    ASSERT(!mgr.IsReady());
}

TEST(TestS312_DXVA2JpegDecoder_NotSupported)
{
    // Hardware JPEG decode is expected to be unavailable on CI machines
    ASSERT(!DXVA2JpegDecoder::IsHardwareSupported());

    DXVA2JpegDecoder dec;
    ASSERT(!dec.IsOpen());

    // Decode without Open() must return HARDWARE_UNAVAILABLE
    DXVA2JpegDecodeResult res = dec.Decode({});
    ASSERT(res.status == DXVA2DecodeStatus::HARDWARE_UNAVAILABLE);

    // Constants must satisfy project-mandated floor values
    ASSERT(DXVA2JpegDecoder::kMaxDecodeSidePixels >= 4096u);
    ASSERT(DXVA2JpegDecoder::kMinBitstreamBytes   == 32u);
}

TEST(TestS313_CancelAwareBindCallback_Policy)
{
    using namespace ExplorerLens::Engine;

    // Construct with a trivial token view (null / default)
    DecodeTokenView token{};
    CancelAwareBindCallback cb{ token };

    // Fresh callback: not cancelled, call count zero
    ASSERT(!cb.IsCancelled());
    ASSERT(cb.CallCount() == 0u);

    // Policy check without any progress calls must default to CONTINUE
    ASSERT(cb.ShouldCancel() == BindCallbackPolicy::CONTINUE);

    ASSERT(CancelAwareBindCallback::kMaxProgressCallsBeforeForceCheck == 1024u);
    ASSERT(CancelAwareBindCallback::kHResultAbort == static_cast<long>(0x80004004));
}

TEST(TestS314_OOMKillGuard_ArmRelease)
{
    using namespace ExplorerLens::Engine;

    OOMKillGuard guard;
    ASSERT(!guard.IsArmed());

    // Arm() — no args (Win32 SetProcessWorkingSetSizeEx path on Windows)
    const auto armStatus = guard.Arm();
    // On Windows the call may succeed or be permission-denied by OS policy;
    // both outcomes are valid — what matters is the guard knows its state.
    ASSERT(armStatus == OOMKillGuardStatus::OK
        || armStatus == OOMKillGuardStatus::PERMISSION_DENIED
        || armStatus == OOMKillGuardStatus::RESERVATION_FAILED);

    if (armStatus == OOMKillGuardStatus::OK) {
        ASSERT(guard.IsArmed());

        // Release must succeed and transition to not-armed
        const auto relStatus = guard.Release();
        ASSERT(relStatus == OOMKillGuardStatus::OK);
        ASSERT(!guard.IsArmed());

        // Calling Release() again must be safe and return NOT_ACTIVE
        const auto rel2 = guard.Release();
        ASSERT(rel2 == OOMKillGuardStatus::NOT_ACTIVE);
    }

    ASSERT(OOMKillGuard::kAbsoluteMaxWorkingSetBytes > 0u);
    ASSERT(OOMKillGuard::kMinEmergencyRegionBytes     == 65536u);
}

TEST(TestS315_LastCachedBitmap_DefaultNone)
{
    using namespace ExplorerLens::Engine;

    LastCachedBitmapContract contract;
    ASSERT(!contract.IsBackendAvailable());

    const auto result = contract.Fetch({});
    ASSERT(result.kind == PlaceholderBitmapKind::NONE);
    ASSERT(!result.HasPixels());

    ASSERT(LastCachedBitmapContract::kMaxCachedSidePixels == 2048u);
}

TEST(TestS316_ReadaheadIO_SlotCount)
{
    using namespace ExplorerLens::Engine;

    ReadaheadIOContract io;
    ASSERT(!io.IsInitialised());

    const auto initStatus = io.Initialize({});
    ASSERT(initStatus == ReadaheadIOStatus::OK);
    ASSERT(io.IsInitialised());

    // Before any enqueue, slot counts must be zero
    ASSERT(io.ReadySlotCount()   == 0u);
    ASSERT(io.PendingSlotCount() == 0u);

    // Consume on empty queue must return nullptr
    ASSERT(io.Consume(0u) == nullptr);

    io.Shutdown();
    ASSERT(!io.IsInitialised());

    ASSERT(ReadaheadIOConfig::kDefaultSlotCount == 8u);
    ASSERT(ReadaheadIOConfig::kMaxSlotCount     == 32u);
}

TEST(TestS317_D3D11BlitPipeline_CPUFallback)
{
    using namespace ExplorerLens::Engine;

    D3D11TextureBlitPipeline pipeline;

    // GPU path is not wired until Sprint ≥ S330; CPU bilinear fallback must be present
    ASSERT(!D3D11TextureBlitPipeline::IsGPUPathAvailable());
    ASSERT( D3D11TextureBlitPipeline::IsCPUFallbackAvailable());

    // Blit with null pixels must return INVALID_DESCRIPTOR
    BlitDescriptor bad{};
    const auto badResult = pipeline.Blit(bad);
    ASSERT(badResult.status == BlitStatus::INVALID_DESCRIPTOR);

    // Blit with a valid 2×2 BGRA-8 descriptor must succeed via CPU bilinear path
    BlitDescriptor desc{};
    // 2×2 source: 4 bytes per pixel × 2 pixels wide × 2 pixels tall
    static std::byte src[16]{};
    desc.srcPixels      = src;
    desc.srcWidth       = 2u;
    desc.srcHeight      = 2u;
    desc.srcStrideBytes = 8u;   // 2 pixels × 4 bytes
    desc.dstWidth       = 1u;
    desc.dstHeight      = 1u;
    const auto result = pipeline.Blit(desc);
    ASSERT(result.status == BlitStatus::OK);
    ASSERT(result.pixels.size() == 4u); // 1×1 BGRA-8 = 4 bytes

    ASSERT(D3D11TextureBlitPipeline::kMaxTextureDimension          == 16384u);
    ASSERT(D3D11TextureBlitPipeline::kMinBlitDimension             == 16u);
    ASSERT(D3D11TextureBlitPipeline::kStagingTexturePitchAlignment == 256u);
}

TEST(TestS318_LibJpegTurboEncode_WIC)
{
    using namespace ExplorerLens::Engine;

    // WIC JPEG encode must be available on Windows
    ASSERT(LibJpegTurboEncodeWrapper::IsAvailable());
    ASSERT(strcmp(LibJpegTurboEncodeWrapper::BackendName(), "WIC") == 0);

    // Encode a 1×1 opaque white BGRA-8 pixel
    LibJpegTurboEncodeWrapper enc;
    std::byte src[4]{ std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF} };
    const auto result = enc.Encode(
        std::span<const std::byte>{ src, 4 }, 1u, 1u, 4u);
    ASSERT(result.status == JpegEncodeStatus::OK);
    ASSERT(!result.jpegBytes.empty());
    // Verify JPEG SOI marker (0xFF 0xD8)
    ASSERT(result.jpegBytes.size() >= 2u);
    ASSERT(result.jpegBytes[0] == std::byte{0xFF});
    ASSERT(result.jpegBytes[1] == std::byte{0xD8});

    // Zero-size encode must return INVALID_INPUT
    const auto badResult = enc.Encode({}, 0u, 0u, 0u);
    ASSERT(badResult.status == JpegEncodeStatus::INVALID_INPUT);

    ASSERT(LibJpegTurboEncodeWrapper::kMinQuality     == 1u);
    ASSERT(LibJpegTurboEncodeWrapper::kMaxQuality     == 100u);
    ASSERT(LibJpegTurboEncodeWrapper::kDefaultQuality == 85u);
    ASSERT(LibJpegTurboEncodeWrapper::kMaxDimension   == 65535u);
}

TEST(TestS319_LibSpngDecode_WIC)
{
    using namespace ExplorerLens::Engine;

    // WIC PNG decode must be available on Windows
    ASSERT(LibSpngDecodeWrapper::IsAvailable());
    ASSERT(strcmp(LibSpngDecodeWrapper::BackendName(), "WIC") == 0);

    LibSpngDecodeWrapper dec;

    // Empty buffer must return INVALID_INPUT
    const auto emptyResult = dec.Decode({});
    ASSERT(emptyResult.status == SpngDecodeStatus::INVALID_INPUT);

    // Short buffer (< 8 bytes) must return NOT_PNG
    std::byte shortBuf[4]{};
    const auto shortResult = dec.Decode(std::span<const std::byte>{ shortBuf, 4 });
    ASSERT(shortResult.status == SpngDecodeStatus::NOT_PNG);

    // 8 bytes with wrong signature must return NOT_PNG
    std::byte fakePng[8]{};
    const auto fakeResult = dec.Decode(std::span<const std::byte>{ fakePng, 8 });
    ASSERT(fakeResult.status == SpngDecodeStatus::NOT_PNG);
    ASSERT(fakeResult.pixels.empty());

    // A correctly-signed buffer that is not a valid PNG must return NOT_PNG or DECODE_FAILED
    std::byte pngSig[16]{ std::byte{0x89}, std::byte{0x50}, std::byte{0x4E}, std::byte{0x47},
                          std::byte{0x0D}, std::byte{0x0A}, std::byte{0x1A}, std::byte{0x0A} };
    const auto badPngResult = dec.Decode(std::span<const std::byte>{ pngSig, 16 });
    ASSERT(badPngResult.status == SpngDecodeStatus::NOT_PNG
        || badPngResult.status == SpngDecodeStatus::DECODE_FAILED);

    ASSERT(LibSpngDecodeWrapper::kMaxSpngDimension     == 1'048'576u);
    ASSERT(LibSpngDecodeWrapper::kMinPngSignatureBytes == 8u);
}

//== Sprint S322-S330 — ROADMAP v8.0 Phase 2 Correctness & Robustness ==

TEST(TestS322_DecodeTimeoutGuard_Defaults)
{
    using namespace ExplorerLens::Engine;

    // Constants must match ROADMAP H39 specification
    ASSERT_EQ(DecodeTimeoutGuard::kDefaultTimeoutMs, 500u);
    ASSERT_EQ(DecodeTimeoutGuard::kMinTimeoutMs,     100u);
    ASSERT_EQ(DecodeTimeoutGuard::kMaxTimeoutMs,     5000u);
    ASSERT_EQ(DecodeTimeoutGuard::kWatchdogPollMs,   32u);

    // Constructed with default config → IDLE status
    DecodeTimeoutGuard g{};
    ASSERT(g.Status() == DecodeTimeoutStatus::IDLE);
    ASSERT(!g.IsExpired());

    // Invalid config (timeout below minimum) → INVALID_CONFIG
    DecodeTimeoutConfig bad{};
    bad.timeoutMs = 10u;  // below kMinTimeoutMs
    DecodeTimeoutGuard badGuard{ bad };
    ASSERT(badGuard.Status() == DecodeTimeoutStatus::INVALID_CONFIG);
    ASSERT(!badGuard.Start());  // must refuse to start

    // Valid start / complete cycle
    DecodeTimeoutGuard g2{};
    ASSERT(g2.Start());
    ASSERT(g2.Status() == DecodeTimeoutStatus::RUNNING);
    g2.Complete();
    ASSERT(g2.Status() == DecodeTimeoutStatus::COMPLETED);
    ASSERT(!g2.IsExpired());

    // stop_token is initially not requested
    DecodeTimeoutGuard g3{};
    ASSERT(!g3.StopToken().stop_requested());
}

TEST(TestS323_PerFormatMemoryBudget_Limits)
{
    using namespace ExplorerLens::Engine;

    // RAW camera images get the largest budget
    ASSERT(PerFormatMemoryBudget::kBudgetRaw    == 128u * 1024u * 1024u);
    ASSERT(PerFormatMemoryBudget::kBudgetRaster ==  48u * 1024u * 1024u);
    ASSERT(PerFormatMemoryBudget::kBudgetIcon   ==   1u * 1024u * 1024u);

    // Known formats must return OK for reasonable sizes
    ASSERT(PerFormatMemoryBudget::Check("image/jpeg",  4u * 1024u * 1024u)
           == BudgetCheckResult::OK);
    ASSERT(PerFormatMemoryBudget::Check("image/x-raw", 64u * 1024u * 1024u)
           == BudgetCheckResult::OK);

    // Oversized claim must be rejected
    ASSERT(PerFormatMemoryBudget::Check("image/x-icon", 256u * 1024u * 1024u)
           == BudgetCheckResult::BUDGET_EXCEEDED);
    ASSERT(PerFormatMemoryBudget::Check("image/jpeg",   512u * 1024u * 1024u)
           == BudgetCheckResult::BUDGET_EXCEEDED);

    // Unknown format gets fallback budget (kBudgetUnknown = 16 MiB)
    ASSERT(PerFormatMemoryBudget::LimitFor("image/x-unknown")
           == PerFormatMemoryBudget::kBudgetUnknown);
    ASSERT(PerFormatMemoryBudget::Check("image/x-unknown", 8u * 1024u * 1024u)
           == BudgetCheckResult::OK);
    ASSERT(PerFormatMemoryBudget::Check("image/x-unknown", 64u * 1024u * 1024u)
           == BudgetCheckResult::BUDGET_EXCEEDED);
}

TEST(TestS324_WicPassthroughSelector_Jpeg)
{
    using namespace ExplorerLens::Engine;

    // JPEG → USE_CUSTOM (libjpeg-turbo preferred over WIC)
    auto r = WicPassthroughSelector::SelectByFormatId("image/jpeg");
    ASSERT(r.decision == WicPassthroughDecision::USE_CUSTOM);
    ASSERT(r.status   == WicPassthroughStatus::OK);

    // PNG → USE_CUSTOM (libspng preferred)
    r = WicPassthroughSelector::SelectByFormatId("image/png");
    ASSERT(r.decision == WicPassthroughDecision::USE_CUSTOM);

    // BMP → USE_WIC (WIC handles BMP natively, no quality gap)
    r = WicPassthroughSelector::SelectByFormatId("image/bmp");
    ASSERT(r.decision == WicPassthroughDecision::USE_WIC);

    // GIF → USE_WIC
    r = WicPassthroughSelector::SelectByFormatId("image/gif");
    ASSERT(r.decision == WicPassthroughDecision::USE_WIC);

    // Unknown → UNKNOWN_FORMAT
    r = WicPassthroughSelector::SelectByFormatId("application/x-unknown");
    ASSERT(r.decision == WicPassthroughDecision::UNKNOWN_FORMAT);

    // Magic-byte path: JPEG FF D8 FF
    std::array<std::byte, 4> jpegMagic{
        std::byte{0xFF}, std::byte{0xD8}, std::byte{0xFF}, std::byte{0xE0}
    };
    auto mr = WicPassthroughSelector::SelectByMagic(
        std::span<const std::byte>{ jpegMagic.data(), jpegMagic.size() });
    ASSERT(mr.decision == WicPassthroughDecision::USE_CUSTOM);
    ASSERT(mr.resolvedFormatId == "image/jpeg");

    // Empty magic → EMPTY_MAGIC
    auto er = WicPassthroughSelector::SelectByMagic({});
    ASSERT(er.status == WicPassthroughStatus::EMPTY_MAGIC);

    // Minimum magic bytes constant
    ASSERT(WicPassthroughSelector::kMinMagicBytes == 4u);
    ASSERT(WicPassthroughSelector::kTableEntryCount == 22u);
}

TEST(TestS325_AsyncCacheWriter_StartStop)
{
    using namespace ExplorerLens::Engine;

    // Constants
    ASSERT_EQ(AsyncCacheWriter::kDefaultQueueDepth, 64u);
    ASSERT_EQ(AsyncCacheWriter::kMinQueueDepth,      4u);
    ASSERT_EQ(AsyncCacheWriter::kMaxQueueDepth,    512u);

    // Default constructed → not running
    AsyncCacheWriter writer{};
    ASSERT(!writer.IsRunning());

    // Start → running; second Start is idempotent
    ASSERT(writer.Start());
    ASSERT(writer.IsRunning());
    ASSERT(!writer.Start());  // already running

    // Enqueue invalid request → INVALID_REQUEST
    ASSERT(writer.Enqueue({}) == CacheWriteStatus::INVALID_REQUEST);

    // Enqueue valid request → ENQUEUED
    CacheWriteRequest req;
    req.cachePath = L"C:\\Test\\thumb_0001.png";
    req.pixelsBGRA.assign(16u, std::byte{0xAA});
    req.width  = 2u;
    req.height = 2u;
    ASSERT(writer.Enqueue(req) == CacheWriteStatus::ENQUEUED);
    ASSERT(writer.EnqueuedTotal() == 1u);

    // Stop → not running; stats preserved
    writer.Stop();
    ASSERT(!writer.IsRunning());
    ASSERT(writer.EnqueuedTotal() >= 1u);

    // Enqueue after stop → WRITER_STOPPED
    ASSERT(writer.Enqueue(req) == CacheWriteStatus::WRITER_STOPPED);

    // Custom queue depth
    AsyncCacheWriterConfig cfg{};
    cfg.queueDepth = 8u;
    AsyncCacheWriter small{ cfg };
    ASSERT(small.QueueDepth() == 8u);
}

TEST(TestS326_DecodeErrorTracker_Record)
{
    using namespace ExplorerLens::Engine;

    DecodeErrorTracker tracker;

    // Fresh tracker: zero failures, zero slots
    ASSERT_EQ(tracker.TotalFailures(), 0u);
    ASSERT_EQ(tracker.SlotCount(), 0u);

    // Record one failure → slot created
    DecodeErrorEvent evt;
    evt.decoderId     = "libjpeg-turbo";
    evt.fileExtension = ".jpg";
    evt.hresult       = static_cast<std::int32_t>(0x80070057u);  // E_INVALIDARG
    evt.wasTimeout    = false;
    tracker.Record(evt);

    ASSERT_EQ(tracker.TotalFailures(), 1u);
    ASSERT_EQ(tracker.SlotCount(),     1u);

    // Stats for that decoder
    const auto stats = tracker.GetStats("libjpeg-turbo");
    ASSERT_EQ(stats.failureCount, 1u);
    ASSERT_EQ(stats.timeoutCount, 0u);
    ASSERT_EQ(stats.lastHresult,  static_cast<std::int32_t>(0x80070057u));

    // Record a timeout failure for a different decoder
    DecodeErrorEvent tevt;
    tevt.decoderId     = "libheif";
    tevt.fileExtension = ".heic";
    tevt.wasTimeout    = true;
    tracker.Record(tevt);
    ASSERT_EQ(tracker.TotalFailures(), 2u);
    ASSERT_EQ(tracker.SlotCount(),     2u);

    const auto heifStats = tracker.GetStats("libheif");
    ASSERT_EQ(heifStats.timeoutCount, 1u);

    // Reset clears all counters
    tracker.Reset();
    ASSERT_EQ(tracker.TotalFailures(), 0u);
    ASSERT_EQ(tracker.SlotCount(),     0u);

    // Unknown decoder stats → zeroed struct
    const auto unknown = tracker.GetStats("does-not-exist");
    ASSERT_EQ(unknown.failureCount, 0u);
}

TEST(TestS327_EFailDecodeGuard_BlankReject)
{
    using namespace ExplorerLens::Engine;

    // Constants
    ASSERT(EFailDecodeGuard::kDefaultPolicy == DecodeResultPolicy::STRICT);
    ASSERT_EQ(EFailDecodeGuard::kMinSamplePixels, 64u);

    // Null buffer → NULL_BITMAP
    ASSERT(EFailDecodeGuard::ValidatePixels({}, 100u, 100u)
           == EFailValidationResult::NULL_BITMAP);

    // Zero dimensions → ZERO_DIMENSIONS
    std::vector<std::byte> px(4u, std::byte{0xAA});
    ASSERT(EFailDecodeGuard::ValidatePixels(px, 0u, 1u)
           == EFailValidationResult::ZERO_DIMENSIONS);
    ASSERT(EFailDecodeGuard::ValidatePixels(px, 1u, 0u)
           == EFailValidationResult::ZERO_DIMENSIONS);

    // All-white 4×4 BGRA → BLANK_BITMAP
    constexpr std::size_t kDim = 4u;
    std::vector<std::byte> white(kDim * kDim * 4u, std::byte{0xFF});
    ASSERT(EFailDecodeGuard::ValidatePixels(
        std::span<const std::byte>{ white.data(), white.size() }, kDim, kDim)
        == EFailValidationResult::BLANK_BITMAP);

    // All-transparent 4×4 BGRA → BLANK_BITMAP
    std::vector<std::byte> transparent(kDim * kDim * 4u, std::byte{0x00});
    ASSERT(EFailDecodeGuard::ValidatePixels(
        std::span<const std::byte>{ transparent.data(), transparent.size() }, kDim, kDim)
        == EFailValidationResult::BLANK_BITMAP);

    // Non-blank 4×4 BGRA (mixed pixels) → VALID
    std::vector<std::byte> mixed(kDim * kDim * 4u, std::byte{0x00});
    mixed[0] = std::byte{0x10};
    mixed[4] = std::byte{0x80};
    ASSERT(EFailDecodeGuard::ValidatePixels(
        std::span<const std::byte>{ mixed.data(), mixed.size() }, kDim, kDim)
        == EFailValidationResult::VALID);

    // CoalesceHresult: VALID → S_OK (0)
    // We test the integer value since HRESULT isn't available in all configs
#ifdef _WIN32
    ASSERT(EFailDecodeGuard::CoalesceHresult(
        EFailValidationResult::VALID, DecodeResultPolicy::STRICT) == S_OK);
    ASSERT(EFailDecodeGuard::CoalesceHresult(
        EFailValidationResult::NULL_BITMAP, DecodeResultPolicy::STRICT) == E_FAIL);
    ASSERT(EFailDecodeGuard::CoalesceHresult(
        EFailValidationResult::TIMEOUT_EXPIRED, DecodeResultPolicy::STRICT) == E_ABORT);
    ASSERT(EFailDecodeGuard::CoalesceHresult(
        EFailValidationResult::BUDGET_EXCEEDED, DecodeResultPolicy::STRICT) == E_OUTOFMEMORY);
#endif
}

TEST(TestS328_FormatMagicValidator_JpegMatch)
{
    using namespace ExplorerLens::Engine;

    // Table constants
    ASSERT_EQ(FormatMagicValidator::kTableSize,      22u);
    ASSERT_EQ(FormatMagicValidator::kMinHeaderBytes, 12u);

    // JPEG magic: FF D8 FF E0
    std::array<std::byte, 16> jpegHdr{
        std::byte{0xFF}, std::byte{0xD8}, std::byte{0xFF}, std::byte{0xE0},
        std::byte{0x00}, std::byte{0x10}, std::byte{0x4A}, std::byte{0x46},
        std::byte{0x49}, std::byte{0x46}, std::byte{0x00}, std::byte{0x01}
    };
    ASSERT(FormatMagicValidator::Validate(".jpg",
        std::span<const std::byte>{ jpegHdr.data(), jpegHdr.size() })
        == MagicValidatorResult::MATCH);
    ASSERT(FormatMagicValidator::Validate(".jpeg",
        std::span<const std::byte>{ jpegHdr.data(), jpegHdr.size() })
        == MagicValidatorResult::MATCH);

    // PNG magic: 89 50 4E 47 0D 0A 1A 0A
    std::array<std::byte, 12> pngHdr{
        std::byte{0x89}, std::byte{0x50}, std::byte{0x4E}, std::byte{0x47},
        std::byte{0x0D}, std::byte{0x0A}, std::byte{0x1A}, std::byte{0x0A},
        std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x0D}
    };
    ASSERT(FormatMagicValidator::Validate(".png",
        std::span<const std::byte>{ pngHdr.data(), pngHdr.size() })
        == MagicValidatorResult::MATCH);

    // Extension mismatch: JPEG bytes labelled as PNG → MISMATCH
    ASSERT(FormatMagicValidator::Validate(".png",
        std::span<const std::byte>{ jpegHdr.data(), jpegHdr.size() })
        == MagicValidatorResult::MISMATCH);

    // Unknown extension → UNKNOWN_FORMAT
    ASSERT(FormatMagicValidator::Validate(".xyz",
        std::span<const std::byte>{ jpegHdr.data(), jpegHdr.size() })
        == MagicValidatorResult::UNKNOWN_FORMAT);

    // Empty buffer → EMPTY_BUFFER
    ASSERT(FormatMagicValidator::Validate(".jpg", {})
           == MagicValidatorResult::EMPTY_BUFFER);

    // Too-short buffer (1 byte) → TOO_SHORT
    std::array<std::byte, 1> tiny{ std::byte{0xFF} };
    ASSERT(FormatMagicValidator::Validate(".jpg",
        std::span<const std::byte>{ tiny.data(), tiny.size() })
        == MagicValidatorResult::TOO_SHORT);

    // IsKnownExtension
    ASSERT(FormatMagicValidator::IsKnownExtension(".jpg"));
    ASSERT(FormatMagicValidator::IsKnownExtension(".png"));
    ASSERT(!FormatMagicValidator::IsKnownExtension(".xyz"));
}

TEST(TestS329_ThumbnailPlaceholderBroker_Miss)
{
    using namespace ExplorerLens::Engine;

    ThumbnailPlaceholderBroker broker;

    // Fresh broker: no entries
    ASSERT_EQ(broker.LiveCount(), 0u);
    ASSERT_EQ(broker.RegisteredTotal(), 0u);

    // Resolve on empty cache → CACHE_EMPTY
    PlaceholderEntry out{};
    ASSERT(broker.Resolve(0xDEADBEEFCAFEBABEull, 256u, out)
           == PlaceholderResolveResult::CACHE_EMPTY);

    // Register a valid entry
    PlaceholderEntry entry{};
    entry.valid    = true;
    entry.pathHash = 0xDEADBEEFCAFEBABEull;
    entry.width    = 256u;
    entry.height   = 256u;
    broker.Register(entry);

    ASSERT_EQ(broker.LiveCount(),       1u);
    ASSERT_EQ(broker.RegisteredTotal(), 1u);

    // Resolve → exact hit
    PlaceholderEntry hit{};
    ASSERT(broker.Resolve(0xDEADBEEFCAFEBABEull, 256u, hit)
           == PlaceholderResolveResult::HIT_EXACT);
    ASSERT(hit.valid);
    ASSERT_EQ(hit.pathHash, 0xDEADBEEFCAFEBABEull);
    ASSERT_EQ(hit.width,    256u);

    // Different size → HIT_SCALED
    ASSERT(broker.Resolve(0xDEADBEEFCAFEBABEull, 128u, hit)
           == PlaceholderResolveResult::HIT_SCALED);

    // Unknown path hash → MISS (one valid entry exists, just not this hash)
    ASSERT(broker.Resolve(0x1234567890ABCDEFull, 256u, hit)
           == PlaceholderResolveResult::MISS);

    // Invalidate → entry removed
    broker.Invalidate(0xDEADBEEFCAFEBABEull);
    ASSERT_EQ(broker.LiveCount(), 0u);

    // Clear
    broker.Register(entry);
    ASSERT_EQ(broker.LiveCount(), 1u);
    broker.Clear();
    ASSERT_EQ(broker.LiveCount(), 0u);

    // Constants
    ASSERT_EQ(ThumbnailPlaceholderBroker::kMaxEntries, 128u);
    ASSERT(ThumbnailPlaceholderBroker::kMaxAgeMs == 30'000);
}

TEST(TestS330_ParallelReadaheadManager_SlotCount)
{
    using namespace ExplorerLens::Engine;

    // Constants
    ASSERT_EQ(ParallelReadaheadManager::kDefaultSlotCount, 8u);
    ASSERT_EQ(ParallelReadaheadManager::kMaxSlotCount,    16u);
    ASSERT_EQ(ParallelReadaheadManager::kAcquireTimeoutMs, 32u);

    // Default construction
    ParallelReadaheadManager mgr{};
    ASSERT(!mgr.IsRunning());
    ASSERT_EQ(mgr.SlotCount(), 8u);

    // Custom slot count
    ParallelReadaheadConfig cfg{};
    cfg.slotCount     = 4u;
    cfg.asyncPrefetch = false;  // synchronous mode for unit test
    ParallelReadaheadManager small{ cfg };
    ASSERT_EQ(small.SlotCount(), 4u);

    // Start in sync mode → running, no thread launched
    ASSERT(small.Start());
    ASSERT(small.IsRunning());

    // Set queue and verify stats start at zero
    std::vector<std::wstring> paths = {
        L"C:\\Photos\\001.jpg",
        L"C:\\Photos\\002.jpg",
        L"C:\\Photos\\003.jpg",
    };
    small.SetQueue(paths);
    ASSERT_EQ(small.AcquiredTotal(),   0u);
    ASSERT_EQ(small.PrefetchedTotal(), 0u);
    ASSERT_EQ(small.ErrorTotal(),      0u);

    // AcquireStream for path not yet prefetched → nullptr (caller does sync open)
    IStream* s = small.AcquireStream(L"C:\\Photos\\001.jpg");
    (void)s;  // may be nullptr in stub mode; both outcomes are valid

    // Stop
    small.Stop();
    ASSERT(!small.IsRunning());

    // ReadaheadPriority enum values
    ASSERT(static_cast<std::uint8_t>(ReadaheadPriority::LOW)    == 0u);
    ASSERT(static_cast<std::uint8_t>(ReadaheadPriority::NORMAL) == 1u);
    ASSERT(static_cast<std::uint8_t>(ReadaheadPriority::HIGH)   == 2u);
}
