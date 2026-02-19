// Sprint 151 — Plugin Sandbox Policy — GTest
#include <gtest/gtest.h>
#include "Plugin/PluginSandboxPolicy.h"

using namespace DarkThumbs::Plugin;

TEST(PluginSandboxPolicy, StrictPresetLowMemory) {
    auto p = SandboxPolicy::Strict();
    EXPECT_LE(p.limits.maxMemoryBytes, 64ULL * 1024 * 1024);
}

TEST(PluginSandboxPolicy, StandardPresetMediumMemory) {
    auto p = SandboxPolicy::Standard();
    EXPECT_GT(p.limits.maxMemoryBytes, 64ULL * 1024 * 1024);
    EXPECT_LE(p.limits.maxMemoryBytes, 256ULL * 1024 * 1024);
}

TEST(PluginSandboxPolicy, DeveloperPresetHighMemory) {
    auto p = SandboxPolicy::Developer();
    EXPECT_GT(p.limits.maxMemoryBytes, 256ULL * 1024 * 1024);
}

TEST(PluginSandboxPolicy, StrictPresetNoUI) {
    auto p = SandboxPolicy::Strict();
    EXPECT_FALSE(p.limits.allowUIThread);
}

TEST(PluginSandboxPolicy, DeveloperPresetAllowsUI) {
    auto p = SandboxPolicy::Developer();
    EXPECT_TRUE(p.limits.allowUIThread);
}

TEST(PluginSandboxPolicy, TimeoutKillChainTotalWithinBound) {
    auto chain = TimeoutKillChain::Default();
    uint32_t total = chain.softSignalMs + chain.drainMs + chain.forceKillMs;
    EXPECT_LE(total, TimeoutKillChain::kMaxKillTimeMs);
}

TEST(PluginSandboxPolicy, HandleLeakReportDefaultsZero) {
    HandleLeakReport r;
    EXPECT_EQ(r.handleCount, 0u);
}

TEST(PluginSandboxPolicy, TeardownResultDefaultNotSuccess) {
    SandboxTeardownResult r;
    EXPECT_FALSE(r.success);
}

TEST(PluginSandboxPolicy, ValidatorStrictPassesStrictPlugin) {
    SandboxPolicyValidator v;
    auto policy = SandboxPolicy::Strict();
    auto result = v.Validate(policy);
    EXPECT_TRUE(result.isValid);
}

TEST(PluginSandboxPolicy, PolicyPresetEnumCoverage) {
    EXPECT_EQ(static_cast<uint32_t>(SandboxPolicyPreset::Strict),   0u);
    EXPECT_EQ(static_cast<uint32_t>(SandboxPolicyPreset::Standard),  1u);
    EXPECT_EQ(static_cast<uint32_t>(SandboxPolicyPreset::Developer), 2u);
    EXPECT_EQ(static_cast<uint32_t>(SandboxPolicyPreset::Disabled),  3u);
}

TEST(PluginSandboxPolicy, JobObjectLimitsDefaults) {
    JobObjectLimits lim;
    EXPECT_GT(lim.maxMemoryBytes, 0u);
}

TEST(PluginSandboxPolicy, TeardownReasonEnumCoverage) {
    auto r = TeardownReason::Timeout;
    EXPECT_EQ(static_cast<uint32_t>(r), 1u);
}

TEST(PluginSandboxPolicy, PolicyViolationHasMessage) {
    PolicyViolation v;
    v.message = "exceeded memory";
    EXPECT_FALSE(v.message.empty());
}
