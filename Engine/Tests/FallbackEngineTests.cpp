// Format Fallback Engine — GTest
#include "../Pipeline/FormatFallbackEngine.h"
#include "GTestShim.h"

using namespace ExplorerLens::Pipeline;

TEST(FormatFallbackEngine, DefaultEngineHasChains) {
 auto engine = FormatFallbackEngine::CreateDefault();
 EXPECT_GE(engine.chains.size(), 3u);
}

TEST(FormatFallbackEngine, JXLChainExists) {
 auto engine = FormatFallbackEngine::CreateDefault();
 EXPECT_NE(engine.FindChain(".jxl"), nullptr);
}

TEST(FormatFallbackEngine, HEICChainExists) {
 auto engine = FormatFallbackEngine::CreateDefault();
 EXPECT_NE(engine.FindChain(".heic"), nullptr);
}

TEST(FormatFallbackEngine, RAWChainExists) {
 auto engine = FormatFallbackEngine::CreateDefault();
 EXPECT_NE(engine.FindChain(".raw"), nullptr);
}

TEST(FormatFallbackEngine, MissingExtensionReturnsNull) {
 auto engine = FormatFallbackEngine::CreateDefault();
 EXPECT_EQ(engine.FindChain(".notexist"), nullptr);
}

TEST(FormatFallbackEngine, FallbackTriggerToStringNotEmpty) {
 EXPECT_FALSE(ToString(FallbackTrigger::DecodeFailed).empty());
}

TEST(FormatFallbackEngine, TriggerORCombines) {
 auto combined = FallbackTrigger::DecodeFailed | FallbackTrigger::Timeout;
 (void)combined;
 EXPECT_TRUE(HasTrigger(combined, FallbackTrigger::Timeout));
 EXPECT_TRUE(HasTrigger(combined, FallbackTrigger::DecodeFailed));
}

TEST(FormatFallbackEngine, JXLChainHasTerminal) {
 auto engine = FormatFallbackEngine::CreateDefault();
 auto *chain = engine.FindChain(".jxl");
 (void)chain;
 ASSERT_NE(chain, nullptr);
 EXPECT_TRUE(chain->HasTerminal());
}

TEST(FormatFallbackEngine, FallbackEventLogLine) {
 FallbackEvent e;
 e.extension = ".jxl";
 e.primaryDecoder = "JXLDecoder";
 e.usedDecoder = "WICDecoder";
 e.trigger = FallbackTrigger::DecodeFailed;
 e.stagesTraversed = 1;
 EXPECT_FALSE(e.ToLogLine().empty());
}

TEST(FormatFallbackEngine, ChainSelectForDecodeFailed) {
 auto engine = FormatFallbackEngine::CreateDefault();
 auto *chain = engine.FindChain(".jxl");
 ASSERT_NE(chain, nullptr);
 // Primary stage (no trigger) should be selected for no-error case
 auto *stage = chain->SelectForTrigger(FallbackTrigger::None);
 (void)stage;
 EXPECT_NE(stage, nullptr);
}

TEST(FormatFallbackEngine, FallbackStageQualityScore) {
 FallbackStage s;
 s.qualityScore = 100;
 EXPECT_EQ(s.qualityScore, 100u);
}

TEST(FormatFallbackEngine, HasTriggerNoneAlwaysFalse) {
 EXPECT_FALSE(
 HasTrigger(FallbackTrigger::None, FallbackTrigger::DecodeFailed));
}

TEST(FormatFallbackEngine, TelemetryEnabledByDefault) {
 auto engine = FormatFallbackEngine::CreateDefault();
 EXPECT_TRUE(engine.enableTelemetry);
}
