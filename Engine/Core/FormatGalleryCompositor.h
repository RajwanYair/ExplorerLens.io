#pragma once
// Sprint 432: Format Gallery Compositor
// Generates composite gallery views from folders — mosaic, grid, collage,
// and fan layouts for folder thumbnail previews in Explorer.
#include <algorithm>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Gallery layout mode
enum class GalleryLayout : uint8_t {
  Grid2x2 = 0, // 4-up grid (Windows default style)
  Grid3x3,     // 9-up grid
  Mosaic,      // Variable-size mosaic
  Collage,     // Overlapping with rotation
  Fan,         // Fanned like playing cards
  Stack,       // Stacked with offset
  COUNT
};

/// Image selection for gallery
enum class GallerySelection : uint8_t {
  FirstN = 0,   // First N images in sort order
  LargestN,     // N largest images
  MostColorful, // N most colorful images
  Random,       // Random selection
  Covers,       // Detected cover/title images
  COUNT
};

struct GalleryConfig {
  GalleryLayout layout = GalleryLayout::Grid2x2;
  GallerySelection selection = GallerySelection::FirstN;
  uint32_t maxImages = 4;
  uint32_t outputSizePx = 256;
  float padding = 2.0f;
  float rotation = 0.0f; // for Collage/Fan
  bool addShadow = false;
  bool roundCorners = false;
};

struct GalleryStats {
  uint32_t imagesUsed = 0;
  uint32_t imagesScanned = 0;
  double compositeTimeMs = 0.0;
};

class FormatGalleryCompositor {
public:
  static constexpr size_t LayoutCount() {
    return static_cast<size_t>(GalleryLayout::COUNT);
  }
  static constexpr size_t SelectionCount() {
    return static_cast<size_t>(GallerySelection::COUNT);
  }

  static const wchar_t *LayoutName(GalleryLayout l) {
    switch (l) {
    case GalleryLayout::Grid2x2:
      return L"2x2 Grid";
    case GalleryLayout::Grid3x3:
      return L"3x3 Grid";
    case GalleryLayout::Mosaic:
      return L"Mosaic";
    case GalleryLayout::Collage:
      return L"Collage";
    case GalleryLayout::Fan:
      return L"Fan";
    case GalleryLayout::Stack:
      return L"Stack";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *SelectionName(GallerySelection s) {
    switch (s) {
    case GallerySelection::FirstN:
      return L"First N";
    case GallerySelection::LargestN:
      return L"Largest N";
    case GallerySelection::MostColorful:
      return L"Most Colorful";
    case GallerySelection::Random:
      return L"Random";
    case GallerySelection::Covers:
      return L"Cover Images";
    default:
      return L"Unknown";
    }
  }

  /// Calculate cell size for grid layout
  static uint32_t GridCellSize(uint32_t outputSize, uint32_t gridDim,
                               float padding) {
    float paddingTotal = padding * (gridDim + 1);
    float available = static_cast<float>(outputSize) - paddingTotal;
    return static_cast<uint32_t>((std::max)(0.0f, available / gridDim));
  }
};

} // namespace Engine
} // namespace ExplorerLens
