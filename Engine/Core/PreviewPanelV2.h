//==============================================================================
// ExplorerLens Engine — Preview Panel V2
// Rich IPreviewHandler with multi-tab metadata display, raw hex view,
// color picker, zoom/pan, and annotation overlay for Explorer preview pane.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PreviewPanelTab : uint8_t {
  Image = 0,
  Metadata,
  ColorInfo,
  RawHex,
  Histogram,
  COUNT
};
enum class PreviewZoomLevel : uint8_t {
  FitToPane = 0,
  Zoom50,
  Zoom100,
  Zoom200,
  Zoom400,
  FitToWindow = FitToPane, // compat alias
  COUNT = Zoom400 + 1
};
enum class ColorPickerMode : uint8_t {
  Disabled = 0,
  PointSample,
  AreaAverage,
  HEX = PointSample, // compat alias
  COUNT = AreaAverage + 1
};

struct PreviewPanelV2Config {
  PreviewPanelTab defaultTab = PreviewPanelTab::Image;
  PreviewZoomLevel zoomLevel = PreviewZoomLevel::FitToPane;
  ColorPickerMode colorPicker = ColorPickerMode::PointSample;
  bool showHistogram = true;
  bool showGrid = false;
  bool showRuler = false;
};

struct PreviewColorSample {
  uint8_t r = 0, g = 0, b = 0, a = 255;
  float h = 0, s = 0, v = 0;
  std::wstring hexCode;
};

class PreviewPanelV2 {
public:
  static const wchar_t *TabName(PreviewPanelTab t) {
    switch (t) {
    case PreviewPanelTab::Image:
      return L"Image";
    case PreviewPanelTab::Metadata:
      return L"Metadata";
    case PreviewPanelTab::ColorInfo:
      return L"Color Info";
    case PreviewPanelTab::RawHex:
      return L"Raw Hex";
    case PreviewPanelTab::Histogram:
      return L"Histogram";
    default:
      return L"Unknown";
    }
  }
  static const wchar_t *ZoomLevelName(PreviewZoomLevel z) {
    switch (z) {
    case PreviewZoomLevel::FitToPane:
      return L"Fit to Pane";
    case PreviewZoomLevel::Zoom50:
      return L"50%";
    case PreviewZoomLevel::Zoom100:
      return L"100%";
    case PreviewZoomLevel::Zoom200:
      return L"200%";
    case PreviewZoomLevel::Zoom400:
      return L"400%";
    default:
      return L"Unknown";
    }
  }
  static const wchar_t *ColorPickerModeName(ColorPickerMode m) {
    switch (m) {
    case ColorPickerMode::Disabled:
      return L"Disabled";
    case ColorPickerMode::PointSample:
      return L"Point Sample";
    case ColorPickerMode::AreaAverage:
      return L"Area Average";
    default:
      return L"Unknown";
    }
  }
  static constexpr size_t TabCount() {
    return static_cast<size_t>(PreviewPanelTab::COUNT);
  }
  static constexpr size_t ZoomLevelCount() {
    return static_cast<size_t>(PreviewZoomLevel::COUNT);
  }
  static constexpr size_t ColorPickerCount() {
    return static_cast<size_t>(ColorPickerMode::COUNT);
  }
  static PreviewPanelV2Config DefaultConfig() { return PreviewPanelV2Config{}; }
};

} // namespace Engine
} // namespace ExplorerLens
