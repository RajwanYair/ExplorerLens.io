// Plugin Sandbox Policy — GTestShim
#include "GTestShim.h"
#include "Plugin/PluginSandboxPolicy.h"

using namespace ExplorerLens::Plugin;

TEST(PluginSandboxPolicy, StrictPresetLowMemory) {
 auto p = SandboxPolicySpec::Strict();
 EXPECT_LE(p.limits.maxMemoryBytes, 64ULL * 1024 * 1024);
}

TEST(PluginSandboxPolicy, StandardPresetMediumMemory) {
 auto p = SandboxPolicySpec::Standard();
 EXPECT_GT(p.limits.maxMemoryBytes, 64ULL * 1024 * 1024);
 EXPECT_LE(p.limits.maxMemoryBytes, 256ULL * 1024 * 1024);
}

TEST(PluginSandboxPolicy, DeveloperPresetHighMemory) {
 auto p = SandboxPolicySpec::Developer();
 EXPECT_GT(p.limits.maxMemoryBytes, 256ULL * 1024 * 1024);
}

TEST(PluginSandboxPolicy, StrictPresetNoUI) {
 auto p = SandboxPolicySpec::Strict();
 EXPECT_FALSE(p.limits.allowUIAccess);
}

TEST(PluginSandboxPolicy, DeveloperPresetAllowsUI) {
 auto p = SandboxPolicySpec::Developer();
 EXPECT_TRUE(p.limits.allowUIAccess);
}

TEST(PluginSandboxPolicy, TimeoutKillChainTotalWithinBound) {
 TimeoutKillChain chain;
 uint32_t total = chain.TotalBudgetMs();
 (void)total;
 EXPECT_LE(total, TimeoutKillChain::kMaxKillTimeMs);
}

TEST(PluginSandboxPolicy, HandleLeakReportDefaultsZero) {
 HandleLeakReport r;
 EXPECT_EQ(r.handleCountAtStart, 0u);
 EXPECT_EQ(r.handleCountAtEnd, 0u);
 EXPECT_EQ(r.leakCount, 0u);
}

TEST(PluginSandboxPolicy, TeardownResultDefaultClean) {
 SandboxTeardownResult r;
 EXPECT_TRUE(r.WasClean());
}

TEST(PluginSandboxPolicy, ValidatorStrictPassesStrictPlugin) {
 auto policy = SandboxPolicySpec::Strict();
 SandboxPolicyValidator v(policy);
 EXPECT_TRUE(v.IsValid());
}

TEST(PluginSandboxPolicy, PolicyPresetEnumCoverage) {
 EXPECT_EQ(static_cast<uint32_t>(SandboxPolicyPreset::Strict), 0u);
 EXPECT_EQ(static_cast<uint32_t>(SandboxPolicyPreset::Standard), 1u);
 EXPECT_EQ(static_cast<uint32_t>(SandboxPolicyPreset::Developer), 2u);
 EXPECT_EQ(static_cast<uint32_t>(SandboxPolicyPreset::Disabled), 3u);
}

TEST(PluginSandboxPolicy, JobObjectLimitsDefaults) {
 SandboxJobLimits lim;
 EXPECT_GT(lim.maxMemoryBytes, 0u);
}

TEST(PluginSandboxPolicy, TeardownReasonEnumCoverage) {
 auto r = TeardownReason::TimeoutKill;
 (void)r;
 EXPECT_EQ(static_cast<uint32_t>(r), 1u);
}

TEST(PluginSandboxPolicy, PolicyViolationHasRule) {
 PolicyViolation v;
 v.rule = "exceeded-memory";
 EXPECT_FALSE(v.rule.empty());
}
