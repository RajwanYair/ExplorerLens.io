// OfflineAvailabilityChecker.h — Cloud File Offline Availability Check
// Copyright (c) 2026 ExplorerLens Project
//
// Determines whether a file is available offline via the CfApi placeholder state
// or OS network path detection, enabling ExplorerLens to decide the decode path
// (local buffer / partial hydration / cloud-side thumbnail fetch).
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

// ---- Availability Decision --------------------------------------------------

enum class AvailabilityDecision : uint8_t {
    // Decode directly from local bytes — fastest path.
    FullyLocal      = 0,
    // Hydrate a small header range via CfApi, then decode.
    PartialHydrate  = 1,
    // Fetch pre-generated thumbnail from cloud API (skip local decode).
    CloudThumb      = 2,
    // No path available — skip this file silently.
    Skip            = 3,
};

struct AvailabilityReport {
    AvailabilityDecision decision       = AvailabilityDecision::Skip;
    bool   isCloudPlaceholder           = false;
    bool   isNetworkPath                = false;   // UNC / mapped drive
    bool   isOfflineNetworkPath         = false;   // Disconnected network path
    uint64_t localBytesAvailable        = 0;       // CfApi: bytes already hydrated
    uint64_t totalFileSize              = 0;
    float  localFraction                = 0.0f;   // localBytes / totalSize
};

// ---- OfflineAvailabilityChecker ---------------------------------------------

class OfflineAvailabilityChecker {
public:
    OfflineAvailabilityChecker()  = default;
    ~OfflineAvailabilityChecker() = default;

    // Determine the best decode strategy for the given file path.
    AvailabilityReport Check(const std::string& path) const;

    // Quick probe: returns FullyLocal or non-local without full CfApi query.
    static bool IsFullyLocal(const std::string& path);

    // Compute optimal header range size needed to probe a file at a given path.
    // Returns bytes needed for magic detection (e.g. 512 for most formats, 4096 for TIFF).
    static uint64_t HeaderBytesNeeded(const std::string& path);

    // Policy: on metered connections, skip all non-local files.
    bool SkipNonLocalOnMeteredNetwork = true;
    // Policy: max hydration header size before falling back to CloudThumb.
    uint64_t MaxPartialHydrateBytes   = 131072; // 128 KB
};

} // namespace Engine
} // namespace ExplorerLens
