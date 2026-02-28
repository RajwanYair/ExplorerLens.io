#pragma once
// DocumentLayoutAnalyzer.h — Document Layout Analyzer
// Structural analysis of document pages for smart thumbnail cropping —
// identifies title blocks, text regions, figures, tables, and headers.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Document region type
enum class DocRegionType : uint8_t {
 Unknown = 0,
 Title,
 Header,
 Footer,
 TextBlock,
 Figure,
 Table,
 Chart,
 Sidebar,
 PageNumber,
 Watermark,
 COUNT
};

/// Layout analysis method
enum class LayoutMethod : uint8_t {
 Heuristic = 0, // Rule-based (fast)
 ConnectedComponent, // CC analysis
 ProjectionProfile, // Horizontal/vertical projections
 MLBased, // DirectML model
 COUNT
};

struct DocumentRegion {
 DocRegionType type = DocRegionType::Unknown;
 float x = 0.0f;
 float y = 0.0f;
 float width = 0.0f;
 float height = 0.0f;
 float confidence = 0.0f;
 bool isCropTarget = false;
};

struct LayoutAnalysisResult {
 uint32_t regionCount = 0;
 bool hasTitleBlock = false;
 bool hasFigures = false;
 bool hasTables = false;
 float textDensity = 0.0f;
 float analysisTimeMs = 0.0f;
 LayoutMethod methodUsed = LayoutMethod::Heuristic;
};

class DocumentLayoutAnalyzer {
public:
 static constexpr size_t RegionTypeCount() {
 return static_cast<size_t>(DocRegionType::COUNT);
 }
 static constexpr size_t MethodCount() {
 return static_cast<size_t>(LayoutMethod::COUNT);
 }

 static const wchar_t *RegionTypeName(DocRegionType r) {
 switch (r) {
 case DocRegionType::Unknown:
 return L"Unknown";
 case DocRegionType::Title:
 return L"Title";
 case DocRegionType::Header:
 return L"Header";
 case DocRegionType::Footer:
 return L"Footer";
 case DocRegionType::TextBlock:
 return L"Text Block";
 case DocRegionType::Figure:
 return L"Figure";
 case DocRegionType::Table:
 return L"Table";
 case DocRegionType::Chart:
 return L"Chart";
 case DocRegionType::Sidebar:
 return L"Sidebar";
 case DocRegionType::PageNumber:
 return L"Page Number";
 case DocRegionType::Watermark:
 return L"Watermark";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *MethodName(LayoutMethod m) {
 switch (m) {
 case LayoutMethod::Heuristic:
 return L"Heuristic";
 case LayoutMethod::ConnectedComponent:
 return L"Connected Component";
 case LayoutMethod::ProjectionProfile:
 return L"Projection Profile";
 case LayoutMethod::MLBased:
 return L"ML-Based";
 default:
 return L"Unknown";
 }
 }

 /// Estimate text density from ink coverage and region count
 static float EstimateTextDensity(float inkCoverage, uint32_t textRegions,
 uint32_t totalRegions) {
 if (totalRegions == 0)
 return 0.0f;
 float regionRatio = static_cast<float>(textRegions) / totalRegions;
 return inkCoverage * 0.5f + regionRatio * 0.5f;
 }
};

} // namespace Engine
} // namespace ExplorerLens
