// Sprint 153 — Plugin Reference Pack — GTest
#include <gtest/gtest.h>
#include "Plugin/PluginReferencePack.h"

using namespace DarkThumbs::Plugin;

TEST(PluginReferencePack, MinimalPluginCreated) {
    auto p = MinimalImageGeneratorPlugin();
    EXPECT_FALSE(p.pluginId.empty());
}

TEST(PluginReferencePack, MinimalPluginCapabilityGenerate) {
    auto p = MinimalImageGeneratorPlugin();
    uint32_t cap = static_cast<uint32_t>(p.capabilities);
    EXPECT_NE(cap & static_cast<uint32_t>(PluginCapability::Generate), 0u);
}

TEST(PluginReferencePack, MetadataPluginCapabilityMetadata) {
    auto p = MetadataOnlyPlugin();
    uint32_t cap = static_cast<uint32_t>(p.capabilities);
    EXPECT_NE(cap & static_cast<uint32_t>(PluginCapability::Metadata), 0u);
}

TEST(PluginReferencePack, MetadataPluginExtensionCount) {
    auto p = MetadataOnlyPlugin();
    EXPECT_GE(p.supportedExtensions.size(), 1u);
}

TEST(PluginReferencePack, WatermarkPluginCapabilityPostProcess) {
    auto p = WatermarkPlugin();
    uint32_t cap = static_cast<uint32_t>(p.capabilities);
    EXPECT_NE(cap & static_cast<uint32_t>(PluginCapability::PostProcess), 0u);
}

TEST(PluginReferencePack, WatermarkConfigSubtlePreset) {
    auto cfg = WatermarkConfig::Subtle();
    EXPECT_LT(cfg.opacity, 100u);
}

TEST(PluginReferencePack, WatermarkConfigBoldPreset) {
    auto subtle = WatermarkConfig::Subtle();
    auto bold   = WatermarkConfig::Bold();
    EXPECT_GT(bold.opacity, subtle.opacity);
}

TEST(PluginReferencePack, ReferencePluginPackAllReturnsThree) {
    auto all = ReferencePluginPack::All();
    EXPECT_EQ(all.size(), 3u);
}

TEST(PluginReferencePack, ReferencePluginPackFindById) {
    auto minimal = MinimalImageGeneratorPlugin();
    auto found   = ReferencePluginPack::FindById(minimal.pluginId);
    EXPECT_TRUE(found.has_value());
    EXPECT_EQ(found->pluginId, minimal.pluginId);
}

TEST(PluginReferencePack, EmbeddedMetadataDefaultEmpty) {
    EmbeddedMetadata m;
    EXPECT_TRUE(m.fields.empty());
}

TEST(PluginReferencePack, PluginCapabilityEnumFlagsCombine) {
    auto flags = PluginCapability::Generate | PluginCapability::Metadata;
    uint32_t v = static_cast<uint32_t>(flags);
    EXPECT_NE(v, 0u);
}

TEST(PluginReferencePack, MinimalPluginWildcardExtension) {
    auto p = MinimalImageGeneratorPlugin();
    bool hasWildcard = false;
    for (const auto& ext : p.supportedExtensions)
        if (ext == "*") hasWildcard = true;
    EXPECT_TRUE(hasWildcard);
}

TEST(PluginReferencePack, MetadataPluginHasJpgExtension) {
    auto p = MetadataOnlyPlugin();
    bool hasJpg = false;
    for (const auto& ext : p.supportedExtensions)
        if (ext == ".jpg" || ext == ".jpeg") hasJpg = true;
    EXPECT_TRUE(hasJpg);
}
