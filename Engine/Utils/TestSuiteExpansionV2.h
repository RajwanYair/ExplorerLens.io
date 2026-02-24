//==============================================================================
// ExplorerLens Engine — Test Suite Expansion V2
// 100+ real-file decoder tests, COM IThumbnailProvider integration testing,
// test corpus management, and automated test generation.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

/// Test file category V2
enum class TestFileCategoryV2 : uint8_t {
    ArchiveFormats,
    ModernImages,
    ProfessionalImages,
    CameraRAW,
    VectorGraphics,
    Documents,
    Fonts,
    Models3D,
    EBooks,
    Scientific,
    Film,
    TextCode,
    Video,
    Audio,
    COUNT
};

/// Test validation severity
enum class TestValidationV2 : uint8_t {
    DecodesWithoutCrash,    // P0 — must not crash
    ProducesPixels,         // P0 — non-zero output
    CorrectDimensions,      // P1 — expected size
    ReferenceMatch,         // P2 — pixel comparison
    PerformanceTarget,      // P1 — timing gate
    MemoryBudget            // P1 — memory cap
};

/// COM integration test type
enum class COMTestType : uint8_t {
    Registration,           // DllRegisterServer/DllUnregisterServer
    ClassFactory,           // IClassFactory::CreateInstance
    ThumbnailProvider,      // IThumbnailProvider::GetThumbnail
    InitializeWithStream,   // IInitializeWithStream::Initialize
    ConcurrentAccess,       // Multiple simultaneous COM calls
    COUNT
};

/// Test corpus stats V2
struct TestCorpusStatsV2 {
    uint32_t totalFiles         = 0;
    uint32_t formatsRepresented = 0;
    uint64_t totalSizeBytes     = 0;
    uint32_t categoriesCovered  = 0;
};

/// V2 config for real-file testing
struct RealFileTestConfig {
    uint32_t targetDecoderTests = 300;
    uint32_t targetCOMTests     = 50;
    uint32_t targetRegression   = 100;
    uint32_t targetPerformance  = 30;
    double   maxTestTimeSeconds = 60.0;
};

/// Test suite expansion V2 manager
class TestSuiteExpansionV2 {
public:
    static const wchar_t* CategoryName(TestFileCategoryV2 c) {
        switch (c) {
            case TestFileCategoryV2::ArchiveFormats:     return L"Archive Formats";
            case TestFileCategoryV2::ModernImages:       return L"Modern Images";
            case TestFileCategoryV2::ProfessionalImages: return L"Professional Images";
            case TestFileCategoryV2::CameraRAW:          return L"Camera RAW";
            case TestFileCategoryV2::VectorGraphics:     return L"Vector Graphics";
            case TestFileCategoryV2::Documents:          return L"Documents";
            case TestFileCategoryV2::Fonts:              return L"Fonts";
            case TestFileCategoryV2::Models3D:           return L"3D Models";
            case TestFileCategoryV2::EBooks:             return L"eBooks";
            case TestFileCategoryV2::Scientific:         return L"Scientific";
            case TestFileCategoryV2::Film:               return L"Film";
            case TestFileCategoryV2::TextCode:           return L"Text/Code";
            case TestFileCategoryV2::Video:              return L"Video";
            case TestFileCategoryV2::Audio:              return L"Audio";
            default: return L"Unknown";
        }
    }

    static const wchar_t* COMTestName(COMTestType t) {
        switch (t) {
            case COMTestType::Registration:         return L"COM Registration";
            case COMTestType::ClassFactory:         return L"Class Factory";
            case COMTestType::ThumbnailProvider:    return L"Thumbnail Provider";
            case COMTestType::InitializeWithStream: return L"Initialize With Stream";
            case COMTestType::ConcurrentAccess:     return L"Concurrent Access";
            default: return L"Unknown";
        }
    }

    static constexpr size_t CategoryCount() { return static_cast<size_t>(TestFileCategoryV2::COUNT); }
    static constexpr size_t COMTestCount() { return static_cast<size_t>(COMTestType::COUNT); }

    static uint32_t TotalTarget(const RealFileTestConfig& cfg) {
        return cfg.targetDecoderTests + cfg.targetCOMTests +
               cfg.targetRegression + cfg.targetPerformance;
    }
};

}} // namespace ExplorerLens::Engine

