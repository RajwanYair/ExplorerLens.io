#pragma once
// MultiPageNavigator.h — Multi-Page Document Navigator
// Page selection intelligence for multi-page documents (PDF, TIFF, PSD layers),
// choosing the most visually representative page for thumbnail generation.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Page selection strategy
enum class PageSelectionStrategy : uint8_t {
  FirstPage = 0, // Always use page 1
  CoverPage,     // Detect cover/title page
  MostColorful,  // Highest color variance
  MostContent,   // Highest ink coverage
  MiddlePage,    // Use middle page
  UserSpecified, // Custom page index
  COUNT
};

/// Multi-page format type
enum class MultiPageFormat : uint8_t {
  PDF = 0,
  TIFF,
  PSD,
  GIF,
  WEBP_Anim,
  DICOM,
  ICO,
  APNG,
  COUNT
};

struct PageInfo {
  uint32_t index = 0;
  uint32_t widthPx = 0;
  uint32_t heightPx = 0;
  float inkCoverage = 0.0f;   // 0-1 fraction of non-white pixels
  float colorVariance = 0.0f; // higher = more colorful
  bool isBlank = false;
};

class MultiPageNavigator {
public:
  static constexpr size_t StrategyCount() {
    return static_cast<size_t>(PageSelectionStrategy::COUNT);
  }
  static constexpr size_t FormatCount() {
    return static_cast<size_t>(MultiPageFormat::COUNT);
  }

  static const wchar_t *StrategyName(PageSelectionStrategy s) {
    switch (s) {
    case PageSelectionStrategy::FirstPage:
      return L"First Page";
    case PageSelectionStrategy::CoverPage:
      return L"Cover Page";
    case PageSelectionStrategy::MostColorful:
      return L"Most Colorful";
    case PageSelectionStrategy::MostContent:
      return L"Most Content";
    case PageSelectionStrategy::MiddlePage:
      return L"Middle Page";
    case PageSelectionStrategy::UserSpecified:
      return L"User Specified";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *FormatName(MultiPageFormat f) {
    switch (f) {
    case MultiPageFormat::PDF:
      return L"PDF";
    case MultiPageFormat::TIFF:
      return L"TIFF";
    case MultiPageFormat::PSD:
      return L"PSD";
    case MultiPageFormat::GIF:
      return L"GIF";
    case MultiPageFormat::WEBP_Anim:
      return L"Animated WebP";
    case MultiPageFormat::DICOM:
      return L"DICOM";
    case MultiPageFormat::ICO:
      return L"ICO";
    case MultiPageFormat::APNG:
      return L"APNG";
    default:
      return L"Unknown";
    }
  }

  static uint32_t SelectPage(PageSelectionStrategy strategy,
                             uint32_t totalPages, uint32_t userPage = 0) {
    if (totalPages == 0)
      return 0;
    switch (strategy) {
    case PageSelectionStrategy::FirstPage:
      return 0;
    case PageSelectionStrategy::CoverPage:
      return 0; // heuristic: cover is first
    case PageSelectionStrategy::MiddlePage:
      return totalPages / 2;
    case PageSelectionStrategy::UserSpecified:
      return (userPage < totalPages) ? userPage : 0;
    default:
      return 0;
    }
  }
};

} // namespace Engine
} // namespace ExplorerLens
