#pragma once
//=============================================================================
// PluginMarketplaceUnified.h — Unified Plugin Marketplace Umbrella
//
// Consolidates marketplace versions into a single include point:
//   - PluginMarketplaceV2.h  (V2: production API, wstring, SHA-256, has .cpp impl)
//   - PluginMarketplaceV3.h  (V3: static utility — categories, trust levels)
//
// Note: PluginMarketplace.h (V1) is excluded from this umbrella because its
// PackageType and CertificateInfo definitions conflict with MSIXPackageManager.h
// and CertificateTrustValidator.h respectively. V1 is legacy — use V2 for new code.
//
// Usage: #include "PluginMarketplaceUnified.h" instead of individual versions.
//=============================================================================

// V2: Production marketplace with SHA-256 verification (canonical impl)
#include "PluginMarketplaceV2.h"

// V3: Static utility for category/trust/sandbox enum lookups
#include "PluginMarketplaceV3.h"

namespace ExplorerLens {
namespace Engine {

/// Helper to select the appropriate marketplace version at compile time
enum class MarketplaceVersion : uint32_t {
    V2 = 2,   ///< Production implementation with SHA-256 (recommended)
    V3 = 3    ///< Static utility enums only
};

/// Returns the recommended marketplace version for new code
inline constexpr MarketplaceVersion RecommendedMarketplaceVersion() {
    return MarketplaceVersion::V2;
}

/// Returns the number of active (non-legacy) marketplace versions
inline constexpr uint32_t ActiveMarketplaceVersionCount() {
    return 2;
}

} // namespace Engine
} // namespace ExplorerLens
