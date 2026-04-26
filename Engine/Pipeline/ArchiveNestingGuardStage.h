// ============================================================================
// ArchiveNestingGuardStage.h -- S274 / ROADMAP v6.0 S10 archive safety
//
// Phase 3 safety stage contract.  Limits recursion depth, total uncompressed
// size, and child count when a thumbnail request targets a nested archive
// (e.g. first image inside `photos.zip -> inner.7z -> DCIM/x.jpg`).
// Prevents zip-bombs and runaway nested decompression.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class ArchiveNestingGuardStatus : uint8_t
{
    OK                        = 0,
    MAX_DEPTH_EXCEEDED        = 1,
    MAX_TOTAL_BYTES_EXCEEDED  = 2,
    MAX_CHILD_COUNT_EXCEEDED  = 3,
    COMPRESSION_RATIO_ABUSE   = 4,   // uncompressed / compressed > ratio cap
    BUDGET_EXCEEDED           = 5,
    CYCLE_DETECTED            = 6,   // symlink or self-referencing tar
};

enum class ArchiveNestingGuardAction : uint8_t
{
    CONTINUE           = 0,
    USE_FORMAT_ICON    = 1,   // emit generic archive icon
    USE_PARENT_SNAPSHOT = 2,  // reuse outer archive's cover
    ABORT              = 3,
};

struct ArchiveNestingGuardPolicy
{
    uint32_t maxDepth                = 4;
    uint64_t maxTotalUncompressedMb  = 1024;   // 1 GiB cap
    uint32_t maxChildCount           = 100000;
    uint32_t maxCompressionRatio     = 200;    // defuses zip-bombs
    uint32_t budgetMs                = 50;
    bool     abortOnCycle            = true;
    bool     logDeniedRequests       = true;
};

struct ArchiveNestingGuardReport
{
    ArchiveNestingGuardStatus status        = ArchiveNestingGuardStatus::OK;
    ArchiveNestingGuardAction action        = ArchiveNestingGuardAction::CONTINUE;
    uint32_t                  currentDepth  = 0;
    uint64_t                  totalUncompressedBytes = 0;
    uint32_t                  totalChildCount = 0;
    uint32_t                  measuredRatio = 0;
};

inline constexpr uint32_t kArchiveNestingGuardMaxDepth          = 8;
inline constexpr uint64_t kArchiveNestingGuardHardMaxBytes      = 4ull * 1024ull * 1024ull * 1024ull; // 4 GiB
inline constexpr uint32_t kArchiveNestingGuardHardRatio         = 1000;
inline constexpr uint32_t kArchiveNestingGuardDefaultBudgetMs   = 50;

static_assert(std::is_trivially_copyable_v<ArchiveNestingGuardPolicy>,
              "ArchiveNestingGuardPolicy must be trivially copyable");
static_assert(std::is_trivially_copyable_v<ArchiveNestingGuardReport>,
              "ArchiveNestingGuardReport must be trivially copyable");

} // namespace ExplorerLens::Engine
