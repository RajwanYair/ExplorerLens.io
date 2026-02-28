#pragma once
// GPUTextureAtlasManager.h — GPU Texture Atlas Manager
// Batches multiple thumbnails into GPU texture atlases for efficient
// rendering and reduced draw calls — supports dynamic sub-allocation.
#include <algorithm>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Atlas packing algorithm
enum class PackingAlgorithm : uint8_t {
 ShelfNextFit = 0, // Simple shelf packing (fast)
 ShelfBestFit, // Shelf with best-height selection
 MaxRects, // Max-rectangles bin packing (best utilisation)
 Guillotine, // Binary split packing
 COUNT
};

/// Atlas format
enum class AtlasFormat : uint8_t {
 BGRA8_UNorm = 0, // Standard 32-bit
 BC7_UNorm, // Compressed RGBA
 R10G10B10A2, // HDR-ready
 R16G16B16A16_Float, // Full HDR
 COUNT
};

struct AtlasSlot {
 uint16_t x = 0;
 uint16_t y = 0;
 uint16_t width = 0;
 uint16_t height = 0;
 uint32_t atlasId = 0;
 bool occupied = false;
};

struct AtlasStats {
 uint32_t atlasCount = 0;
 uint32_t totalSlots = 0;
 uint32_t occupiedSlots = 0;
 float utilization = 0.0f;
 uint64_t vramUsageBytes = 0;
 uint32_t atlasWidth = 0;
 uint32_t atlasHeight = 0;
};

class GPUTextureAtlasManager {
public:
 static constexpr size_t PackingCount() {
 return static_cast<size_t>(PackingAlgorithm::COUNT);
 }
 static constexpr size_t FormatCount() {
 return static_cast<size_t>(AtlasFormat::COUNT);
 }

 static const wchar_t *PackingName(PackingAlgorithm a) {
 switch (a) {
 case PackingAlgorithm::ShelfNextFit:
 return L"Shelf Next-Fit";
 case PackingAlgorithm::ShelfBestFit:
 return L"Shelf Best-Fit";
 case PackingAlgorithm::MaxRects:
 return L"Max Rectangles";
 case PackingAlgorithm::Guillotine:
 return L"Guillotine";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *FormatName(AtlasFormat f) {
 switch (f) {
 case AtlasFormat::BGRA8_UNorm:
 return L"BGRA8 UNorm";
 case AtlasFormat::BC7_UNorm:
 return L"BC7 UNorm";
 case AtlasFormat::R10G10B10A2:
 return L"R10G10B10A2";
 case AtlasFormat::R16G16B16A16_Float:
 return L"RGBA16 Float";
 default:
 return L"Unknown";
 }
 }

 /// Calculate atlas VRAM usage
 static uint64_t CalcVRAM(uint32_t w, uint32_t h, AtlasFormat fmt) {
 uint32_t bpp = 4;
 switch (fmt) {
 case AtlasFormat::BGRA8_UNorm:
 bpp = 4;
 break;
 case AtlasFormat::BC7_UNorm:
 return ((w + 3) / 4) * ((h + 3) / 4) * 16ULL;
 case AtlasFormat::R10G10B10A2:
 bpp = 4;
 break;
 case AtlasFormat::R16G16B16A16_Float:
 bpp = 8;
 break;
 default:
 break;
 }
 return (uint64_t)w * h * bpp;
 }

 /// Max thumbnails that fit in a single atlas
 static uint32_t MaxSlotsPerAtlas(uint32_t atlasSize, uint32_t thumbnailSize) {
 if (thumbnailSize == 0)
 return 0;
 uint32_t cols = atlasSize / thumbnailSize;
 uint32_t rows = atlasSize / thumbnailSize;
 return cols * rows;
 }
};

} // namespace Engine
} // namespace ExplorerLens
