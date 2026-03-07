// CrashDumpAnalyzer.h — Minidump Post-Mortem Analysis
// Copyright (c) 2026 ExplorerLens Project
//
// Analyzes crash minidumps to extract stack traces, exception codes,
// and module information for automated crash classification.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class CrashDumpCategory : uint8_t {
    Unknown = 0,
    AccessViolation = 1,
    StackOverflow = 2,
    HeapCorruption = 3,
    DivideByZero = 4,
    PureVirtualCall = 5,
    UnhandledException = 6,
    GPUDeviceLost = 7,
    OutOfMemory = 8,
    DecoderTimeout = 9
};

struct CrashFrame {
    std::string moduleName;
    std::string functionName;
    uint64_t address = 0;
    uint32_t lineNumber = 0;
    std::string sourceFile;
};

struct CrashDumpInfo {
    std::wstring dumpPath;
    uint32_t exceptionCode = 0;
    uint64_t exceptionAddress = 0;
    CrashDumpCategory category = CrashDumpCategory::Unknown;
    std::vector<CrashFrame> stackTrace;
    std::string faultingModule;
    uint64_t processUptimeMs = 0;
    uint64_t memoryUsedBytes = 0;
    uint32_t threadCount = 0;
    std::string osVersion;
};

struct CrashClassification {
    CrashDumpCategory category = CrashDumpCategory::Unknown;
    std::string summary;
    std::string suggestedAction;
    bool isKnownIssue = false;
    float confidence = 0.0f;
};

class CrashDumpAnalyzer {
public:
    CrashDumpCategory ClassifyException(uint32_t exceptionCode) const {
        switch (exceptionCode) {
        case 0xC0000005: return CrashDumpCategory::AccessViolation;
        case 0xC00000FD: return CrashDumpCategory::StackOverflow;
        case 0xC0000374: return CrashDumpCategory::HeapCorruption;
        case 0xC0000094: return CrashDumpCategory::DivideByZero;
        case 0xC000001D: return CrashDumpCategory::UnhandledException;
        default: return CrashDumpCategory::Unknown;
        }
    }

    CrashClassification Classify(const CrashDumpInfo& info) const {
        CrashClassification result;
        result.category = ClassifyException(info.exceptionCode);

        if (!info.stackTrace.empty()) {
            result.summary = "Crash in " + info.faultingModule + " at " +
                info.stackTrace[0].functionName;
        }

        switch (result.category) {
        case CrashDumpCategory::AccessViolation:
            result.suggestedAction = "Check null pointer / buffer overrun";
            result.confidence = 0.8f;
            break;
        case CrashDumpCategory::StackOverflow:
            result.suggestedAction = "Check recursive calls / large stack allocations";
            result.confidence = 0.95f;
            break;
        case CrashDumpCategory::HeapCorruption:
            result.suggestedAction = "Enable page heap / check buffer writes";
            result.confidence = 0.7f;
            break;
        default:
            result.suggestedAction = "Analyze minidump with debugger";
            result.confidence = 0.3f;
            break;
        }
        return result;
    }

    bool IsDecoderCrash(const CrashDumpInfo& info) const {
        for (const auto& frame : info.stackTrace) {
            if (frame.moduleName.find("Decoder") != std::string::npos ||
                frame.functionName.find("Decode") != std::string::npos)
                return true;
        }
        return false;
    }
};

} // namespace Engine
} // namespace ExplorerLens
