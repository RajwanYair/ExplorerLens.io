//==============================================================================
// ExplorerLens — Observability & Structured Logging
// Tests ETW events, JSON-lines logger, privacy filters, request tracing,
// diagnostics bundle building, and log level filtering.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>

// Header under test
#include "../Engine/Utils/Observability.h"

using namespace ExplorerLens::Engine::Observability;

//==============================================================================
// ETW Event Tests
//==============================================================================

TEST(ETWEvents, EventNames)
{
    EXPECT_STREQ(ETWEventName(ETWEventId::RequestStart), "RequestStart");
    EXPECT_STREQ(ETWEventName(ETWEventId::RequestStop), "RequestStop");
    EXPECT_STREQ(ETWEventName(ETWEventId::CacheHit), "CacheHit");
    EXPECT_STREQ(ETWEventName(ETWEventId::CacheMiss), "CacheMiss");
    EXPECT_STREQ(ETWEventName(ETWEventId::DecodeStart), "DecodeStart");
    EXPECT_STREQ(ETWEventName(ETWEventId::DecodeFail), "DecodeFail");
    EXPECT_STREQ(ETWEventName(ETWEventId::CrashCaught), "CrashCaught");
    EXPECT_STREQ(ETWEventName(ETWEventId::GPUFallback), "GPUFallback");
}

TEST(ETWEvents, ProviderConfig)
{
    ETWProviderConfig config;
    EXPECT_STREQ(config.ProviderName, "ExplorerLens-Engine-Core");
    EXPECT_TRUE(config.enabled);
    EXPECT_EQ(ETWProviderConfig::TotalEventTypes, 15u);
}

TEST(ETWEvents, ProviderGUID)
{
    std::string guid = ETWProviderConfig::ProviderGUID;
    EXPECT_NE(guid.find("{"), std::string::npos);
    EXPECT_NE(guid.find("}"), std::string::npos);
    EXPECT_GT(guid.size(), 30u);
}

TEST(ETWEvents, EventIdValues)
{
    // Verify event ID ranges per OBSERVABILITY_SPEC_V1.md
    EXPECT_GE(static_cast<uint16_t>(ETWEventId::RequestStart), 100u);
    EXPECT_GE(static_cast<uint16_t>(ETWEventId::CacheHit), 200u);
    EXPECT_GE(static_cast<uint16_t>(ETWEventId::DecodeStart), 300u);
    EXPECT_GE(static_cast<uint16_t>(ETWEventId::CrashCaught), 400u);
    EXPECT_GE(static_cast<uint16_t>(ETWEventId::PluginLoad), 500u);
    EXPECT_GE(static_cast<uint16_t>(ETWEventId::MemoryPressure), 600u);
}

//==============================================================================
// Log Level Tests
//==============================================================================

TEST(LogLevel, LevelNames)
{
    EXPECT_STREQ(LogLevelName(LogLevel::Trace), "TRACE");
    EXPECT_STREQ(LogLevelName(LogLevel::Debug), "DEBUG");
    EXPECT_STREQ(LogLevelName(LogLevel::Info), "INFO");
    EXPECT_STREQ(LogLevelName(LogLevel::Warning), "WARN");
    EXPECT_STREQ(LogLevelName(LogLevel::Error), "ERROR");
    EXPECT_STREQ(LogLevelName(LogLevel::Critical), "CRITICAL");
}

//==============================================================================
// Log Entry Tests
//==============================================================================

TEST(LogEntry, JSONFormat)
{
    LogEntry entry;
    entry.timestamp = "2025-07-11T12:00:00Z";
    entry.level = LogLevel::Info;
    entry.component = "Decoder";
    entry.message = "JPEG decode complete";
    entry.requestId = 42;

    auto json = entry.ToJSON();
    EXPECT_NE(json.find("\"ts\":\"2025-07-11"), std::string::npos);
    EXPECT_NE(json.find("\"level\":\"INFO\""), std::string::npos);
    EXPECT_NE(json.find("\"component\":\"Decoder\""), std::string::npos);
    EXPECT_NE(json.find("\"requestId\":42"), std::string::npos);
}

TEST(LogEntry, TextFormat)
{
    LogEntry entry;
    entry.timestamp = "2025-07-11";
    entry.level = LogLevel::Error;
    entry.component = "Cache";
    entry.message = "Cache miss for file";

    auto text = entry.ToText();
    EXPECT_NE(text.find("ERROR"), std::string::npos);
    EXPECT_NE(text.find("[Cache]"), std::string::npos);
    EXPECT_NE(text.find("Cache miss"), std::string::npos);
}

TEST(LogEntry, CustomFields)
{
    LogEntry entry;
    entry.timestamp = "now";
    entry.component = "GPU";
    entry.message = "fallback";
    entry.fields["gpu"] = "Intel UHD";
    entry.fields["reason"] = "timeout";

    auto json = entry.ToJSON();
    EXPECT_NE(json.find("\"gpu\":\"Intel UHD\""), std::string::npos);
    EXPECT_NE(json.find("\"reason\":\"timeout\""), std::string::npos);
}

//==============================================================================
// JSON-Lines Logger Tests
//==============================================================================

TEST(JSONLogger, EmptyByDefault)
{
    JSONLinesLogger logger;
    EXPECT_EQ(logger.EntryCount(), 0u);
    EXPECT_EQ(logger.ErrorCount(), 0u);
}

TEST(JSONLogger, LogEntry)
{
    JSONLinesLogger logger;
    LogEntry e;
    e.timestamp = "now"; e.level = LogLevel::Info;
    e.component = "Test"; e.message = "Hello";
    logger.Log(e);
    EXPECT_EQ(logger.EntryCount(), 1u);
}

TEST(JSONLogger, MinLevelFilter)
{
    JSONLinesLogger logger;
    logger.SetMinLevel(LogLevel::Warning);

    LogEntry info{};
    info.level = LogLevel::Info;
    info.timestamp = "now"; info.component = "T"; info.message = "filtered";
    logger.Log(info);

    LogEntry warn{};
    warn.level = LogLevel::Warning;
    warn.timestamp = "now"; warn.component = "T"; warn.message = "kept";
    logger.Log(warn);

    EXPECT_EQ(logger.EntryCount(), 1u);  // Only warning kept
}

TEST(JSONLogger, FlushJSON)
{
    JSONLinesLogger logger;
    LogEntry e;
    e.timestamp = "t1"; e.level = LogLevel::Info;
    e.component = "A"; e.message = "msg1";
    logger.Log(e);
    e.timestamp = "t2"; e.message = "msg2";
    logger.Log(e);

    auto output = logger.Flush();
    // Should have 2 JSON lines
    size_t newlines = std::count(output.begin(), output.end(), '\n');
    EXPECT_EQ(newlines, 2u);
}

TEST(JSONLogger, FilterByLevel)
{
    JSONLinesLogger logger;
    LogEntry e;
    e.timestamp = "now"; e.component = "T";

    e.level = LogLevel::Info; e.message = "info"; logger.Log(e);
    e.level = LogLevel::Error; e.message = "err"; logger.Log(e);
    e.level = LogLevel::Info; e.message = "info2"; logger.Log(e);

    auto errors = logger.GetByLevel(LogLevel::Error);
    EXPECT_EQ(errors.size(), 1u);
}

TEST(JSONLogger, FilterByComponent)
{
    JSONLinesLogger logger;
    LogEntry e;
    e.timestamp = "now"; e.level = LogLevel::Info;

    e.component = "Decoder"; e.message = "d1"; logger.Log(e);
    e.component = "Cache"; e.message = "c1"; logger.Log(e);
    e.component = "Decoder"; e.message = "d2"; logger.Log(e);

    auto decoder = logger.GetByComponent("Decoder");
    EXPECT_EQ(decoder.size(), 2u);
}

TEST(JSONLogger, ErrorAndWarningCount)
{
    JSONLinesLogger logger;
    LogEntry e;
    e.timestamp = "now"; e.component = "T";

    e.level = LogLevel::Error; e.message = "e1"; logger.Log(e);
    e.level = LogLevel::Critical; e.message = "c1"; logger.Log(e);
    e.level = LogLevel::Warning; e.message = "w1"; logger.Log(e);
    e.level = LogLevel::Info; e.message = "i1"; logger.Log(e);

    EXPECT_EQ(logger.ErrorCount(), 2u);    // Error + Critical
    EXPECT_EQ(logger.WarningCount(), 1u);
}

TEST(JSONLogger, Clear)
{
    JSONLinesLogger logger;
    LogEntry e; e.timestamp = "now"; e.level = LogLevel::Info;
    e.component = "T"; e.message = "test";
    logger.Log(e);
    logger.Clear();
    EXPECT_EQ(logger.EntryCount(), 0u);
}

//==============================================================================
// Privacy Filter Tests
//==============================================================================

TEST(Privacy, HashMode)
{
    PrivacyFilter filter(PrivacyFilter::Mode::Hash);
    auto hashed = filter.FilterPath("C:\\Users\\ryair\\Documents\\photo.jpg");
    EXPECT_NE(hashed.find("path:"), std::string::npos);
    EXPECT_EQ(hashed.find("ryair"), std::string::npos);  // No username
}

TEST(Privacy, VerboseMode)
{
    PrivacyFilter filter(PrivacyFilter::Mode::Verbose);
    std::string path = "C:\\Users\\test\\photo.jpg";
    EXPECT_EQ(filter.FilterPath(path), path);  // Full path preserved
}

TEST(Privacy, DeterministicHash)
{
    PrivacyFilter filter;
    auto h1 = filter.FilterPath("C:\\test\\file.jpg");
    auto h2 = filter.FilterPath("C:\\test\\file.jpg");
    EXPECT_EQ(h1, h2);  // Same input → same hash
}

TEST(Privacy, DifferentHashForDifferentPaths)
{
    PrivacyFilter filter;
    auto h1 = filter.FilterPath("C:\\a\\file.jpg");
    auto h2 = filter.FilterPath("C:\\b\\file.jpg");
    EXPECT_NE(h1, h2);
}

TEST(Privacy, FilenamExtraction)
{
    PrivacyFilter filter;
    EXPECT_EQ(filter.FilterToFilename("C:\\Users\\test\\photo.jpg"), "photo.jpg");
    EXPECT_EQ(filter.FilterToFilename("photo.jpg"), "photo.jpg");
}

TEST(Privacy, ModeSwitch)
{
    PrivacyFilter filter;
    EXPECT_FALSE(filter.IsVerbose());
    filter.SetMode(PrivacyFilter::Mode::Verbose);
    EXPECT_TRUE(filter.IsVerbose());
}

//==============================================================================
// Request Trace Tests
//==============================================================================

TEST(RequestTrace, PipelineMs)
{
    RequestTrace t;
    t.detectMs = 1.0;
    t.decodeMs = 10.0;
    t.resizeMs = 2.0;
    t.cacheMs = 0.5;
    t.marshalMs = 0.5;
    EXPECT_DOUBLE_EQ(t.PipelineMs(), 14.0);
}

TEST(RequestTrace, JSONOutput)
{
    RequestTrace t;
    t.requestId = 99;
    t.format = "JPEG";
    t.decoder = "WIC";
    t.totalMs = 15.5;
    t.cacheHit = true;
    auto json = t.ToJSON();
    EXPECT_NE(json.find("\"requestId\":99"), std::string::npos);
    EXPECT_NE(json.find("\"format\":\"JPEG\""), std::string::npos);
    EXPECT_NE(json.find("\"cacheHit\":true"), std::string::npos);
}

//==============================================================================
// Request Tracer Tests
//==============================================================================

TEST(RequestTracer, EmptyStats)
{
    RequestTracer tracer;
    EXPECT_DOUBLE_EQ(tracer.AverageLatencyMs(), 0.0);
    EXPECT_DOUBLE_EQ(tracer.CacheHitRate(), 0.0);
    EXPECT_DOUBLE_EQ(tracer.SuccessRate(), 100.0);
}

TEST(RequestTracer, RecordAndStats)
{
    RequestTracer tracer;
    RequestTrace t;
    t.totalMs = 10.0; t.succeeded = true; t.cacheHit = false;
    tracer.RecordTrace(t);
    t.totalMs = 20.0; t.cacheHit = true;
    tracer.RecordTrace(t);

    EXPECT_EQ(tracer.TraceCount(), 2u);
    EXPECT_DOUBLE_EQ(tracer.AverageLatencyMs(), 15.0);
    EXPECT_DOUBLE_EQ(tracer.CacheHitRate(), 50.0);
    EXPECT_DOUBLE_EQ(tracer.SuccessRate(), 100.0);
}

TEST(RequestTracer, ErrorTracking)
{
    RequestTracer tracer;
    RequestTrace ok; ok.totalMs = 5.0; ok.succeeded = true;
    RequestTrace fail; fail.totalMs = 100.0; fail.succeeded = false;
    tracer.RecordTrace(ok);
    tracer.RecordTrace(ok);
    tracer.RecordTrace(fail);
    EXPECT_EQ(tracer.ErrorCount(), 1u);
    EXPECT_GT(tracer.SuccessRate(), 66.0);
    EXPECT_LT(tracer.SuccessRate(), 67.0);
}

TEST(RequestTracer, P95Latency)
{
    RequestTracer tracer;
    for (int i = 0; i < 100; ++i) {
        RequestTrace t;
        t.totalMs = static_cast<double>(i + 1);
        t.succeeded = true;
        tracer.RecordTrace(t);
    }
    EXPECT_GE(tracer.P95LatencyMs(), 95.0);
    EXPECT_LE(tracer.P95LatencyMs(), 96.0);
}

//==============================================================================
// Diagnostic Bundle Tests
//==============================================================================

TEST(DiagBundle, SystemInfo)
{
    SystemInfo info;
    EXPECT_EQ(info.explorerLensVersion, "7.0.0");
    auto json = info.ToJSON();
    EXPECT_NE(json.find("\"version\":\"7.0.0\""), std::string::npos);
}

TEST(DiagBundle, SectionCount)
{
    DiagnosticBundle bundle;
    EXPECT_EQ(bundle.SectionCount(), 1u);  // system info only
    bundle.decoderStatus = "All stable";
    EXPECT_EQ(bundle.SectionCount(), 2u);
    bundle.pluginStatus = "None loaded";
    EXPECT_EQ(bundle.SectionCount(), 3u);
}

TEST(DiagBundle, Builder)
{
    DiagnosticBundleBuilder builder;
    SystemInfo info;
    info.cpuModel = "i9-14900K";
    info.gpuModel = "RTX 4090";
    info.ramMB = 65536;
    builder.SetSystemInfo(info);

    LogEntry log;
    log.timestamp = "now"; log.level = LogLevel::Info;
    log.component = "Test"; log.message = "Ready";
    builder.AddLog(log);

    RequestTrace trace;
    trace.requestId = 1; trace.totalMs = 10.0;
    builder.AddTrace(trace);

    builder.SetDecoderStatus("24 decoders active");

    EXPECT_EQ(builder.LogCount(), 1u);
    EXPECT_EQ(builder.TraceCount(), 1u);
    EXPECT_GE(builder.SectionCount(), 3u);
}

TEST(DiagBundle, Report)
{
    DiagnosticBundleBuilder builder;
    builder.SetDecoderStatus("JPEG: Stable");
    auto& bundle = builder.Build();
    auto report = bundle.GenerateReport();
    EXPECT_NE(report.find("Diagnostic Report"), std::string::npos);
    EXPECT_NE(report.find("JPEG: Stable"), std::string::npos);
}
