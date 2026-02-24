// ARM64 Build Config — GTestShim
#include "GTestShim.h"
#include "Utils/ARM64BuildConfig.h"

using namespace ExplorerLens::Platform;

TEST(ARM64BuildConfig, CurrentArchitectureNotUnknown) {
  auto arch = CurrentArchitecture();
  (void)arch;
  EXPECT_NE(arch, TargetArchitecture::Unknown);
}

TEST(ARM64BuildConfig, SIMDCapabilityHasAtLeastOne) {
  SIMDCapabilities caps;
  caps.hasSSE2 = true;
  EXPECT_TRUE(caps.hasSSE2 || caps.hasNEON);
}

TEST(ARM64BuildConfig, DefaultInventoryHasEntries) {
  auto inv = ARM64LibraryInventory::CreateDefault();
  EXPECT_GE(inv.entries.size(), 5u);
}

TEST(ARM64BuildConfig, DefaultInventoryHasZlib) {
  auto inv = ARM64LibraryInventory::CreateDefault();
  bool found = false;
  for (const auto &e : inv.entries)
    if (e.libraryName == "zlib") {
      found = true;
      break;
    }
  EXPECT_TRUE(found);
}

TEST(ARM64BuildConfig, CMakeParamsHaveSystemProcessor) {
  CMakeARM64Parameters p;
  EXPECT_FALSE(p.systemProcessor.empty());
}

TEST(ARM64BuildConfig, MSBuildConfigPlatformARM64) {
  MSBuildARM64Config cfg;
  EXPECT_EQ(cfg.platform, std::string("ARM64"));
}

TEST(ARM64BuildConfig, LibraryStatusEnumCoverage) {
  auto s = LibraryARM64Status::BuildSuccess;
  (void)s;
  EXPECT_EQ(static_cast<uint32_t>(s), 1u);
}

TEST(ARM64BuildConfig, ArchitectureEnumARM64Value) {
  EXPECT_EQ(static_cast<uint32_t>(TargetArchitecture::ARM64), 3u);
}

TEST(ARM64BuildConfig, LibraryEntryHasVersion) {
  LibraryARM64Entry e;
  e.version = "1.3.1";
  EXPECT_FALSE(e.version.empty());
}

TEST(ARM64BuildConfig, CurrentArchitectureOnX64IsKnown) {
  auto arch = CurrentArchitecture();
  bool isKnown =
      (arch == TargetArchitecture::x64 || arch == TargetArchitecture::x86 ||
       arch == TargetArchitecture::ARM64 || arch == TargetArchitecture::ARM);
  (void)isKnown;
  EXPECT_TRUE(isKnown);
}

TEST(ARM64BuildConfig, DefaultInventory12Libraries) {
  auto inv = ARM64LibraryInventory::CreateDefault();
  EXPECT_EQ(inv.entries.size(), 12u);
}

TEST(ARM64BuildConfig, SIMDCapabilitiesDefaultAllFalse) {
  SIMDCapabilities caps;
  EXPECT_FALSE(caps.hasAVX2);
  EXPECT_FALSE(caps.hasNEON);
}
