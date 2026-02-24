//==============================================================================
// CodeCoverageIntegration
// OpenCppCoverage + LibFuzzer infrastructure
//==============================================================================

#include "CodeCoverageIntegration.h"
#include <algorithm>
#include <sstream>

namespace ExplorerLens { namespace Engine {

CodeCoverageIntegration::CodeCoverageIntegration()
    : m_thresholds(CoverageThresholds::ForCI())
{
}

CodeCoverageIntegration::CodeCoverageIntegration(const CoverageThresholds& thresholds)
    : m_thresholds(thresholds)
{
}

std::wstring CodeCoverageIntegration::GenerateCoverageCommand(
    const std::wstring& testExecutable,
    const std::wstring& outputDir) const
{
    // Generate OpenCppCoverage command line
    std::wstring cmd = L"OpenCppCoverage.exe";
    cmd += L" --sources Engine\\";
    cmd += L" --sources LENSShell\\";
    cmd += L" --excluded_sources Engine\\Tests\\";
    cmd += L" --excluded_sources external\\";
    cmd += L" --export_type cobertura:" + outputDir + L"\\coverage.xml";
    cmd += L" --export_type html:" + outputDir + L"\\html";
    cmd += L" --cover_children";
    cmd += L" -- " + testExecutable;
    return cmd;
}

CoverageReport CodeCoverageIntegration::ParseCoverageReport(
    const std::wstring& xmlPath) const
{
    CoverageReport report;
    report.reportPath = xmlPath;

    // In production, parse Cobertura XML format
    // For now, return empty report structure
    report.totalModules = 0;
    report.overallLineCoverage = 0.0;
    report.overallBranchCoverage = 0.0;
    report.overallFunctionCoverage = 0.0;
    report.meetsMinThresholds = false;
    report.meetsTargetThresholds = false;

    return report;
}

bool CodeCoverageIntegration::ValidateCoverage(const CoverageReport& report) const
{
    return report.overallLineCoverage >= m_thresholds.minLineCoverage &&
           report.overallBranchCoverage >= m_thresholds.minBranchCoverage &&
           report.overallFunctionCoverage >= m_thresholds.minFunctionCoverage;
}

std::vector<FuzzTargetConfig> CodeCoverageIntegration::GenerateFuzzTargets() const
{
    std::vector<FuzzTargetConfig> targets;
    auto decoders = GetFuzzableDecoders();

    for (const auto& decoder : decoders) {
        FuzzTargetConfig config;
        config.decoderName = decoder;
        config.seedCorpusDir = L"test-archives\\fuzz\\" + decoder;
        config.maxInputSize = 65536;
        config.maxDurationSeconds = 300;
        config.maxIterations = 100000;
        config.targetTypes = {
            FuzzTargetType::HeaderParsing,
            FuzzTargetType::FullDecode,
            FuzzTargetType::MalformedInput
        };
        targets.push_back(std::move(config));
    }

    return targets;
}

ModuleCoverage CodeCoverageIntegration::GetModuleCoverage(
    const std::wstring& moduleName,
    const CoverageReport& report) const
{
    for (const auto& mod : report.modules) {
        if (mod.moduleName == moduleName)
            return mod;
    }
    return ModuleCoverage{};
}

CoverageReport CodeCoverageIntegration::MergeReports(
    const std::vector<CoverageReport>& reports) const
{
    CoverageReport merged;
    uint32_t totalLines = 0, coveredLines = 0;
    uint32_t totalBranches = 0, coveredBranches = 0;
    uint32_t totalFunctions = 0, coveredFunctions = 0;

    for (const auto& report : reports) {
        for (const auto& mod : report.modules) {
            totalLines += mod.totalLines;
            coveredLines += mod.coveredLines;
            totalBranches += mod.totalBranches;
            coveredBranches += mod.coveredBranches;
            totalFunctions += mod.totalFunctions;
            coveredFunctions += mod.coveredFunctions;
            merged.modules.push_back(mod);
        }
    }

    merged.totalModules = static_cast<uint32_t>(merged.modules.size());
    merged.overallLineCoverage = totalLines > 0 ?
        (100.0 * coveredLines / totalLines) : 0.0;
    merged.overallBranchCoverage = totalBranches > 0 ?
        (100.0 * coveredBranches / totalBranches) : 0.0;
    merged.overallFunctionCoverage = totalFunctions > 0 ?
        (100.0 * coveredFunctions / totalFunctions) : 0.0;
    merged.meetsMinThresholds = ValidateCoverage(merged);

    return merged;
}

const wchar_t* CodeCoverageIntegration::GetMetricName(CoverageMetric metric)
{
    switch (metric) {
        case CoverageMetric::LineCoverage:     return L"LineCoverage";
        case CoverageMetric::BranchCoverage:   return L"BranchCoverage";
        case CoverageMetric::FunctionCoverage: return L"FunctionCoverage";
        case CoverageMetric::RegionCoverage:   return L"RegionCoverage";
        default:                               return L"Unknown";
    }
}

const wchar_t* CodeCoverageIntegration::GetFuzzTargetName(FuzzTargetType type)
{
    switch (type) {
        case FuzzTargetType::HeaderParsing:      return L"HeaderParsing";
        case FuzzTargetType::FullDecode:         return L"FullDecode";
        case FuzzTargetType::MetadataExtraction: return L"MetadataExtraction";
        case FuzzTargetType::MalformedInput:     return L"MalformedInput";
        case FuzzTargetType::BoundaryValues:     return L"BoundaryValues";
        default:                                 return L"Unknown";
    }
}

std::vector<std::wstring> CodeCoverageIntegration::GetFuzzableDecoders()
{
    return {
        L"ImageDecoder", L"WebPDecoder", L"AVIFDecoder", L"JXLDecoder",
        L"HEIFDecoder", L"RAWDecoder", L"ICODecoder", L"TGADecoder",
        L"QOIDecoder", L"PSDDecoder", L"DDSDecoder", L"HDRDecoder",
        L"PPMDecoder", L"EXRDecoder", L"SVGDecoder", L"PDFDecoder",
        L"FontDecoder", L"WMFDecoder", L"PCXDecoder", L"FarbfeldDecoder",
        L"EPSDecoder", L"KTXTextureDecoder", L"VTFDecoder",
        L"OpenRasterDecoder", L"XCFDecoder", L"SGIDecoder", L"XPMDecoder"
    };
}

}} // namespace ExplorerLens::Engine

