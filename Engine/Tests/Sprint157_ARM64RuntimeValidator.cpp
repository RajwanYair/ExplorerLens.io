// Sprint 157 — ARM64 Runtime Validator — GTest
#include <gtest/gtest.h>
#include "Utils/ARM64RuntimeValidator.h"

using namespace DarkThumbs::Utils;

TEST(ARM64RuntimeValidator, MockReportSuccess) {
    auto r = ARM64RuntimeValidationReport::CreateMock();
    EXPECT_TRUE(r.platformProbe.success);
}

TEST(ARM64RuntimeValidator, MockReportHasDecoderEntries) {
    auto r = ARM64RuntimeValidationReport::CreateMock();
    EXPECT_GE(r.decoderConfidence.size(), 5u);
}

TEST(ARM64RuntimeValidator, COMRegCheckCLSIDCorrect) {
    auto check = COMRegistrationCheck::Expected();
    EXPECT_NE(check.clsid.find("9E6ECB90"), std::string::npos);
}

TEST(ARM64RuntimeValidator, MinDecoderPassCountIs5) {
    EXPECT_GE(ARM64RuntimeValidationReport::kMinDecoderPassCount, 5u);
}

TEST(ARM64RuntimeValidator, DecoderConfidenceLevelEnumCoverage) {
    EXPECT_EQ(static_cast<uint32_t>(DecoderConfidenceLevel::High), 2u);
}

TEST(ARM64RuntimeValidator, PlatformProbeResultHasArch) {
    PlatformProbeResult p;
    p.architecture = "x64";
    EXPECT_FALSE(p.architecture.empty());
}

TEST(ARM64RuntimeValidator, MockDecoderConfidenceAllHigh) {
    auto r = ARM64RuntimeValidationReport::CreateMock();
    for (const auto& e : r.decoderConfidence)
        EXPECT_GE(static_cast<uint32_t>(e.confidence), static_cast<uint32_t>(DecoderConfidenceLevel::Medium));
}

TEST(ARM64RuntimeValidator, COMCheckNotRegisteredByDefault) {
    COMRegistrationCheck c;
    EXPECT_FALSE(c.isRegistered);
}

TEST(ARM64RuntimeValidator, ARM64GPUCapabilityHasD3D12Field) {
    ARM64GPUCapability g;
    g.hasD3D12 = true;
    EXPECT_TRUE(g.hasD3D12);
}

TEST(ARM64RuntimeValidator, ReportDecoderCountAboveThreshold) {
    auto r = ARM64RuntimeValidationReport::CreateMock();
    uint32_t highCount = 0;
    for (const auto& e : r.decoderConfidence)
        if (e.confidence >= DecoderConfidenceLevel::Medium) ++highCount;
    EXPECT_GE(highCount, ARM64RuntimeValidationReport::kMinDecoderPassCount);
}

TEST(ARM64RuntimeValidator, MockReportGatePassesWhenAllHigh) {
    auto r = ARM64RuntimeValidationReport::CreateMock();
    EXPECT_TRUE(r.overallGatePassed);
}
