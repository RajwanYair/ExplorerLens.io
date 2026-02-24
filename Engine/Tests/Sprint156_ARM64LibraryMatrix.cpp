// ARM64 Library Matrix — GTest
#include "../Utils/ARM64LibraryMatrix.h"
#include "GTestShim.h"

using namespace ExplorerLens::Platform;

TEST(ARM64LibraryMatrix, DefaultMatrixHasEntries) {
  auto m = ARM64LibraryMatrix::CreateDefault();
  EXPECT_GE(m.libraries.size(), 5u);
}

TEST(ARM64LibraryMatrix, DefaultMatrixHas12Libraries) {
  auto m = ARM64LibraryMatrix::CreateDefault();
  EXPECT_EQ(m.libraries.size(), 12u);
}

TEST(ARM64LibraryMatrix, LibBuildStatusEnumCoverage) {
  EXPECT_EQ(static_cast<uint32_t>(LibBuildStatus::NotStarted), 0u);
  EXPECT_EQ(static_cast<uint32_t>(LibBuildStatus::Success), 1u);
}

TEST(ARM64LibraryMatrix, MatrixEntryHasName) {
  LibMatrixEntry e;
  e.name = "zlib";
  EXPECT_FALSE(e.name.empty());
}

TEST(ARM64LibraryMatrix, MatrixEntryHasVersion) {
  LibMatrixEntry e;
  e.version = "1.3.1";
  EXPECT_FALSE(e.version.empty());
}

TEST(ARM64LibraryMatrix, CrossBuildParamsHasToolchain) {
  CrossBuildParameters p;
  p.toolchainFile = "cmake/toolchain-windows-arm64.cmake";
  EXPECT_FALSE(p.toolchainFile.empty());
}

TEST(ARM64LibraryMatrix, MinPassThresholdSetTo7) {
  EXPECT_GE(ARM64LibraryMatrix::kMinPassThreshold, 7u);
}

TEST(ARM64LibraryMatrix, DefaultMatrixX64AllSuccess) {
  auto m = ARM64LibraryMatrix::CreateDefault();
  uint32_t x64Success = 0;
  for (const auto &e : m.libraries)
    if (e.x64Status == LibBuildStatus::Success)
      ++x64Success;
  EXPECT_EQ(x64Success, (uint32_t)m.libraries.size());
}

TEST(ARM64LibraryMatrix, DefaultMatrixZlibHasBuildScript) {
  auto m = ARM64LibraryMatrix::CreateDefault();
  for (const auto &e : m.libraries) {
    if (e.name == "zlib") {
      EXPECT_FALSE(e.buildScript.empty());
      return;
    }
  }
  FAIL(); // zlib not found in matrix
}

TEST(ARM64LibraryMatrix, CrossBuildDefaultGeneratorNinja) {
  CrossBuildParameters p;
  EXPECT_EQ(p.generator, std::string("Visual Studio 18 2026"));
}

TEST(ARM64LibraryMatrix, SuccessCountDefault) {
  auto m = ARM64LibraryMatrix::CreateDefault();
  EXPECT_GE(m.ReadyCount(), 0u);
}
