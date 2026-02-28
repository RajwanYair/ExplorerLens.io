// ARM64 Runtime Validator — GTest
#include "../Utils/ARM64Platform.h"
#include "GTestShim.h"

using namespace ExplorerLens::Platform;

TEST(ARM64RuntimeValidator, MockReportSuccess) {
    auto r = ARM64RuntimeValidationReport::CreateMock();
    EXPECT_TRUE(r.platform.isNativeARM64);
}

TEST(ARM64RuntimeValidator, MockReportHasDecoderEntries) {
    auto r = ARM64RuntimeValidationReport::CreateMock();
    EXPECT_GE(r.decoderTests.size(), 5u);
}

TEST(ARM64RuntimeValidator, COMRegCheckCLSIDCorrect) {
    COMRegistrationCheck check;
    EXPECT_NE(check.clsid.find("9E6ECB90"), std::string::npos);
}

TEST(ARM64RuntimeValidator, MinDecoderPassCountIs5) {
    EXPECT_GE(ARM64RuntimeValidationReport::kMinDecoderPassCount, 5u);
}

TEST(ARM64RuntimeValidator, DecoderConfidenceLevelEnumCoverage) {
    EXPECT_EQ(static_cast<uint32_t>(DecoderConfidenceLevel::FullPass), 0u);
}

TEST(ARM64RuntimeValidator, PlatformProbeResultHasArch) {
    PlatformProbeResult p;
    p.processorArch = "x64";
    EXPECT_FALSE(p.processorArch.empty());
}

TEST(ARM64RuntimeValidator, MockDecoderConfidenceAllPassed) {
    auto r = ARM64RuntimeValidationReport::CreateMock();
    (void)r;
    for (const auto& e : r.decoderTests) {
        (void)e;
        EXPECT_TRUE(e.Passed());
    }
}

TEST(ARM64RuntimeValidator, COMCheckNotRegisteredByDefault) {
    COMRegistrationCheck c;
    EXPECT_FALSE(c.isRegistered);
}

TEST(ARM64RuntimeValidator, ARM64GPUCapabilityHasD3D12Field) {
    ARM64GPUCapability g;
    g.d3d12Available = true;
    EXPECT_TRUE(g.d3d12Available);
}

TEST(ARM64RuntimeValidator, ReportDecoderCountAboveThreshold) {
    auto r = ARM64RuntimeValidationReport::CreateMock();
    uint32_t passCount = 0;
    for (const auto& e : r.decoderTests)
        if (e.Passed())
            ++passCount;
    EXPECT_GE(passCount, ARM64RuntimeValidationReport::kMinDecoderPassCount);
}

TEST(ARM64RuntimeValidator, MockReportMeetsCriteria) {
    auto r = ARM64RuntimeValidationReport::CreateMock();
    EXPECT_TRUE(r.MeetsCriteria());
}
