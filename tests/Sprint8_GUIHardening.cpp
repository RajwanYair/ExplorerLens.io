//==============================================================================
// ExplorerLens — Sprint 8 Tests: GUI Hardening
// Tests DarkModeHelper, high-DPI scaling, decoder health dashboard,
// diagnostics exporter, theme colors, and control type enumeration.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>

// Header under test
#include "../Engine/Utils/GUIHardening.h"

using namespace ExplorerLens::Engine::GUI;

//==============================================================================
// Theme Colors Tests
//==============================================================================

TEST(ThemeColors, DarkDefaults)
{
    auto dark = ThemeColors::Dark();
    EXPECT_EQ(GetRValue(dark.background), 32);
    EXPECT_EQ(GetGValue(dark.background), 32);
    EXPECT_EQ(GetBValue(dark.background), 32);
    EXPECT_EQ(GetRValue(dark.text), 240);
}

TEST(ThemeColors, LightDefaults)
{
    auto light = ThemeColors::Light();
    EXPECT_EQ(GetRValue(light.background), 255);
    EXPECT_EQ(GetGValue(light.background), 255);
    EXPECT_EQ(GetBValue(light.background), 255);
    EXPECT_EQ(GetRValue(light.text), 30);
}

TEST(ThemeColors, DefinesAllColors)
{
    auto dark = ThemeColors::Dark();
    EXPECT_NE(dark.background, dark.text);
    EXPECT_NE(dark.controlBg, dark.background);
    EXPECT_NE(dark.buttonBg, dark.controlBg);
    EXPECT_NE(dark.errorText, dark.successText);
    EXPECT_NE(dark.warningText, dark.errorText);
}

//==============================================================================
// Control Type Tests
//==============================================================================

TEST(ControlType, AllNamesValid)
{
    for (uint32_t i = 0; i < static_cast<uint32_t>(ControlType::MaxType); ++i) {
        auto name = ControlTypeName(static_cast<ControlType>(i));
        EXPECT_STRNE(name, "Unknown") << "Index " << i << " returned Unknown";
    }
}

TEST(ControlType, SpecificNames)
{
    EXPECT_STREQ(ControlTypeName(ControlType::Button), "Button");
    EXPECT_STREQ(ControlTypeName(ControlType::ListView), "ListView");
    EXPECT_STREQ(ControlTypeName(ControlType::TreeView), "TreeView");
    EXPECT_STREQ(ControlTypeName(ControlType::ComboBox), "ComboBox");
    EXPECT_STREQ(ControlTypeName(ControlType::Dialog), "Dialog");
}

TEST(ControlType, UnknownForOutOfRange)
{
    EXPECT_STREQ(ControlTypeName(static_cast<ControlType>(99)), "Unknown");
}

TEST(ControlType, ControlCount)
{
    EXPECT_EQ(static_cast<uint32_t>(ControlType::MaxType), 20u);
}

//==============================================================================
// Dark Mode Helper Tests
//==============================================================================

TEST(DarkModeHelper, DefaultLightMode)
{
    DarkModeHelper helper;
    EXPECT_FALSE(helper.IsDarkMode());
    EXPECT_EQ(GetRValue(helper.Colors().background), 255);
}

TEST(DarkModeHelper, SwitchToDark)
{
    DarkModeHelper helper;
    helper.SetDarkMode(true);
    EXPECT_TRUE(helper.IsDarkMode());
    EXPECT_EQ(GetRValue(helper.Colors().background), 32);
}

TEST(DarkModeHelper, SwitchBackToLight)
{
    DarkModeHelper helper;
    helper.SetDarkMode(true);
    helper.SetDarkMode(false);
    EXPECT_FALSE(helper.IsDarkMode());
    EXPECT_EQ(GetRValue(helper.Colors().background), 255);
}

TEST(DarkModeHelper, ThemedCounts)
{
    DarkModeHelper helper;
    EXPECT_EQ(helper.ThemedWindowCount(), 0u);
    EXPECT_EQ(helper.ThemedControlCount(), 0u);
    EXPECT_EQ(helper.TotalThemed(), 0u);
}

TEST(DarkModeHelper, NullWindowRejected)
{
    DarkModeHelper helper;
    helper.SetDarkMode(true);
    EXPECT_FALSE(helper.ApplyToWindow(nullptr));
    EXPECT_FALSE(helper.ApplyToControl(nullptr, ControlType::Button));
}

TEST(DarkModeHelper, Reset)
{
    DarkModeHelper helper;
    helper.Reset();
    EXPECT_EQ(helper.TotalThemed(), 0u);
}

//==============================================================================
// High-DPI Manager Tests
//==============================================================================

TEST(HighDPI, ScaleForDPI96)
{
    EXPECT_EQ(HighDPIManager::ScaleForDPI(100, 96), 100);
}

TEST(HighDPI, ScaleForDPI144)
{
    EXPECT_EQ(HighDPIManager::ScaleForDPI(100, 144), 150);
}

TEST(HighDPI, ScaleForDPI192)
{
    EXPECT_EQ(HighDPIManager::ScaleForDPI(100, 192), 200);
}

TEST(HighDPI, ScaledThumbnailSize)
{
    auto sz = HighDPIManager::GetScaledThumbnailSize(256, 144);
    EXPECT_EQ(sz.cx, 384); // 256 * 150%
    EXPECT_EQ(sz.cy, 384);
}

TEST(HighDPI, ScaledThumbnailSize200)
{
    auto sz = HighDPIManager::GetScaledThumbnailSize(256, 192);
    EXPECT_EQ(sz.cx, 512);
    EXPECT_EQ(sz.cy, 512);
}

TEST(HighDPI, MonitorDPIInfoScale)
{
    HighDPIManager::MonitorDPIInfo info{};
    info.dpiX = 144;
    info.dpiY = 144;
    EXPECT_EQ(info.ScaleX(100), 150);
    EXPECT_EQ(info.ScaleY(200), 300);
}

//==============================================================================
// Decoder Health State Tests
//==============================================================================

TEST(DecoderHealth, StateNames)
{
    EXPECT_STREQ(DecoderHealthStateName(DecoderHealthState::Healthy), "Healthy");
    EXPECT_STREQ(DecoderHealthStateName(DecoderHealthState::Degraded), "Degraded");
    EXPECT_STREQ(DecoderHealthStateName(DecoderHealthState::CircuitOpen), "CircuitOpen");
    EXPECT_STREQ(DecoderHealthStateName(DecoderHealthState::NotLoaded), "NotLoaded");
    EXPECT_STREQ(DecoderHealthStateName(DecoderHealthState::Error), "Error");
    EXPECT_STREQ(DecoderHealthStateName(DecoderHealthState::Unknown), "Unknown");
}

TEST(DecoderHealth, SuccessRate)
{
    DecoderHealthEntry entry;
    entry.totalDecodes = 100;
    entry.failedDecodes = 5;
    EXPECT_DOUBLE_EQ(entry.SuccessRate(), 95.0);
}

TEST(DecoderHealth, SuccessRateZero)
{
    DecoderHealthEntry entry;
    EXPECT_DOUBLE_EQ(entry.SuccessRate(), 0.0);
}

//==============================================================================
// Decoder Health Dashboard Tests
//==============================================================================

TEST(HealthDashboard, RegisterDecoder)
{
    DecoderHealthDashboard dashboard;
    dashboard.RegisterDecoder("JPEG", {".jpg", ".jpeg"});
    dashboard.RegisterDecoder("PNG", {".png"});
    EXPECT_EQ(dashboard.DecoderCount(), 2u);
}

TEST(HealthDashboard, InitialStateNotLoaded)
{
    DecoderHealthDashboard dashboard;
    dashboard.RegisterDecoder("WebP", {".webp"});
    auto health = dashboard.GetHealth("WebP");
    EXPECT_EQ(health.state, DecoderHealthState::NotLoaded);
}

TEST(HealthDashboard, RecordDecodeHealthy)
{
    DecoderHealthDashboard dashboard;
    dashboard.RegisterDecoder("JPEG", {".jpg"});
    for (int i = 0; i < 100; ++i) {
        dashboard.RecordDecode("JPEG", 5.0, true);
    }
    auto health = dashboard.GetHealth("JPEG");
    EXPECT_EQ(health.state, DecoderHealthState::Healthy);
    EXPECT_EQ(health.totalDecodes, 100u);
    EXPECT_EQ(health.failedDecodes, 0u);
    EXPECT_DOUBLE_EQ(health.SuccessRate(), 100.0);
}

TEST(HealthDashboard, RecordDecodeDegraded)
{
    DecoderHealthDashboard dashboard;
    dashboard.RegisterDecoder("HEIF", {".heif", ".heic"});
    for (int i = 0; i < 90; ++i) dashboard.RecordDecode("HEIF", 10.0, true);
    for (int i = 0; i < 5; ++i) dashboard.RecordDecode("HEIF", 100.0, false);
    auto health = dashboard.GetHealth("HEIF");
    EXPECT_GE(health.SuccessRate(), 90.0);
    EXPECT_LT(health.SuccessRate(), 99.0);
    EXPECT_EQ(health.state, DecoderHealthState::Degraded);
}

TEST(HealthDashboard, SetError)
{
    DecoderHealthDashboard dashboard;
    dashboard.RegisterDecoder("PSD", {".psd"});
    dashboard.SetError("PSD", "DLL not found");
    auto health = dashboard.GetHealth("PSD");
    EXPECT_EQ(health.state, DecoderHealthState::Error);
    EXPECT_EQ(health.lastError, "DLL not found");
}

TEST(HealthDashboard, GetAllHealth)
{
    DecoderHealthDashboard dashboard;
    dashboard.RegisterDecoder("JPEG", {".jpg"});
    dashboard.RegisterDecoder("PNG", {".png"});
    dashboard.RegisterDecoder("TIFF", {".tiff"});
    auto all = dashboard.GetAllHealth();
    EXPECT_EQ(all.size(), 3u);
}

TEST(HealthDashboard, HealthyCounts)
{
    DecoderHealthDashboard dashboard;
    dashboard.RegisterDecoder("A", {});
    dashboard.RegisterDecoder("B", {});
    dashboard.RecordDecode("A", 5.0, true);
    dashboard.SetError("B", "error");
    EXPECT_EQ(dashboard.HealthyCount(), 1u);
    EXPECT_EQ(dashboard.ErrorCount(), 1u);
}

TEST(HealthDashboard, UnknownDecoderReturnsEmpty)
{
    DecoderHealthDashboard dashboard;
    auto health = dashboard.GetHealth("NonExistent");
    EXPECT_EQ(health.state, DecoderHealthState::Unknown);
}

//==============================================================================
// Diagnostics Exporter Tests
//==============================================================================

TEST(Diagnostics, AddSectionAndEntries)
{
    DiagnosticsExporter exporter;
    exporter.AddSection("System");
    exporter.AddEntry("OS", "Windows 11");
    exporter.AddEntry("Build", "22631");
    EXPECT_EQ(exporter.SectionCount(), 1u);
    EXPECT_EQ(exporter.TotalEntries(), 2u);
}

TEST(Diagnostics, ExportJSON)
{
    DiagnosticsExporter exporter;
    exporter.AddSection("Info");
    exporter.AddEntry("Version", "7.0.0");
    auto json = exporter.ExportJSON();
    EXPECT_NE(json.find("Info"), std::string::npos);
    EXPECT_NE(json.find("Version"), std::string::npos);
    EXPECT_NE(json.find("7.0.0"), std::string::npos);
    EXPECT_NE(json.find("{"), std::string::npos);
    EXPECT_NE(json.find("}"), std::string::npos);
}

TEST(Diagnostics, ExportText)
{
    DiagnosticsExporter exporter;
    exporter.AddSection("Decoders");
    exporter.AddEntry("JPEG", "Healthy");
    auto text = exporter.ExportText();
    EXPECT_NE(text.find("=== Decoders ==="), std::string::npos);
    EXPECT_NE(text.find("JPEG: Healthy"), std::string::npos);
}

TEST(Diagnostics, Clear)
{
    DiagnosticsExporter exporter;
    exporter.AddSection("Test");
    exporter.AddEntry("Key", "Val");
    exporter.Clear();
    EXPECT_EQ(exporter.SectionCount(), 0u);
    EXPECT_EQ(exporter.TotalEntries(), 0u);
}

TEST(Diagnostics, BuildFullReport)
{
    DecoderHealthDashboard dashboard;
    dashboard.RegisterDecoder("JPEG", {".jpg", ".jpeg"});
    dashboard.RegisterDecoder("PNG", {".png"});
    dashboard.RecordDecode("JPEG", 5.0, true);

    auto exporter = DiagnosticsExporter::BuildFullReport(dashboard);
    EXPECT_GE(exporter.SectionCount(), 2u); // Health + Summary
    EXPECT_GE(exporter.TotalEntries(), 4u);

    auto json = exporter.ExportJSON();
    EXPECT_NE(json.find("Decoder Health"), std::string::npos);
    EXPECT_NE(json.find("Summary"), std::string::npos);
}

TEST(Diagnostics, MultipleSections)
{
    DiagnosticsExporter exporter;
    exporter.AddSection("A");
    exporter.AddEntry("x", "1");
    exporter.AddSection("B");
    exporter.AddEntry("y", "2");
    exporter.AddSection("C");
    exporter.AddEntry("z", "3");
    EXPECT_EQ(exporter.SectionCount(), 3u);
    EXPECT_EQ(exporter.TotalEntries(), 3u);
}

