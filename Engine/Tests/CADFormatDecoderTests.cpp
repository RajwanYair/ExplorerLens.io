// CAD Format Plugin — GTest
#include "../Decoders/CADFormatPlugin.h"
#include "GTestShim.h"

using namespace ExplorerLens::Decoders;

TEST(CADFormatPlugin, SupportedExtensionsNotEmpty)
{
    auto exts = CADFormatPlugin::SupportedExtensions();
    EXPECT_GE(exts.size(), 5u);
}

TEST(CADFormatPlugin, DWGDetection)
{
    EXPECT_EQ(DetectCADFormat(".dwg"), PluginCADFormat::DWG);
}

TEST(CADFormatPlugin, DXFDetection)
{
    EXPECT_EQ(DetectCADFormat(".dxf"), PluginCADFormat::DXF);
}

TEST(CADFormatPlugin, STEPDetection)
{
    EXPECT_EQ(DetectCADFormat(".step"), PluginCADFormat::STEP);
    EXPECT_EQ(DetectCADFormat(".stp"), PluginCADFormat::STEP);
}

TEST(CADFormatPlugin, IGESDetection)
{
    EXPECT_EQ(DetectCADFormat(".iges"), PluginCADFormat::IGES);
    EXPECT_EQ(DetectCADFormat(".igs"), PluginCADFormat::IGES);
}

TEST(CADFormatPlugin, UnknownExtensionReturnsUnknown)
{
    EXPECT_EQ(DetectCADFormat(".xyz"), PluginCADFormat::Unknown);
}

TEST(CADFormatPlugin, CADToStringDWG)
{
    EXPECT_EQ(ToString(PluginCADFormat::DWG), std::string("DWG"));
}

TEST(CADFormatPlugin, BoundingBoxValid)
{
    CADBoundingBox box{0, 0, 10, 10};
    EXPECT_TRUE(box.IsValid());
}

TEST(CADFormatPlugin, BoundingBoxInvalidWidth)
{
    CADBoundingBox box{5, 0, 5, 10};
    EXPECT_FALSE(box.IsValid());
}

TEST(CADFormatPlugin, CADLayerHasName)
{
    CADLayer layer;
    layer.name = "Layer 0";
    EXPECT_FALSE(layer.name.empty());
}

TEST(CADFormatPlugin, FallbackThumbnailForDWG)
{
    auto badge = CADFormatPlugin::GetFallbackThumbnail(".dwg");
    EXPECT_EQ(badge.label, std::string("DWG"));
}

TEST(CADFormatPlugin, IsPluginBacked)
{
    EXPECT_TRUE(CADFormatPlugin::IsPluginBacked());
}

TEST(CADFormatPlugin, CADDecodeResultDefaultFail)
{
    CADDecodeResult r;
    EXPECT_FALSE(r.success);
}

TEST(CADFormatPlugin, DecodeCapabilitiesDefaultFalse)
{
    CADDecodeCapabilities c;
    EXPECT_FALSE(c.canRasterize);
}

TEST(CADFormatPlugin, AspectRatioBoundingBox)
{
    CADBoundingBox box{0, 0, 200, 100};
    EXPECT_DOUBLE_EQ(box.AspectRatio(), 2.0);
}
