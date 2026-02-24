//==============================================================================
// ExplorerLens Engine — Quality Assurance V2
// Final QA matrix aggregating unit, integration, fuzzing, regression, and
// performance test results into a unified pass/fail signal. Tracks defect
// density, escape rate, and test-age staleness for v14.0 ship criteria.
//==============================================================================
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class QATestCategory  : uint8_t { Unit=0,Integration,Performance,Fuzz,Regression,Accessibility,COUNT };
enum class QADefectSeverity: uint8_t { Critical=0,High,Medium,Low,Info,COUNT };
enum class QAShipSignal    : uint8_t { Ship=0,ConditionalShip,Hold,Block,COUNT };

struct QASuiteResult {
    QATestCategory category     = QATestCategory::Unit;
    uint32_t       testsTotal   = 0;
    uint32_t       testsPassed  = 0;
    uint32_t       testsFailed  = 0;
    float          passPct      = 0.0f;
    bool           suitePass    = false;
};

struct QAFinalMatrix {
    uint32_t       totalTests           = 0;
    uint32_t       totalPassed          = 0;
    float          overallPassPct       = 0.0f;
    uint32_t       criticalDefects      = 0;
    float          defectDensity        = 0.0f; // defects per 1K lines
    float          escapeRate           = 0.0f; // % defects found post-release
    QAShipSignal   shipSignal           = QAShipSignal::Hold;
};

class QualityAssuranceV2 {
public:
    static const wchar_t* TestCategoryName(QATestCategory c) {
        switch(c) {
            case QATestCategory::Unit:          return L"Unit Tests";
            case QATestCategory::Integration:   return L"Integration Tests";
            case QATestCategory::Performance:   return L"Performance Tests";
            case QATestCategory::Fuzz:          return L"Fuzz Tests";
            case QATestCategory::Regression:    return L"Regression Tests";
            case QATestCategory::Accessibility: return L"Accessibility Tests";
            default: return L"Unknown";
        }
    }
    static const wchar_t* DefectSeverityName(QADefectSeverity s) {
        switch(s) {
            case QADefectSeverity::Critical: return L"Critical";
            case QADefectSeverity::High:     return L"High";
            case QADefectSeverity::Medium:   return L"Medium";
            case QADefectSeverity::Low:      return L"Low";
            case QADefectSeverity::Info:     return L"Info";
            default: return L"Unknown";
        }
    }
    static const wchar_t* ShipSignalName(QAShipSignal s) {
        switch(s) {
            case QAShipSignal::Ship:            return L"SHIP";
            case QAShipSignal::ConditionalShip: return L"CONDITIONAL SHIP";
            case QAShipSignal::Hold:            return L"HOLD";
            case QAShipSignal::Block:           return L"BLOCK";
            default: return L"Unknown";
        }
    }
    static constexpr size_t TestCategoryCount()   { return static_cast<size_t>(QATestCategory::COUNT); }
    static constexpr size_t DefectSeverityCount() { return static_cast<size_t>(QADefectSeverity::COUNT); }
    static constexpr size_t ShipSignalCount()     { return static_cast<size_t>(QAShipSignal::COUNT); }
    static QAShipSignal Evaluate(const QAFinalMatrix& m) {
        if (m.criticalDefects > 0)           return QAShipSignal::Block;
        if (m.overallPassPct < 99.0f)        return QAShipSignal::Hold;
        if (m.defectDensity  > 0.5f)         return QAShipSignal::ConditionalShip;
        return QAShipSignal::Ship;
    }
};

}} // namespace ExplorerLens::Engine

