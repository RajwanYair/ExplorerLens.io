// Plugin Reference Pack — GTestShim
#include "GTestShim.h"
#include "Plugin/PluginReferencePack.h"

using namespace ExplorerLens::Plugin;

TEST(PluginReferencePack, MinimalPluginCreated) {
 auto p = MinimalImageGeneratorPlugin();
 EXPECT_FALSE(p.id.empty());
}

TEST(PluginReferencePack, MinimalPluginCapabilityDecode) {
 auto p = MinimalImageGeneratorPlugin();
 uint32_t cap = static_cast<uint32_t>(p.capabilities);
 (void)cap;
 EXPECT_NE(cap & static_cast<uint32_t>(PluginCapability::Decode), 0u);
}

TEST(PluginReferencePack, MetadataPluginCapabilityMetadataOnly) {
 auto p = MetadataOnlyPlugin();
 uint32_t cap = static_cast<uint32_t>(p.capabilities);
 (void)cap;
 EXPECT_NE(cap & static_cast<uint32_t>(PluginCapability::MetadataOnly), 0u);
}

TEST(PluginReferencePack, MetadataPluginExtensionCount) {
 auto p = MetadataOnlyPlugin();
 EXPECT_GE(p.supportedExtensions.size(), 1u);
}

TEST(PluginReferencePack, WatermarkPluginCapabilityPostProcess) {
 auto p = WatermarkPlugin();
 uint32_t cap = static_cast<uint32_t>(p.capabilities);
 (void)cap;
 EXPECT_NE(cap & static_cast<uint32_t>(PluginCapability::PostProcess), 0u);
}

TEST(PluginReferencePack, WatermarkConfigSubtlePreset) {
 auto cfg = WatermarkConfig::Subtle();
 EXPECT_LT(cfg.alpha, (uint8_t)200u);
}

TEST(PluginReferencePack, WatermarkConfigBoldPreset) {
 auto subtle = WatermarkConfig::Subtle();
 auto bold = WatermarkConfig::Bold();
 EXPECT_GT(bold.alpha, subtle.alpha);
}

TEST(PluginReferencePack, ReferencePluginPackAllReturnsThree) {
 ReferencePluginPack pack;
 auto all = pack.All();
 EXPECT_EQ(all.size(), 3u);
}

TEST(PluginReferencePack, ReferencePluginPackFindById) {
 auto minimal = MinimalImageGeneratorPlugin();
 ReferencePluginPack pack;
 auto found = pack.FindById(minimal.id);
 (void)found;
 EXPECT_TRUE(found != nullptr);
 EXPECT_EQ(found->id, minimal.id);
}

TEST(PluginReferencePack, EmbeddedMetadataDefaultEmpty) {
 EmbeddedMetadata m;
 EXPECT_TRUE(m.make.empty());
 EXPECT_FALSE(m.HasGPS());
}

TEST(PluginReferencePack, PluginCapabilityEnumFlagsCombine) {
 auto flags = PluginCapability::Decode | PluginCapability::MetadataOnly;
 uint32_t v = static_cast<uint32_t>(flags);
 (void)v;
 EXPECT_NE(v, 0u);
}

TEST(PluginReferencePack, MinimalPluginWildcardExtension) {
 auto p = MinimalImageGeneratorPlugin();
 EXPECT_FALSE(p.supportedExtensions.empty());
}

TEST(PluginReferencePack, MetadataPluginHasJpgExtension) {
 auto p = MetadataOnlyPlugin();
 bool hasJpg = false;
 for (const auto &ext : p.supportedExtensions)
 if (ext == ".jpg" || ext == ".jpeg")
 hasJpg = true;
 EXPECT_TRUE(hasJpg);
}
