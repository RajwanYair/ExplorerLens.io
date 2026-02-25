// BitmapPool.h — Pre-allocated Bitmap Pool for Thumbnail Generation
// Copyright (c) 2026 ExplorerLens Project
//
// Reduces allocation overhead by maintaining a pool of pre-allocated
// HBITMAP objects at common thumbnail sizes (128x128, 256x256, 512x512).
// Thread-safe via SRWLOCK for high-throughput shell extension use.
//
// Usage:
//   auto& pool = BitmapPool::Instance();
//   HBITMAP bmp = pool.Acquire(256, 256);
//   // ... render into bitmap ...
//   pool.Release(bmp);  // Returns to pool instead of destroying
//
// Performance target: <0.1ms acquire vs ~0.5ms CreateDIBSection

#pragma once

#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Configuration for a bitmap pool tier
struct BitmapPoolConfig {
  uint32_t width = 256;
  uint32_t height = 256;
  uint32_t poolSize = 50;     ///< Max bitmaps to keep in pool per size
  uint32_t bitsPerPixel = 32; ///< Always 32bpp BGRA for thumbnail use
};

/// Statistics for monitoring pool effectiveness
struct BitmapPoolStats {
  uint64_t acquireCount = 0;   ///< Total acquire calls
  uint64_t releaseCount = 0;   ///< Total release calls
  uint64_t poolHits = 0;       ///< Acquires served from pool (no allocation)
  uint64_t poolMisses = 0;     ///< Acquires that required new allocation
  uint64_t currentPooled = 0;  ///< Currently pooled (available) bitmaps
  uint64_t peakPooled = 0;     ///< Peak pooled bitmap count
  uint64_t totalAllocated = 0; ///< Total bytes allocated across all pools

  /// Hit rate as percentage (0-100)
  double HitRate() const {
    return acquireCount > 0 ? (100.0 * poolHits / acquireCount) : 0.0;
  }
};

/// Thread-safe pre-allocated bitmap pool for common thumbnail sizes.
/// Singleton pattern — access via BitmapPool::Instance().
class BitmapPool {
public:
  /// Get the global singleton instance
  static BitmapPool &Instance();

  /// Initialize pool with default tiers (128, 256, 512)
  void Initialize();

  /// Initialize pool with custom tier configurations
  void Initialize(const std::vector<BitmapPoolConfig> &configs);

  /// Shutdown and free all pooled bitmaps
  void Shutdown();

  /// Acquire a bitmap of the requested size.
  /// Returns a pooled bitmap if available, otherwise creates a new one.
  /// @param width  Requested bitmap width
  /// @param height Requested bitmap height
  /// @return HBITMAP (32bpp top-down DIB), or nullptr on failure.
  ///         Caller MUST call Release() when done, NOT DeleteObject().
  HBITMAP Acquire(uint32_t width, uint32_t height);

  /// Return a bitmap to the pool for reuse.
  /// If the pool for this size is full, the bitmap is destroyed.
  /// @param hBitmap Bitmap previously obtained from Acquire()
  void Release(HBITMAP hBitmap);

  /// Get current pool statistics
  BitmapPoolStats GetStats() const;

  /// Reset statistics counters
  void ResetStats();

  // Non-copyable
  BitmapPool(const BitmapPool &) = delete;
  BitmapPool &operator=(const BitmapPool &) = delete;

private:
  BitmapPool();
  ~BitmapPool();

  /// Key for pool lookup: packs width and height into a single uint64
  static uint64_t MakeKey(uint32_t w, uint32_t h) {
    return (static_cast<uint64_t>(w) << 32) | h;
  }

  /// Create a new 32bpp top-down DIB section
  static HBITMAP CreatePoolBitmap(uint32_t width, uint32_t height);

  /// Pool tier: holds available bitmaps for a specific size
  struct PoolTier {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t maxSize = 50;
    std::vector<HBITMAP> available;
    mutable SRWLOCK lock = SRWLOCK_INIT;
  };

  std::unordered_map<uint64_t, PoolTier> m_tiers;
  mutable std::mutex m_tiersMutex; // Protects m_tiers map structure
  BitmapPoolStats m_stats;
  mutable SRWLOCK m_statsLock = SRWLOCK_INIT;
  bool m_initialized = false;

  /// Track which bitmaps came from the pool (for proper Release handling)
  std::unordered_map<HBITMAP, uint64_t> m_bitmapOrigin;
  mutable SRWLOCK m_originLock = SRWLOCK_INIT;
};

} // namespace Engine
} // namespace ExplorerLens
