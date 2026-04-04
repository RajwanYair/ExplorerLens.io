// Plugin Compatibility Kit V2 — GTestShim
#include "GTestShim.h"
#include "Plugin/PluginCompatibilityKitV2.h"

using namespace ExplorerLens::Plugin;

TEST(PluginCompatibilityKitV2, ABIVersionMajorOnly)
{
    ABIVersion v{2, 0, 0};
    EXPECT_TRUE(v.IsCompatible(ABIVersion{2, 99, 0}));
    EXPECT_FALSE(v.IsCompatible(ABIVersion{3, 0, 0}));
}

TEST(PluginCompatibilityKitV2, V1BaselineHasMandatoryExports)
{
    auto surface = ABIStableSurface::V1Baseline();
    EXPECT_GE(surface.symbols.size(), 5u);
}

TEST(PluginCompatibilityKitV2, ConformanceCheckResultDefaultFail)
{
    ConformanceCheckResult r{ConformanceCheckId::ABIVersionMatch};
    EXPECT_FALSE(r.passed);
}

TEST(PluginCompatibilityKitV2, PerfGateDecodeThreshold)
{
    PluginPerfGate g;
    EXPECT_GT(g.maxDecodeMs, 0.0);
    EXPECT_LE(g.maxDecodeMs, 200.0);
}

TEST(PluginCompatibilityKitV2, MemoryGatePeakThreshold)
{
    PluginMemoryGate g;
    EXPECT_GT(g.maxPeakBytes, 0u);
    EXPECT_LE(g.maxPeakBytes, 200ULL * 1024 * 1024);
}

TEST(PluginCompatibilityKitV2, ConformanceReportNoFailuresDefault)
{
    PluginConformanceReport r;
    EXPECT_EQ(r.failCount, 0u);
}

TEST(PluginCompatibilityKitV2, ABIDriftDetectorNoDriftOnSameSurface)
{
    auto s1 = ABIStableSurface::V1Baseline();
    auto s2 = ABIStableSurface::V1Baseline();
    ABIDriftDetector det(s1, s2);
    auto result = det.Detect();
    EXPECT_TRUE(result.passed);
}

TEST(PluginCompatibilityKitV2, ABIDriftDetectorDriftOnDifferentSurface)
{
    auto s1 = ABIStableSurface::V1Baseline();
    ABIStableSurface s2;
    s2.version = ABIVersion::V2();
    ABIDriftDetector det(s1, s2);
    auto result = det.Detect();
    EXPECT_FALSE(result.passed);
}

TEST(PluginCompatibilityKitV2, ExportedSymbolRequired)
{
    ExportedSymbol sym{"DT_CreateDecoder", "", true};
    EXPECT_TRUE(sym.isRequired);
}

TEST(PluginCompatibilityKitV2, ConformanceCheckIdEnumCoverage)
{
    auto id = ConformanceCheckId::PerfGate;
    (void)id;
    EXPECT_EQ(static_cast<uint32_t>(id), 3u);
}

TEST(PluginCompatibilityKitV2, PerfGateDefaultThreshold100ms)
{
    PluginPerfGate g;
    EXPECT_DOUBLE_EQ(g.maxDecodeMs, 100.0);
}

TEST(PluginCompatibilityKitV2, MemGateDefault50MB)
{
    PluginMemoryGate g;
    EXPECT_EQ(g.maxPeakBytes, 50ULL * 1024 * 1024);
}

TEST(PluginCompatibilityKitV2, ConformanceReportPassWhenNone)
{
    PluginConformanceReport r;
    EXPECT_TRUE(r.AllPassed());
}

TEST(PluginCompatibilityKitV2, ConformanceReportFailWhenAnyFail)
{
    PluginConformanceReport r;
    r.failCount = 2;
    EXPECT_FALSE(r.AllPassed());
}
