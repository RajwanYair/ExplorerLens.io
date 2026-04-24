// ============================================================================
// TiledTiffReaderStub.h -- S255 / ROADMAP v6.0 H10
//
// Phase 2 tile-based TIFF pyramid reader contract (H10 from libvips).  For
// large TIFFs / BigTIFF / OME-TIFF we want to read only the tiles that cover
// the requested thumbnail region, never the full image.
//
// Header-only.  Actual tile I/O lands in Phase 2 alongside the libtiff
// decoder (S258).
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class TiledTiffReadMode : uint8_t
{
    SINGLE_TILE         = 0,   // read one tile
    TILE_RECT           = 1,   // read a rectangle of tiles covering a region
    PYRAMID_BEST_FIT    = 2,   // auto-select pyramid level for requested size
    FULL_STRIP          = 3,   // non-tiled TIFF fallback (strips)
};

enum class TiledTiffStatus : uint8_t
{
    OK                   = 0,
    NOT_TILED            = 1,
    NO_PYRAMID           = 2,
    BIGTIFF_UNSUPPORTED  = 3,
    IO_ERROR             = 4,
    OUT_OF_BOUNDS        = 5,
    OUT_OF_BUDGET        = 6,
};

struct TiledTiffTile
{
    uint32_t tileX     = 0;   // tile column index
    uint32_t tileY     = 0;   // tile row index
    uint32_t widthPx   = 0;
    uint32_t heightPx  = 0;
    uint64_t offset    = 0;   // byte offset inside TIFF stream
    uint64_t sizeBytes = 0;
};

struct TiledTiffLayout
{
    uint32_t fullWidth      = 0;
    uint32_t fullHeight     = 0;
    uint32_t tileWidth      = 0;
    uint32_t tileHeight     = 0;
    uint32_t pyramidLevels  = 0;
    bool     isBigTiff      = false;
    bool     isTiled        = false;
};

struct TiledTiffProbeResult
{
    TiledTiffStatus  status      = TiledTiffStatus::OK;
    TiledTiffLayout  layout      = {};
    uint32_t         bestLevel   = 0;    // chosen pyramid level
    uint32_t         levelWidth  = 0;
    uint32_t         levelHeight = 0;
};

// Budgets -- prevent a malicious TIFF from consuming unbounded memory.
inline constexpr uint32_t kTiledTiffMaxTileSide      = 8192;
inline constexpr uint32_t kTiledTiffMaxPyramidLevels = 16;
inline constexpr uint64_t kTiledTiffMaxBudgetBytes   = 64ull * 1024 * 1024;  // 64 MiB tile rect

static_assert(std::is_trivially_copyable_v<TiledTiffTile>,
              "TiledTiffTile must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TiledTiffLayout>,
              "TiledTiffLayout must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TiledTiffProbeResult>,
              "TiledTiffProbeResult must be trivially copyable");

} // namespace ExplorerLens::Engine
