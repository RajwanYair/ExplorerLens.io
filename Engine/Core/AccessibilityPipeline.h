//==============================================================================
// ExplorerLens Engine — Accessibility Pipeline
// Screen reader, high contrast, and keyboard navigation for thumbnails.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// Accessibility feature
enum class PipelineA11yFeature : uint8_t {
 ScreenReader, // MSAA/UIA alt-text for thumbnails
 HighContrast, // High-contrast border/overlay
 KeyboardNav, // Arrow key thumbnail navigation
 ColorBlindMode, // Deuteranopia/Protanopia palettes
 ReducedMotion, // No animations
 LargeText, // Enlarged labels and status text
 COUNT
};

/// Color blind mode
enum class ColorBlindMode : uint8_t {
 None,
 Deuteranopia, // Red-green (most common)
 Protanopia, // Red-green (variant)
 Tritanopia, // Blue-yellow
 Achromatopsia, // Total color blindness
 COUNT
};

/// High contrast theme
enum class PipelineHCTheme : uint8_t {
 SystemDefault,
 WhiteOnBlack,
 BlackOnWhite,
 YellowOnBlack,
 Custom,
 COUNT
};

/// Accessibility config
struct AccessibilityConfig {
 bool screenReader = true;
 bool highContrast = false;
 bool keyboardNav = true;
 bool reducedMotion = false;
 bool largeText = false;
 ColorBlindMode cbMode = ColorBlindMode::None;
 PipelineHCTheme hcTheme = PipelineHCTheme::SystemDefault;
 uint32_t borderWidth = 2;
};

/// Accessibility pipeline
class AccessibilityPipeline {
public:
 static const wchar_t* FeatureName(PipelineA11yFeature f) {
 switch (f) {
 case PipelineA11yFeature::ScreenReader: return L"Screen Reader";
 case PipelineA11yFeature::HighContrast: return L"High Contrast";
 case PipelineA11yFeature::KeyboardNav: return L"Keyboard Navigation";
 case PipelineA11yFeature::ColorBlindMode: return L"Color Blind Mode";
 case PipelineA11yFeature::ReducedMotion: return L"Reduced Motion";
 case PipelineA11yFeature::LargeText: return L"Large Text";
 default: return L"Unknown";
 }
 }

 static const wchar_t* ColorBlindModeName(ColorBlindMode m) {
 switch (m) {
 case ColorBlindMode::None: return L"None";
 case ColorBlindMode::Deuteranopia: return L"Deuteranopia";
 case ColorBlindMode::Protanopia: return L"Protanopia";
 case ColorBlindMode::Tritanopia: return L"Tritanopia";
 case ColorBlindMode::Achromatopsia: return L"Achromatopsia";
 default: return L"Unknown";
 }
 }

 static const wchar_t* HCThemeName(PipelineHCTheme t) {
 switch (t) {
 case PipelineHCTheme::SystemDefault: return L"System Default";
 case PipelineHCTheme::WhiteOnBlack: return L"White on Black";
 case PipelineHCTheme::BlackOnWhite: return L"Black on White";
 case PipelineHCTheme::YellowOnBlack: return L"Yellow on Black";
 case PipelineHCTheme::Custom: return L"Custom";
 default: return L"Unknown";
 }
 }

 static constexpr size_t FeatureCount() { return static_cast<size_t>(PipelineA11yFeature::COUNT); }
 static constexpr size_t ColorBlindModeCount() { return static_cast<size_t>(ColorBlindMode::COUNT); }
 static constexpr size_t HCThemeCount() { return static_cast<size_t>(PipelineHCTheme::COUNT); }
};

}} // namespace ExplorerLens::Engine

