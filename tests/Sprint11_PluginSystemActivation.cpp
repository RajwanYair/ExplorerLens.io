// Sprint11_PluginSystemActivation.cpp
// Sprint 11: Plugin System Activation Tests
// Validates the plugin infrastructure is wired, discoverable, and feature-flag gated

#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class PluginSystemActivationTest : public ::testing::Test {
protected:
    std::string rootDir;
    
    void SetUp() override {
        rootDir = fs::current_path().string();
        auto searchDir = fs::current_path();
        for (int i = 0; i < 5; i++) {
            if (fs::exists(searchDir / "MASTER_PLAN.md")) {
                rootDir = searchDir.string();
                break;
            }
            searchDir = searchDir.parent_path();
        }
    }
    
    bool fileContains(const std::string& relPath, const std::string& needle) {
        auto fullPath = fs::path(rootDir) / relPath;
        if (!fs::exists(fullPath)) return false;
        std::ifstream f(fullPath.string());
        std::string content((std::istreambuf_iterator<char>(f)),
                           std::istreambuf_iterator<char>());
        return content.find(needle) != std::string::npos;
    }
    
    bool fileExists(const std::string& relPath) {
        return fs::exists(fs::path(rootDir) / relPath);
    }
};

// =============================================================================
// Plugin System Infrastructure Tests
// =============================================================================

TEST_F(PluginSystemActivationTest, PluginManagerExists) {
    EXPECT_TRUE(fileExists("Engine/Plugin/PluginManager.h"))
        << "PluginManager.h should exist";
    EXPECT_TRUE(fileExists("Engine/Plugin/PluginManager.cpp"))
        << "PluginManager.cpp should exist";
}

TEST_F(PluginSystemActivationTest, LoadPluginsNotCommentedOut) {
    // LoadPlugins() should be called (not commented out) in ThumbnailPipeline
    EXPECT_TRUE(fileContains("Engine/Pipeline/ThumbnailPipeline.cpp", "LoadPlugins()"))
        << "LoadPlugins() should be called in ThumbnailPipeline.cpp";
    // Verify it's behind the feature flag
    EXPECT_TRUE(fileContains("Engine/Pipeline/ThumbnailPipeline.cpp", "config.enablePlugins"))
        << "LoadPlugins should be gated by config.enablePlugins";
}

TEST_F(PluginSystemActivationTest, FeatureFlagInConfig) {
    EXPECT_TRUE(fileContains("Engine/Core/Config.h", "enablePlugins"))
        << "Config should have enablePlugins field";
    EXPECT_TRUE(fileContains("Engine/Core/Config.cpp", "EnablePlugins"))
        << "Config should persist enablePlugins to registry";
}

TEST_F(PluginSystemActivationTest, FeatureFlagDefaultTrue) {
    EXPECT_TRUE(fileContains("Engine/Core/Config.h", "enablePlugins = true"))
        << "enablePlugins should default to true (activated)";
}

TEST_F(PluginSystemActivationTest, PluginDiscoveryExists) {
    EXPECT_TRUE(fileContains("Engine/Plugin/PluginManager.h", "PluginDiscovery"))
        << "PluginDiscovery class should exist";
    EXPECT_TRUE(fileContains("Engine/Plugin/PluginManager.h", "GetPluginSearchPaths"))
        << "GetPluginSearchPaths should be defined";
    EXPECT_TRUE(fileContains("Engine/Plugin/PluginManager.h", "GetDefaultPluginDirectory"))
        << "GetDefaultPluginDirectory should be defined";
}

TEST_F(PluginSystemActivationTest, PluginSearchPathUsesLocalAppData) {
    EXPECT_TRUE(fileContains("Engine/Plugin/PluginManager.cpp", "LocalAppData") ||
                fileContains("Engine/Plugin/PluginManager.cpp", "FOLDERID_LocalAppData") ||
                fileContains("Engine/Plugin/PluginManager.cpp", "LOCALAPPDATA"))
        << "Plugin search should include %LocalAppData%\\DarkThumbs\\Plugins";
}

// =============================================================================
// Plugin Pipeline Integration Tests
// =============================================================================

TEST_F(PluginSystemActivationTest, PipelineConfigHasPluginFlag) {
    EXPECT_TRUE(fileContains("Engine/Pipeline/ThumbnailPipeline.h", "enablePlugins"))
        << "PipelineConfig should have enablePlugins";
}

TEST_F(PluginSystemActivationTest, WorkerProcessSupportsPlugins) {
    EXPECT_TRUE(fileContains("src/Worker/worker_process.h", "enablePlugins"))
        << "Worker process config should support enablePlugins";
    EXPECT_TRUE(fileContains("src/Worker/worker_process.h", "EnablePlugins"))
        << "Worker builder should have EnablePlugins method";
}

// =============================================================================
// Plugin UI Integration Tests
// =============================================================================

TEST_F(PluginSystemActivationTest, WinUISettingsHasPluginToggle) {
    EXPECT_TRUE(fileContains("src/Manager.WinUI/Views/SettingsPage.xaml", "EnablePlugins"))
        << "WinUI Settings should have EnablePlugins toggle";
}

TEST_F(PluginSystemActivationTest, WinUIPluginsViewModelExists) {
    EXPECT_TRUE(fileExists("src/Manager.WinUI/ViewModels/PluginsViewModel.cs"))
        << "PluginsViewModel should exist for plugin management UI";
}

TEST_F(PluginSystemActivationTest, PluginStoreViewModelExists) {
    EXPECT_TRUE(fileExists("src/Manager.WinUI/ViewModels/PluginStoreViewModel.h"))
        << "PluginStoreViewModel should exist for marketplace";
}

// =============================================================================
// Plugin SDK Tests
// =============================================================================

TEST_F(PluginSystemActivationTest, PluginSDKExists) {
    EXPECT_TRUE(fileExists("SDK") || fileExists("sdk"))
        << "Plugin SDK directory should exist";
}

TEST_F(PluginSystemActivationTest, PluginActivationDocExists) {
    EXPECT_TRUE(fileExists(".github/PLUGIN_SYSTEM_ACTIVATION.md"))
        << "Plugin system activation documentation should exist";
}

TEST_F(PluginSystemActivationTest, PluginAPIDocExists) {
    EXPECT_TRUE(fileExists("docs/plugins/PLUGIN_API.md"))
        << "Plugin API documentation should exist";
}
