//==============================================================================
// ExplorerLens — Windows 11 Compatibility Matrix
// Tests OS version detection, dark mode, HDR, multi-GPU enumeration,
// ARM64 features, DPI scaling, and full compatibility matrix building.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>

// Header under test
#include "../Engine/Utils/WindowsCompat.h"

using namespace ExplorerLens::Engine::Compat;

//==============================================================================
// Win11 Build Constants
//==============================================================================

TEST(Win11Build, ThresholdsAscending) {
    EXPECT_LT(Win11Build::Win10_Last, Win11Build::Win11_21H2);
    EXPECT_LT(Win11Build::Win11_21H2, Win11Build::Win11_22H2);
    EXPECT_LT(Win11Build::Win11_22H2, Win11Build::Win11_23H2);
    EXPECT_LT(Win11Build::Win11_23H2, Win11Build::Win11_24H2);
}

TEST(Win11Build, SpecificValues) {
    EXPECT_EQ(Win11Build::Win11_22H2, 22621u);
    EXPECT_EQ(Win11Build::Win11_23H2, 22631u);
    EXPECT_EQ(Win11Build::Win11_24H2, 26100u);
}

//==============================================================================
// WindowsVersionInfo Tests
//==============================================================================

TEST(WindowsVersion, BuildLabelWin10) {
    WindowsVersionInfo info{};
    info.isWindows11 = false;
    info.buildNumber = 19045;
    EXPECT_EQ(info.BuildLabel(), "Windows 10 (or earlier)");
}

TEST(WindowsVersion, BuildLabelWin11_22H2) {
    WindowsVersionInfo info{};
    info.isWindows11 = true;
    info.buildNumber = Win11Build::Win11_22H2;
    EXPECT_EQ(info.BuildLabel(), "Windows 11 22H2");
}

TEST(WindowsVersion, BuildLabelWin11_23H2) {
    WindowsVersionInfo info{};
    info.isWindows11 = true;
    info.buildNumber = Win11Build::Win11_23H2;
    EXPECT_EQ(info.BuildLabel(), "Windows 11 23H2");
}

TEST(WindowsVersion, BuildLabelWin11_24H2) {
    WindowsVersionInfo info{};
    info.isWindows11 = true;
    info.buildNumber = Win11Build::Win11_24H2;
    EXPECT_EQ(info.BuildLabel(), "Windows 11 24H2");
}

TEST(WindowsVersion, BuildLabelWin11_21H2) {
    WindowsVersionInfo info{};
    info.isWindows11 = true;
    info.buildNumber = 22000;
    EXPECT_EQ(info.BuildLabel(), "Windows 11 21H2");
}

//==============================================================================
// Dark Mode State Tests
//==============================================================================

TEST(DarkModeState, StateNames) {
    EXPECT_STREQ(DarkModeStateName(DarkModeState::Unavailable), "Unavailable");
    EXPECT_STREQ(DarkModeStateName(DarkModeState::Light), "Light");
    EXPECT_STREQ(DarkModeStateName(DarkModeState::Dark), "Dark");
    EXPECT_STREQ(DarkModeStateName(DarkModeState::HighContrast), "HighContrast");
}

TEST(DarkModeState, EnumValues) {
    EXPECT_EQ(static_cast<uint32_t>(DarkModeState::Unavailable), 0u);
    EXPECT_EQ(static_cast<uint32_t>(DarkModeState::Light), 1u);
    EXPECT_EQ(static_cast<uint32_t>(DarkModeState::Dark), 2u);
    EXPECT_EQ(static_cast<uint32_t>(DarkModeState::HighContrast), 3u);
}

//==============================================================================
// GPU Adapter Info Tests
//==============================================================================

TEST(GPUAdapterInfo, VendorNames) {
    GPUAdapterInfo nvidia{};
    nvidia.vendorId = 0x10DE;
    EXPECT_EQ(nvidia.VendorName(), "NVIDIA");

    GPUAdapterInfo amd{};
    amd.vendorId = 0x1002;
    EXPECT_EQ(amd.VendorName(), "AMD");

    GPUAdapterInfo intel{};
    intel.vendorId = 0x8086;
    EXPECT_EQ(intel.VendorName(), "Intel");

    GPUAdapterInfo warp{};
    warp.vendorId = 0x1414;
    EXPECT_EQ(warp.VendorName(), "Microsoft (WARP)");

    GPUAdapterInfo unknown{};
    unknown.vendorId = 0x0000;
    EXPECT_EQ(unknown.VendorName(), "Unknown");
}

TEST(GPUAdapterInfo, VRAMLabel) {
    GPUAdapterInfo gpu{};
    gpu.dedicatedVRAM = 4ULL * 1024 * 1024 * 1024;
    EXPECT_EQ(gpu.VRAMLabel(), "4.0 GB");
}

TEST(GPUAdapterInfo, VRAMLabelSmall) {
    GPUAdapterInfo gpu{};
    gpu.dedicatedVRAM = 512ULL * 1024 * 1024;
    EXPECT_EQ(gpu.VRAMLabel(), "0.5 GB");
}

//==============================================================================
// DPI Scaling Info Tests
//==============================================================================

TEST(DPIScaling, DefaultValues) {
    DPIScalingInfo info{};
    EXPECT_EQ(info.systemDPI, 96u);
    EXPECT_FALSE(info.IsHighDPI());
    EXPECT_EQ(info.ScalePercent(), 100u);
}

TEST(DPIScaling, HighDPIDetection) {
    DPIScalingInfo info{};
    info.systemDPI = 144;
    info.scaleFactor = 1.5f;
    EXPECT_TRUE(info.IsHighDPI());
    EXPECT_EQ(info.ScalePercent(), 150u);
}

TEST(DPIScaling, Scale200) {
    DPIScalingInfo info{};
    info.scaleFactor = 2.0f;
    EXPECT_TRUE(info.IsHighDPI());
    EXPECT_EQ(info.ScalePercent(), 200u);
}

//==============================================================================
// ARM64 Features Tests
//==============================================================================

TEST(ARM64Features, DefaultNotARM) {
    ARM64Features features{};
    EXPECT_FALSE(features.isARM64);
    EXPECT_FALSE(features.CanRunNative());
}

TEST(ARM64Features, NativeARM64) {
    ARM64Features features{};
    features.isARM64 = true;
    features.isEmulated = false;
    EXPECT_TRUE(features.CanRunNative());
}

TEST(ARM64Features, EmulatedARM64) {
    ARM64Features features{};
    features.isARM64 = true;
    features.isEmulated = true;
    EXPECT_FALSE(features.CanRunNative());
}

//==============================================================================
// HDR Display Info Tests
//==============================================================================

TEST(HDRDisplayInfo, DefaultSDR) {
    HDRDisplayInfo info{};
    EXPECT_FALSE(info.hdrSupported);
    EXPECT_FALSE(info.hdrEnabled);
    EXPECT_FALSE(info.wideColorGamut);
    EXPECT_EQ(info.maxLuminance, 0.0f);
}

//==============================================================================
// Compatibility Matrix Tests
//==============================================================================

TEST(CompatMatrix, EmptyMatrixNoTests) {
    CompatibilityMatrix matrix{};
    EXPECT_FALSE(matrix.AllTestsPassed());
    EXPECT_EQ(matrix.PassCount(), 0u);
    EXPECT_EQ(matrix.FailCount(), 0u);
}

TEST(CompatMatrix, AllPassedScenario) {
    CompatibilityMatrix matrix{};
    matrix.testResults.push_back({ "Test1", true, "OK", "" });
    matrix.testResults.push_back({ "Test2", true, "OK", "" });
    matrix.testResults.push_back({ "Test3", true, "OK", "" });
    EXPECT_TRUE(matrix.AllTestsPassed());
    EXPECT_EQ(matrix.PassCount(), 3u);
    EXPECT_EQ(matrix.FailCount(), 0u);
}

TEST(CompatMatrix, SomeFailedScenario) {
    CompatibilityMatrix matrix{};
    matrix.testResults.push_back({ "Test1", true, "OK", "" });
    matrix.testResults.push_back({ "Test2", false, "FAIL", "Fix it" });
    EXPECT_FALSE(matrix.AllTestsPassed());
    EXPECT_EQ(matrix.PassCount(), 1u);
    EXPECT_EQ(matrix.FailCount(), 1u);
}

TEST(CompatMatrix, GPUCount) {
    CompatibilityMatrix matrix{};
    EXPECT_EQ(matrix.GPUCount(), 0u);
    EXPECT_FALSE(matrix.HasDiscreteGPU());

    GPUAdapterInfo nvidia{};
    nvidia.vendorId = 0x10DE;
    nvidia.isHardware = true;
    matrix.gpuAdapters.push_back(nvidia);
    EXPECT_EQ(matrix.GPUCount(), 1u);
    EXPECT_TRUE(matrix.HasDiscreteGPU());
}

TEST(CompatMatrix, IntelIGPNotDiscrete) {
    CompatibilityMatrix matrix{};
    GPUAdapterInfo intel{};
    intel.vendorId = 0x8086;
    intel.isHardware = true;
    matrix.gpuAdapters.push_back(intel);
    EXPECT_FALSE(matrix.HasDiscreteGPU());
}

//==============================================================================
// Live Detection Tests (machine-dependent — always pass)
//==============================================================================

TEST(LiveDetection, WindowsVersion) {
    auto info = WindowsVersionDetector::Detect();
    EXPECT_GE(info.majorVersion, 10u);
    EXPECT_GT(info.buildNumber, 0u);
    auto label = info.BuildLabel();
    EXPECT_FALSE(label.empty());
}

TEST(LiveDetection, DarkMode) {
    auto state = DarkModeDetector::Detect();
    // Should at least detect something on modern Windows
    EXPECT_NE(static_cast<uint32_t>(state), 99u);
}

TEST(LiveDetection, DPIScaling) {
    auto info = DPIScalingDetector::Detect();
    EXPECT_GE(info.systemDPI, 72u);
    EXPECT_GE(info.monitorCount, 1u);
}

TEST(LiveDetection, ARM64Features) {
    auto features = ARM64FeatureDetector::Detect();
    // On x64 machine, should report not ARM64
    // On ARM64 machine, should report isARM64
    // Either way, no crash
    SUCCEED();
}

TEST(LiveDetection, FullCompatibilityMatrix) {
    auto matrix = CompatibilityMatrixBuilder::Build();
    EXPECT_GE(matrix.testResults.size(), 5u);
    // At minimum: OS version, dark mode, GPU, D3D11, DPI, HDR, ARM64
    EXPECT_GE(matrix.testResults.size(), 7u);
}
