#pragma once
//==============================================================================
// TestInfrastructure.h — Umbrella header for Test Infrastructure
//
// Consolidates: TestFramework.h, TestSuiteExpansion.h,
//               CodeCoverageIntegration.h
//
// Single include for all test infrastructure needs:
//   - Integration test framework (suites, fixtures, corpus validation)
//   - Test suite expansion (decoder spec generation)
//   - Coverage + fuzzing integration
//
// NOTE: CodeCoverage.h excluded — CoverageTool/CoverageMetric/CoverageThresholds
//       are already defined in TestFramework.h and CodeCoverageIntegration.h.
//==============================================================================

#include "TestFramework.h"
#include "TestSuiteExpansion.h"
#include "CodeCoverageIntegration.h"

namespace ExplorerLens {
namespace Engine {

/// Quick check that all test infra modules are available.
inline bool TestInfrastructureAvailable() {
    // Each module has a key type — verify they're all reachable.
    static_assert(sizeof(IntegrationTestFramework) > 0,
        "IntegrationTestFramework missing");
    static_assert(sizeof(TestSuiteExpansion) > 0,
        "TestSuiteExpansion missing");
    static_assert(sizeof(CodeCoverageIntegration) > 0,
        "CodeCoverageIntegration missing");
    return true;
}

/// Count of consolidated test infrastructure modules.
inline constexpr uint32_t TestInfrastructureModuleCount() { return 3; }

}
} // namespace ExplorerLens::Engine
