# ExplorerLens.py — Tiered Cache
# Copyright (c) 2026 ExplorerLens Project
"""
Two-tier cache combining MemoryCache (fast L1) and DiskCache (persistent L2).
Mirrors the multi-tier caching architecture of ExplorerLens.io's
SubMillisecondCacheEngine + DiskThumbnailCache.
"""

from __future__ import annotations

import logging
from pathlib import Path
from typing import Optional

from PIL import Image

from .memory_cache import MemoryCache
from .disk_cache import DiskCache

logger = logging.getLogger("explorerlens.cache.tiered")


class TieredCache:
    """
    L1 (memory) + L2 (disk) cache with automatic promotion.

    On get:  L1 hit → return.  L1 miss → check L2 → promote to L1.
    On put:  Write to both L1 and L2.
    """

    def __init__(self, memory: MemoryCache | None = None,
                 disk: DiskCache | None = None) -> None:
        self._l1 = memory or MemoryCache()
        self._l2 = disk or DiskCache()

    def get(self, key: str, size: int) -> Optional[Image.Image]:
        """Look up key in L1, then L2. Promotes L2 hits to L1."""
        img = self._l1.get(key, size)
        if img is not None:
            return img

        img = self._l2.get(key, size)
        if img is not None:
            # Promote to L1
            self._l1.put(key, size, img)
            return img

        return None

    def put(self, key: str, size: int, image: Image.Image) -> None:
        """Store in both L1 and L2."""
        self._l1.put(key, size, image)
        self._l2.put(key, size, image)

    def invalidate(self, key: str) -> None:
        """Remove from both tiers."""
        self._l1.invalidate(key)
        self._l2.invalidate(key)

    def clear(self) -> None:
        """Clear both tiers."""
        self._l1.clear()
        self._l2.clear()

    @property
    def stats(self) -> dict:
        """Combined stats from both tiers."""
        l1 = self._l1.stats
        l2 = self._l2.stats
        total_hits = l1.get("hits", 0) + l2.get("hits", 0)
        total_misses = l2.get("misses", 0)  # L2 miss = true miss
        return {
            "l1_items": l1.get("items", 0),
            "l1_memory_mb": l1.get("memory_mb", 0),
            "l1_hits": l1.get("hits", 0),
            "l2_items": l2.get("items", 0),
            "l2_size_mb": l2.get("size_mb", 0),
            "l2_hits": l2.get("hits", 0),
            "total_hits": total_hits,
            "total_misses": total_misses,
            "hit_rate": (total_hits / max(1, total_hits + total_misses)) * 100,
        }

    @property
    def memory_cache(self) -> MemoryCache:
        return self._l1

    @property
    def disk_cache(self) -> DiskCache:
        return self._l2

    def close(self) -> None:
        """Shut down disk cache."""
        self._l2.close()
