// Sprint 162 — glTF / GLB 3D Model Decoder — GTest
#include <gtest/gtest.h>
#include "Decoders/GLTFModelDecoder.h"

using namespace DarkThumbs::Decoders;

TEST(GLTFModelDecoder, SupportedExtensionsHasGLTF) {
    auto exts = GLTFModelDecoder::SupportedExtensions();
    bool found = false;
    for (const auto& e : exts) if (e == ".gltf") found = true;
    EXPECT_TRUE(found);
}

TEST(GLTFModelDecoder, SupportedExtensionsHasGLB) {
    auto exts = GLTFModelDecoder::SupportedExtensions();
    bool found = false;
    for (const auto& e : exts) if (e == ".glb") found = true;
    EXPECT_TRUE(found);
}

TEST(GLTFModelDecoder, DetectVariantGLB) {
    EXPECT_EQ(GLTFModelDecoder::DetectVariant(".glb"), GLTFVariant::GLB);
}

TEST(GLTFModelDecoder, DetectVariantGLTF) {
    EXPECT_EQ(GLTFModelDecoder::DetectVariant(".gltf"), GLTFVariant::GLTF);
}

TEST(GLTFModelDecoder, AABB3DMaxExtent) {
    AABB3D box{ -1, -1, -1, 1, 1, 1 };
    EXPECT_FLOAT_EQ(box.MaxExtent(), 2.0f);
}

TEST(GLTFModelDecoder, AABB3DCenter) {
    AABB3D box{ 0, 0, 0, 2, 2, 2 };
    auto c = box.Center();
    EXPECT_FLOAT_EQ(c[0], 1.0f);
    EXPECT_FLOAT_EQ(c[1], 1.0f);
    EXPECT_FLOAT_EQ(c[2], 1.0f);
}

TEST(GLTFModelDecoder, MeshComplexityThreshold) {
    MeshComplexity c;
    c.totalTriangles = 100001;
    EXPECT_TRUE(c.IsComplex());
}

TEST(GLTFModelDecoder, MeshNotComplex) {
    MeshComplexity c;
    c.totalTriangles = 500;
    EXPECT_FALSE(c.IsComplex());
}

TEST(GLTFModelDecoder, CameraDefaultForScene) {
    AABB3D box{ -1, -1, -1, 1, 1, 1 };
    auto cam = Camera3D::DefaultForScene(box);
    EXPECT_FLOAT_EQ(cam.target[0], 0.0f);
}

TEST(GLTFModelDecoder, RenderPathToStringNotEmpty) {
    EXPECT_FALSE(ToString(GLTFRenderPath::FullPBR).empty());
}

TEST(GLTFModelDecoder, DecodeSuccessForNonEmptyData) {
    GLTFModelDecoder dec;
    std::vector<uint8_t> data(16, 0xA0);
    auto res = dec.Decode(data.data(), data.size(), 256, 256, true);
    EXPECT_TRUE(res.success);
}

TEST(GLTFModelDecoder, DecodeFailForEmptyData) {
    GLTFModelDecoder dec;
    auto res = dec.Decode(nullptr, 0, 256, 256, true);
    EXPECT_FALSE(res.success);
}

TEST(GLTFModelDecoder, DecodeOutputWH) {
    GLTFModelDecoder dec;
    std::vector<uint8_t> data(32, 0xB0);
    auto res = dec.Decode(data.data(), data.size(), 128, 128, true);
    EXPECT_EQ(res.widthPx, 128u);
    EXPECT_EQ(res.heightPx, 128u);
}

TEST(GLTFModelDecoder, DecodeNoD3D11FallsBackToShapeOnly) {
    GLTFModelDecoder dec;
    std::vector<uint8_t> data(32, 0xB1);
    auto res = dec.Decode(data.data(), data.size(), 128, 128, false);
    EXPECT_EQ(res.renderPath, GLTFRenderPath::ShapeOnly);
}

TEST(GLTFModelDecoder, WasFallbackFalseForFullPBR) {
    GLTFDecodeResult r;
    r.renderPath = GLTFRenderPath::FullPBR;
    EXPECT_FALSE(r.WasFallback());
}

TEST(GLTFModelDecoder, WasFallbackTrueForWireframe) {
    GLTFDecodeResult r;
    r.renderPath = GLTFRenderPath::WireframeFallback;
    EXPECT_TRUE(r.WasFallback());
}
