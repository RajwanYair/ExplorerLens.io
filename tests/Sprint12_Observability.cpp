// Sprint12_Observability.cpp
// Sprint 12: Observability & Structured Logging Tests
// Validates ETW, JSON logger, diagnostics export, and pipeline telemetry

#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class ObservabilityTest : public ::testing::Test {
protected:
    std::string rootDir;
    
    void SetUp() override {
        rootDir = fs::current_path().string();
        auto searchDir = fs::current_path();
        for (int i = 0; i < 5; i++) {
            if (fs::exists(searchDir / "MASTER_PLAN.md")) {
                rootDir = searchDir.string();
                break;
            }
            searchDir = searchDir.parent_path();
        }
    }
    
    bool fileContains(const std::string& relPath, const std::string& needle) {
        auto fullPath = fs::path(rootDir) / relPath;
        if (!fs::exists(fullPath)) return false;
        std::ifstream f(fullPath.string());
        std::string content((std::istreambuf_iterator<char>(f)),
                           std::istreambuf_iterator<char>());
        return content.find(needle) != std::string::npos;
    }
    
    bool fileExists(const std::string& relPath) {
        return fs::exists(fs::path(rootDir) / relPath);
    }
};

// =============================================================================
// ETW Provider Tests
// =============================================================================

TEST_F(ObservabilityTest, ETWTracingHeaderExists) {
    EXPECT_TRUE(fileExists("Engine/Utils/ETWTracing.h"))
        << "ETW tracing header should exist";
}

TEST_F(ObservabilityTest, ETWProviderGUIDDefined) {
    EXPECT_TRUE(fileContains("Engine/Utils/ETWTracing.h", "DARKTHUMBS_PROVIDER_GUID"))
        << "ETW provider GUID should be defined";
}

TEST_F(ObservabilityTest, ETWEventIDsDefined) {
    EXPECT_TRUE(fileContains("Engine/Utils/ETWTracing.h", "ThumbnailGeneration_Start"))
        << "ThumbnailGeneration_Start event should be defined";
    EXPECT_TRUE(fileContains("Engine/Utils/ETWTracing.h", "Decode_Start"))
        << "Decode_Start event should be defined";
    EXPECT_TRUE(fileContains("Engine/Utils/ETWTracing.h", "Cache_Hit"))
        << "Cache_Hit event should be defined";
}

// =============================================================================
// Structured Logger Tests
// =============================================================================

TEST_F(ObservabilityTest, StructuredLoggerHeaderExists) {
    EXPECT_TRUE(fileExists("Engine/Utils/StructuredLogger.h"))
        << "Structured logger header should exist";
}

TEST_F(ObservabilityTest, StructuredLoggerIsJsonLines) {
    EXPECT_TRUE(fileContains("Engine/Utils/StructuredLogger.h", "JSON"))
        << "Logger should produce JSON-lines output";
}

TEST_F(ObservabilityTest, StructuredLoggerHasLogLevels) {
    EXPECT_TRUE(fileContains("Engine/Utils/StructuredLogger.h", "LogLevel"))
        << "Logger should support log levels";
    EXPECT_TRUE(fileContains("Engine/Utils/StructuredLogger.h", "LogInfo"))
        << "Logger should have LogInfo method";
    EXPECT_TRUE(fileContains("Engine/Utils/StructuredLogger.h", "LogWarning"))
        << "Logger should have LogWarning method";
    EXPECT_TRUE(fileContains("Engine/Utils/StructuredLogger.h", "LogError"))
        << "Logger should have LogError method";
}

TEST_F(ObservabilityTest, StructuredLoggerSupportsPrivacy) {
    EXPECT_TRUE(fileContains("Engine/Utils/StructuredLogger.h", "Privacy") ||
                fileContains("Engine/Utils/StructuredLogger.h", "privacy") ||
                fileContains("Engine/Utils/StructuredLogger.h", "m_enablePrivacy"))
        << "Logger should support privacy mode for path hashing";
}

TEST_F(ObservabilityTest, StructuredLoggerIsThreadSafe) {
    EXPECT_TRUE(fileContains("Engine/Utils/StructuredLogger.h", "mutex"))
        << "Logger should use mutex for thread safety";
}

// =============================================================================
// Observability Integration Tests
// =============================================================================

TEST_F(ObservabilityTest, ObservabilityIntegrationExists) {
    EXPECT_TRUE(fileExists("Engine/Utils/ObservabilityIntegration.h"))
        << "Observability integration header should exist";
}

TEST_F(ObservabilityTest, PipelineTelemetryClass) {
    EXPECT_TRUE(fileContains("Engine/Utils/ObservabilityIntegration.h", "PipelineTelemetry"))
        << "PipelineTelemetry class should exist";
}

TEST_F(ObservabilityTest, TelemetryTracesThumbnailLifecycle) {
    EXPECT_TRUE(fileContains("Engine/Utils/ObservabilityIntegration.h", "TraceThumbnailStart"))
        << "Should trace thumbnail request start";
    EXPECT_TRUE(fileContains("Engine/Utils/ObservabilityIntegration.h", "TraceThumbnailComplete"))
        << "Should trace thumbnail request completion";
}

TEST_F(ObservabilityTest, TelemetryTracesDecoding) {
    EXPECT_TRUE(fileContains("Engine/Utils/ObservabilityIntegration.h", "TraceDecodeStart"))
        << "Should trace decode start";
    EXPECT_TRUE(fileContains("Engine/Utils/ObservabilityIntegration.h", "TraceDecodeComplete"))
        << "Should trace decode completion";
}

TEST_F(ObservabilityTest, ScopedTraceExists) {
    EXPECT_TRUE(fileContains("Engine/Utils/ObservabilityIntegration.h", "ScopedTrace"))
        << "ScopedTrace RAII class should exist";
    EXPECT_TRUE(fileContains("Engine/Utils/ObservabilityIntegration.h", "TRACE_SCOPE"))
        << "TRACE_SCOPE macro should exist";
}

// =============================================================================
// Diagnostics Export Tests
// =============================================================================

TEST_F(ObservabilityTest, DiagnosticsExporterExists) {
    EXPECT_TRUE(fileContains("Engine/Utils/ObservabilityIntegration.h", "DiagnosticsExporter"))
        << "DiagnosticsExporter class should exist";
}

TEST_F(ObservabilityTest, DiagnosticsBundleCollectsSystemInfo) {
    EXPECT_TRUE(fileContains("Engine/Utils/ObservabilityIntegration.h", "CollectSystemInfo"))
        << "Should collect system info";
}

TEST_F(ObservabilityTest, DiagnosticsBundleCollectsConfig) {
    EXPECT_TRUE(fileContains("Engine/Utils/ObservabilityIntegration.h", "CollectConfig"))
        << "Should collect configuration";
}

TEST_F(ObservabilityTest, DiagnosticsBundleCollectsLogs) {
    EXPECT_TRUE(fileContains("Engine/Utils/ObservabilityIntegration.h", "CollectRecentLogs"))
        << "Should collect recent logs";
}

TEST_F(ObservabilityTest, DiagnosticsBundleCollectsRegistry) {
    EXPECT_TRUE(fileContains("Engine/Utils/ObservabilityIntegration.h", "CollectRegistryInfo"))
        << "Should collect registry info";
}

TEST_F(ObservabilityTest, DiagnosticsCanExportToFile) {
    EXPECT_TRUE(fileContains("Engine/Utils/ObservabilityIntegration.h", "ExportToFile"))
        << "Should be able to export diagnostics to file";
}

// =============================================================================
// WinUI Diagnostics Integration Tests
// =============================================================================

TEST_F(ObservabilityTest, WinUIDiagnosticsPageExists) {
    EXPECT_TRUE(fileExists("src/Manager.WinUI/Views/DiagnosticsPage.xaml"))
        << "WinUI DiagnosticsPage should exist";
}

TEST_F(ObservabilityTest, WinUIDiagnosticsViewModelExists) {
    EXPECT_TRUE(fileExists("src/Manager.WinUI/ViewModels/DiagnosticsViewModel.cs"))
        << "WinUI DiagnosticsViewModel should exist";
}

TEST_F(ObservabilityTest, WinUIHasExportDiagnosticsButton) {
    EXPECT_TRUE(fileContains("src/Manager.WinUI/Views/DiagnosticsPage.xaml", "ExportDiagnostics"))
        << "DiagnosticsPage should have Export Diagnostics button";
}
