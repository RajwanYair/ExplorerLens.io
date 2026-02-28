#include <gtest/gtest.h>
#include "Utils/WindowsUI.h"
using namespace ExplorerLens::Utils;

TEST(Sprint141_DarkMode, ThemeColor_FromRGB) {
    auto c = ThemeColor::FromRGB(30, 30, 30);
    EXPECT_EQ(c.r, 30); EXPECT_EQ(c.g, 30); EXPECT_EQ(c.b, 30);
    EXPECT_EQ(c.a, 255);
}
TEST(Sprint141_DarkMode, ThemeColor_ToColorRef) {
    auto c = ThemeColor::FromRGB(0xFF, 0, 0);
    EXPECT_EQ(c.ToColorRef(), 0x000000FFu);
}
TEST(Sprint141_DarkMode, Palette_Dark_ElementCount) {
    auto p = ThemePalette::Dark();
    EXPECT_EQ(p.ElementCount(), 14u);
}
TEST(Sprint141_DarkMode, Palette_Light_HasBrightBG) {
    auto p = ThemePalette::Light();
    auto bg = p.Get(ThemeElement::Background);
    EXPECT_EQ(bg.r, 255);
}
TEST(Sprint141_DarkMode, ControlTypeName_Coverage) {
    EXPECT_STREQ(ControlTypeName(ControlType::Button), "Button");
    EXPECT_STREQ(ControlTypeName(ControlType::ListView), "ListView");
    EXPECT_STREQ(ControlTypeName(ControlType::StatusBar), "StatusBar");
}
TEST(Sprint141_DarkMode, ThemeModeName_Coverage) {
    EXPECT_STREQ(ThemeModeName(ThemeMode::Dark), "Dark");
    EXPECT_STREQ(ThemeModeName(ThemeMode::FollowSystem), "FollowSystem");
}
TEST(Sprint141_DarkMode, Manager_CreateDark) {
    auto mgr = DarkModeManagerV2::Create(ThemeMode::Dark);
    EXPECT_EQ(mgr.Mode(), ThemeMode::Dark);
    EXPECT_TRUE(mgr.IsDark());
}
TEST(Sprint141_DarkMode, Manager_CreateLight) {
    auto mgr = DarkModeManagerV2::Create(ThemeMode::Light);
    EXPECT_FALSE(mgr.IsDark());
}
TEST(Sprint141_DarkMode, Manager_RegisterAndApply) {
    auto mgr = DarkModeManagerV2::Create();
    EXPECT_TRUE(mgr.RegisterControl(100, ControlType::Button));
    EXPECT_TRUE(mgr.RegisterControl(200, ControlType::EditBox));
    EXPECT_FALSE(mgr.RegisterControl(100, ControlType::Button)); // duplicate
    EXPECT_EQ(mgr.RegisteredControlCount(), 2u);
    auto applied = mgr.ApplyTheme();
    EXPECT_EQ(applied, 2u);
    EXPECT_EQ(mgr.ThemedControlCount(), 2u);
}
TEST(Sprint141_DarkMode, Manager_SwitchMode) {
    auto mgr = DarkModeManagerV2::Create(ThemeMode::Dark);
    mgr.RegisterControl(100, ControlType::Button);
    mgr.ApplyTheme();
    EXPECT_EQ(mgr.ThemedControlCount(), 1u);
    mgr.SwitchMode(ThemeMode::Light);
    EXPECT_EQ(mgr.ThemedControlCount(), 0u); // reset after switch
    EXPECT_EQ(mgr.Mode(), ThemeMode::Light);
}
TEST(Sprint141_DarkMode, Manager_CustomPalette) {
    auto mgr = DarkModeManagerV2::Create();
    ThemePalette custom;
    custom.Set(ThemeElement::Background, ThemeColor::FromRGB(10, 10, 10));
    mgr.SetCustomPalette(custom);
    EXPECT_EQ(mgr.Mode(), ThemeMode::Custom);
    EXPECT_TRUE(mgr.IsDark());
}
TEST(Sprint141_DarkMode, Manager_Summary) {
    auto mgr = DarkModeManagerV2::Create(ThemeMode::Dark);
    mgr.RegisterControl(100, ControlType::Button);
    auto s = mgr.Summary();
    EXPECT_NE(s.find("Dark"), std::string::npos);
    EXPECT_NE(s.find("controls=1"), std::string::npos);
}
TEST(Sprint141_DarkMode, AllControlTypesSupported) {
    EXPECT_TRUE(DarkModeManagerV2::IsControlSupported(ControlType::Button));
    EXPECT_TRUE(DarkModeManagerV2::IsControlSupported(ControlType::StatusBar));
}

