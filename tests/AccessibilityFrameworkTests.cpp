//==============================================================================
// ExplorerLens — Accessibility & Internationalization
// Tests locale parsing, string tables, localization manager, screen reader
// descriptions, contrast modes, keyboard navigation, accessibility config.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "../Engine/Utils/AccessibilityEngine.h"

using namespace ExplorerLens::Engine::Utils;

//==============================================================================
// Locale Tests
//==============================================================================

TEST(Locale, Tag) {
    Locale loc{ "en", "US" };
    EXPECT_EQ(loc.Tag(), "en-US");
}

TEST(Locale, TagNoRegion) {
    Locale loc{ "en", "" };
    EXPECT_EQ(loc.Tag(), "en");
}

TEST(Locale, IsRTL) {
    EXPECT_TRUE(Locale::Arabic().IsRTL());
    EXPECT_TRUE(Locale::Hebrew().IsRTL());
    EXPECT_FALSE(Locale::English().IsRTL());
    EXPECT_FALSE(Locale::Japanese().IsRTL());
    EXPECT_FALSE(Locale::German().IsRTL());
}

TEST(Locale, ParseWithDash) {
    auto loc = Locale::Parse("en-US");
    EXPECT_EQ(loc.language, "en");
    EXPECT_EQ(loc.region, "US");
}

TEST(Locale, ParseWithUnderscore) {
    auto loc = Locale::Parse("ja_JP");
    EXPECT_EQ(loc.language, "ja");
    EXPECT_EQ(loc.region, "JP");
}

TEST(Locale, ParseLanguageOnly) {
    auto loc = Locale::Parse("de");
    EXPECT_EQ(loc.language, "de");
    EXPECT_TRUE(loc.region.empty());
}

TEST(Locale, IsEmpty) {
    Locale loc;
    EXPECT_TRUE(loc.IsEmpty());
    loc.language = "en";
    EXPECT_FALSE(loc.IsEmpty());
}

TEST(Locale, Presets) {
    EXPECT_EQ(Locale::English().Tag(), "en-US");
    EXPECT_EQ(Locale::Japanese().Tag(), "ja-JP");
    EXPECT_EQ(Locale::German().Tag(), "de-DE");
    EXPECT_EQ(Locale::Hebrew().Tag(), "he-IL");
    EXPECT_EQ(Locale::Arabic().Tag(), "ar-SA");
}

//==============================================================================
// String Table Tests
//==============================================================================

TEST(StringTable, SetAndGet) {
    StringTable t;
    t.Set("key1", "value1");
    EXPECT_EQ(t.Get("key1"), "value1");
    EXPECT_TRUE(t.Has("key1"));
}

TEST(StringTable, MissingKeyFallback) {
    StringTable t;
    EXPECT_EQ(t.Get("missing", "default"), "default");
    EXPECT_EQ(t.Get("missing"), "");
}

TEST(StringTable, Count) {
    StringTable t;
    EXPECT_EQ(t.Count(), 0u);
    t.Set("a", "1");
    t.Set("b", "2");
    EXPECT_EQ(t.Count(), 2u);
}

TEST(StringTable, DefaultEnglish) {
    auto t = StringTable::DefaultEnglish();
    EXPECT_GT(t.Count(), 10u);
    EXPECT_EQ(t.Get("app.name"), "ExplorerLens");
    EXPECT_TRUE(t.Has("action.regenerate"));
    EXPECT_TRUE(t.Has("status.loading"));
}

TEST(StringTable, MissingKeys) {
    auto ref = StringTable::DefaultEnglish();
    StringTable partial;
    partial.Set("app.name", "DT");
    auto missing = partial.MissingKeys(ref);
    EXPECT_GT(missing.size(), 5u);
}

TEST(StringTable, CoveragePercent) {
    auto ref = StringTable::DefaultEnglish();

    // Full coverage
    EXPECT_NEAR(ref.CoveragePercent(ref), 100.0, 0.1);

    // Partial coverage
    StringTable partial;
    partial.Set("app.name", "DT");
    EXPECT_LT(partial.CoveragePercent(ref), 20.0);
}

//==============================================================================
// Localization Manager Tests
//==============================================================================

TEST(LocalizationManager, ResolveEnglish) {
    LocalizationManager mgr;
    mgr.RegisterLocale("en-US", StringTable::DefaultEnglish());
    mgr.SetLocale(Locale::English());

    EXPECT_EQ(mgr.Resolve("app.name"), "ExplorerLens");
    EXPECT_EQ(mgr.Resolve("action.regenerate"), "Regenerate Thumbnail");
}

TEST(LocalizationManager, FallbackToEnglish) {
    LocalizationManager mgr;
    mgr.RegisterLocale("en-US", StringTable::DefaultEnglish());
    mgr.SetLocale(Locale::Japanese()); // No Japanese table registered

    // Should fall back to English
    EXPECT_EQ(mgr.Resolve("app.name"), "ExplorerLens");
}

TEST(LocalizationManager, MissingKeyReturnsKey) {
    LocalizationManager mgr;
    // No tables at all
    EXPECT_EQ(mgr.Resolve("nonexistent.key"), "nonexistent.key");
}

TEST(LocalizationManager, IsRTL) {
    LocalizationManager mgr;
    mgr.SetLocale(Locale::English());
    EXPECT_FALSE(mgr.IsRTL());
    mgr.SetLocale(Locale::Arabic());
    EXPECT_TRUE(mgr.IsRTL());
}

TEST(LocalizationManager, AvailableLocales) {
    LocalizationManager mgr;
    mgr.RegisterLocale("en-US", StringTable::DefaultEnglish());
    EXPECT_EQ(mgr.LocaleCount(), 1u);
    auto locales = mgr.AvailableLocales();
    EXPECT_EQ(locales.size(), 1u);
    EXPECT_EQ(locales[0], "en-US");
}

//==============================================================================
// Accessibility Description Tests
//==============================================================================

TEST(AccessibilityDescription, Empty) {
    AccessibilityDescription d;
    EXPECT_TRUE(d.IsEmpty());
}

TEST(AccessibilityDescription, ForThumbnail) {
    auto d = AccessibilityDescription::ForThumbnail(
        "sunset.heic", "HEIF", 4000, 3000, "2.5 MB");
    EXPECT_EQ(d.name, "sunset.heic");
    EXPECT_EQ(d.role, "image");
    EXPECT_NE(d.description.find("HEIF"), std::string::npos);
    EXPECT_NE(d.description.find("4000x3000"), std::string::npos);
    EXPECT_FALSE(d.IsEmpty());
}

TEST(AccessibilityDescription, ForBadge) {
    auto d = AccessibilityDescription::ForBadge("JXL", "Format badge");
    EXPECT_EQ(d.name, "JXL");
    EXPECT_EQ(d.role, "status");
}

TEST(AccessibilityDescription, NarratorText) {
    AccessibilityDescription d;
    d.name = "photo.jpg";
    d.description = "JPEG image, 1920x1080";
    d.value = "Format: JPEG";
    auto text = d.NarratorText();
    EXPECT_NE(text.find("photo.jpg"), std::string::npos);
    EXPECT_NE(text.find("JPEG image"), std::string::npos);
    EXPECT_NE(text.find("Format: JPEG"), std::string::npos);
}

TEST(AccessibilityDescription, NarratorTextMinimal) {
    AccessibilityDescription d;
    d.name = "icon.png";
    auto text = d.NarratorText();
    EXPECT_EQ(text, "icon.png");
}

//==============================================================================
// Contrast Mode Tests
//==============================================================================

TEST(ContrastMode, Names) {
    EXPECT_STREQ(ContrastModeName(ContrastMode::Standard), "Standard");
    EXPECT_STREQ(ContrastModeName(ContrastMode::HighContrast), "High Contrast");
    EXPECT_STREQ(ContrastModeName(ContrastMode::DarkMode), "Dark Mode");
    EXPECT_STREQ(ContrastModeName(ContrastMode::Custom), "Custom");
}

TEST(ContrastConfig, Standard) {
    auto c = ContrastConfig::Standard();
    EXPECT_EQ(c.mode, ContrastMode::Standard);
    EXPECT_TRUE(c.MeetsWCAGAA());
}

TEST(ContrastConfig, HighContrast) {
    auto c = ContrastConfig::HighContrast();
    EXPECT_EQ(c.mode, ContrastMode::HighContrast);
    EXPECT_TRUE(c.MeetsWCAGAAA());
    EXPECT_FLOAT_EQ(c.badgeOpacity, 1.0f);
}

TEST(ContrastConfig, DarkMode) {
    auto c = ContrastConfig::DarkMode();
    EXPECT_EQ(c.mode, ContrastMode::DarkMode);
    EXPECT_TRUE(c.MeetsWCAGAA());
}

//==============================================================================
// Keyboard Navigation Tests
//==============================================================================

TEST(KeyboardNavigation, ThumbnailGrid) {
    auto n = KeyboardNavigation::ForThumbnailGrid();
    EXPECT_TRUE(n.tabStopEnabled);
    EXPECT_TRUE(n.arrowKeyNavigation);
}

TEST(KeyboardNavigation, ContextMenu) {
    auto n = KeyboardNavigation::ForContextMenu();
    EXPECT_FALSE(n.tabStopEnabled);
    EXPECT_TRUE(n.arrowKeyNavigation);
}

//==============================================================================
// Accessibility Config Tests
//==============================================================================

TEST(AccessibilityConfig, Default) {
    auto c = AccessibilityConfig::Default();
    EXPECT_FALSE(c.screenReaderEnabled);
    EXPECT_FALSE(c.reduceMotion);
    EXPECT_FLOAT_EQ(c.fontScale, 1.0f);
}

TEST(AccessibilityConfig, ScreenReaderOptimized) {
    auto c = AccessibilityConfig::ScreenReaderOptimized();
    EXPECT_TRUE(c.screenReaderEnabled);
    EXPECT_TRUE(c.reduceMotion);
    EXPECT_TRUE(c.announceStatusChanges);
    EXPECT_EQ(c.contrast.mode, ContrastMode::HighContrast);
}

TEST(AccessibilityConfig, LowVision) {
    auto c = AccessibilityConfig::LowVision();
    EXPECT_TRUE(c.largeFonts);
    EXPECT_FLOAT_EQ(c.fontScale, 1.5f);
    EXPECT_EQ(c.contrast.mode, ContrastMode::HighContrast);
}
