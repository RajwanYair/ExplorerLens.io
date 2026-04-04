// Format Fallback Engine — GTest (rewritten for Core/FormatFallbackEngine.h API)
#include "../Core/FormatFallbackEngine.h"
#include "GTestShim.h"

using namespace ExplorerLens::Engine;

TEST(FormatFallbackEngine, DefaultEngineHasChains)
{
    FormatFallbackEngine engine;
    // Register decoders for .jpg
    engine.RegisterDecoder(L".jpg", {1, "JpegDecoder", 0, true});
    engine.RegisterDecoder(L".jpg", {2, "WICDecoder", 10, true});
    auto chain = engine.GetChain(L".jpg");
    EXPECT_GE(chain.size(), 2u);
}

TEST(FormatFallbackEngine, JXLChainExists)
{
    FormatFallbackEngine engine;
    engine.RegisterDecoder(L".jxl", {1, "JXLDecoder", 0, true});
    auto chain = engine.GetChain(L".jxl");
    EXPECT_EQ(chain.size(), 1u);
}

TEST(FormatFallbackEngine, HEICChainExists)
{
    FormatFallbackEngine engine;
    engine.RegisterDecoder(L".heic", {1, "HEIFDecoder", 0, true});
    auto chain = engine.GetChain(L".heic");
    EXPECT_EQ(chain.size(), 1u);
}

TEST(FormatFallbackEngine, RAWChainExists)
{
    FormatFallbackEngine engine;
    engine.RegisterDecoder(L".raw", {1, "RAWDecoder", 0, true});
    auto chain = engine.GetChain(L".raw");
    EXPECT_EQ(chain.size(), 1u);
}

TEST(FormatFallbackEngine, MissingExtensionReturnsNull)
{
    FormatFallbackEngine engine;
    auto result = engine.SelectDecoder(L".notexist", nullptr, 0);
    EXPECT_EQ(result, 0u);
}

TEST(FormatFallbackEngine, SelectDecoderReturnsFirst)
{
    FormatFallbackEngine engine;
    engine.RegisterDecoder(L".jpg", {10, "JpegDecoder", 0, true});
    engine.RegisterDecoder(L".jpg", {20, "WICDecoder", 10, true});
    auto id = engine.SelectDecoder(L".jpg", nullptr, 0);
    EXPECT_EQ(id, 10u);
}

TEST(FormatFallbackEngine, SelectDecoderMagicBytes)
{
    FormatFallbackEngine engine;
    engine.RegisterDecoder(L".jpg", {10, "JpegDecoder", 0, true});
    // JPEG magic: FF D8 FF
    uint8_t jpegHeader[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10};
    auto id = engine.SelectDecoder(L".jpg", jpegHeader, sizeof(jpegHeader));
    EXPECT_EQ(id, 10u);
}

TEST(FormatFallbackEngine, GetStatsInitiallyZero)
{
    FormatFallbackEngine engine;
    auto stats = engine.GetStats();
    EXPECT_EQ(stats.totalAttempts, 0u);
    EXPECT_EQ(stats.totalFailures, 0u);
}

TEST(FormatFallbackEngine, RecordResultUpdatesStats)
{
    FormatFallbackEngine engine;
    engine.RegisterDecoder(L".jpg", {1, "JpegDecoder", 0, true});
    engine.RecordDecoderResult(L".jpg", 1, true, 5);
    auto stats = engine.GetStats();
    EXPECT_EQ(stats.totalAttempts, 1u);
    EXPECT_EQ(stats.firstTrySuccesses, 1u);
}

TEST(FormatFallbackEngine, DecoderEntryDefaults)
{
    DecoderEntry e;
    EXPECT_EQ(e.decoderId, 0u);
    EXPECT_EQ(e.priority, 0);
    EXPECT_TRUE(e.enabled);
}

TEST(FormatFallbackEngine, SetFallbackChainReorder)
{
    FormatFallbackEngine engine;
    engine.RegisterDecoder(L".png", {1, "PngPrimary", 0, true});
    engine.RegisterDecoder(L".png", {2, "PngFallback", 10, true});
    // Reverse the order
    engine.SetFallbackChain(L".png", {2, 1});
    auto chain = engine.GetChain(L".png");
    EXPECT_GE(chain.size(), 2u);
    EXPECT_EQ(chain[0].decoderId, 2u);
}

TEST(FormatFallbackEngine, CaseInsensitiveExtension)
{
    FormatFallbackEngine engine;
    engine.RegisterDecoder(L".JPG", {1, "Decoder", 0, true});
    auto chain = engine.GetChain(L".jpg");
    EXPECT_EQ(chain.size(), 1u);
}
