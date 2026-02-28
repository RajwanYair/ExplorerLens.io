#include <gtest/gtest.h>
#include "Utils/WindowsUI.h"
using namespace ExplorerLens::Utils;

TEST(Sprint142_DPI, ScaleFactor_100) {
    EXPECT_DOUBLE_EQ(DPIScaleFactor(DPIScale::S100), 1.0);
}
TEST(Sprint142_DPI, ScaleFactor_200) {
    EXPECT_DOUBLE_EQ(DPIScaleFactor(DPIScale::S200), 2.0);
}
TEST(Sprint142_DPI, ScaleName_Coverage) {
    EXPECT_STREQ(DPIScaleName(DPIScale::S100), "100%");
    EXPECT_STREQ(DPIScaleName(DPIScale::S150), "150%");
    EXPECT_STREQ(DPIScaleName(DPIScale::S300), "300%");
}
TEST(Sprint142_DPI, LayoutRect_Scaled) {
    LayoutRect r{10, 20, 100, 50};
    auto s = r.Scaled(2.0);
    EXPECT_EQ(s.x, 20); EXPECT_EQ(s.y, 40);
    EXPECT_EQ(s.width, 200); EXPECT_EQ(s.height, 100);
}
TEST(Sprint142_DPI, ScaledFont_Create) {
    auto f = ScaledFont::Create("Segoe UI", 9, 2.0);
    EXPECT_EQ(f.baseSizePt, 9);
    EXPECT_EQ(f.scaledSizePt, 18);
}
TEST(Sprint142_DPI, DPIChangeEvent_ScaleRatio) {
    DPIChangeEvent evt;
    evt.oldDPI = DPIScale::S100;
    evt.newDPI = DPIScale::S200;
    EXPECT_DOUBLE_EQ(evt.ScaleRatio(), 2.0);
    EXPECT_TRUE(evt.IsUpscale());
    EXPECT_FALSE(evt.IsDownscale());
}
TEST(Sprint142_DPI, Manager_RegisterMonitor) {
    auto mgr = PerMonitorDPIManager::Create();
    MonitorInfo m1; m1.handle = 1; m1.currentDPI = DPIScale::S100;  m1.isPrimary = true;
    MonitorInfo m2; m2.handle = 2; m2.currentDPI = DPIScale::S150;
    mgr.RegisterMonitor(m1);
    mgr.RegisterMonitor(m2);
    EXPECT_EQ(mgr.MonitorCount(), 2u);
}
TEST(Sprint142_DPI, Manager_GetMonitorDPI) {
    auto mgr = PerMonitorDPIManager::Create();
    MonitorInfo m; m.handle = 1; m.currentDPI = DPIScale::S150;
    mgr.RegisterMonitor(m);
    EXPECT_EQ(mgr.GetMonitorDPI(1), DPIScale::S150);
    EXPECT_EQ(mgr.GetMonitorDPI(99), DPIScale::S100); // not found
}
TEST(Sprint142_DPI, Manager_HandleDPIChange) {
    auto mgr = PerMonitorDPIManager::Create();
    MonitorInfo m; m.handle = 1; m.currentDPI = DPIScale::S100;
    mgr.RegisterMonitor(m);
    auto evt = mgr.HandleDPIChange(1, DPIScale::S200);
    EXPECT_EQ(evt.oldDPI, DPIScale::S100);
    EXPECT_EQ(evt.newDPI, DPIScale::S200);
    EXPECT_EQ(mgr.ChangeCount(), 1u);
    EXPECT_EQ(mgr.GetMonitorDPI(1), DPIScale::S200);
}
TEST(Sprint142_DPI, Manager_Callback) {
    auto mgr = PerMonitorDPIManager::Create();
    MonitorInfo m; m.handle = 1; m.currentDPI = DPIScale::S100;
    mgr.RegisterMonitor(m);
    bool called = false;
    mgr.OnDPIChange([&](const DPIChangeEvent& e) { called = true; });
    mgr.HandleDPIChange(1, DPIScale::S150);
    EXPECT_TRUE(called);
}
TEST(Sprint142_DPI, Manager_ScaleRect) {
    auto mgr = PerMonitorDPIManager::Create();
    LayoutRect base{0, 0, 100, 50};
    auto scaled = mgr.ScaleRect(base, DPIScale::S100, DPIScale::S200);
    EXPECT_EQ(scaled.width, 200);
    EXPECT_EQ(scaled.height, 100);
}
TEST(Sprint142_DPI, Manager_ScaleFont_Clamped) {
    DPIConfig cfg;
    cfg.maxFontSizePt = 20;
    auto mgr = PerMonitorDPIManager::Create(cfg);
    auto f = mgr.ScaleFont("Segoe UI", 9, DPIScale::S300);
    EXPECT_LE(f.scaledSizePt, 20);
}
TEST(Sprint142_DPI, Manager_Summary) {
    auto mgr = PerMonitorDPIManager::Create();
    auto s = mgr.Summary();
    EXPECT_NE(s.find("PerMonitorDPI"), std::string::npos);
}

