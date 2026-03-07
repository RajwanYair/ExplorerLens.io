# ExplorerLens.py — Memory Cache
# Copyright (c) 2026 ExplorerLens Project
"""
In-memory LRU cache for thumbnails. Mirrors SubMillisecondCacheEngine
from ExplorerLens.io with Python's built-in OrderedDict for O(1) eviction.
"""

from __future__ import annotations

import sys
import threading
import time
from collections import OrderedDict
from dataclasses import dataclass
from typing import Optional

from PIL import Image


@dataclass
class CacheEntry:
    """A cached thumbnail with metadata."""
    image: Image.Image
    size: int
    timestamp: float
    access_count: int = 0

    @property
    def memory_bytes(self) -> int:
        """Estimate memory usage of the cached image."""
        if self.image is None:
            return 0
        w, h = self.image.size
        channels = len(self.image.getbands())
        return w * h * channels


class MemoryCache:
    """
    Thread-safe in-memory LRU cache for thumbnail images.

    Provides sub-millisecond lookups mirroring the C++ SubMillisecondCacheEngine.
    Eviction is by LRU order with an optional memory budget.
    """

    def __init__(self, max_items: int = 10000,
                 max_memory_mb: int = 256) -> None:
        self._max_items = max_items
        self._max_memory = max_memory_mb * 1024 * 1024
        self._cache: OrderedDict[str, CacheEntry] = OrderedDict()
        self._lock = threading.Lock()
        self._hits = 0
        self._misses = 0
        self._current_memory = 0

    def get(self, key: str, size: int) -> Optional[Image.Image]:
        """Retrieve a cached thumbnail. Returns None on miss."""
        cache_key = f"{key}:{size}"
        with self._lock:
            entry = self._cache.get(cache_key)
            if entry is None:
                self._misses += 1
                return None
            # Move to end (most recently used)
            self._cache.move_to_end(cache_key)
            entry.access_count += 1
            self._hits += 1
            return entry.image.copy()

    def put(self, key: str, size: int, image: Image.Image) -> None:
        """Store a thumbnail in the cache."""
        cache_key = f"{key}:{size}"
        entry = CacheEntry(
            image=image.copy(),
            size=size,
            timestamp=time.time(),
        )
        with self._lock:
            # Remove old entry if exists
            if cache_key in self._cache:
                old = self._cache.pop(cache_key)
                self._current_memory -= old.memory_bytes

            self._cache[cache_key] = entry
            self._current_memory += entry.memory_bytes

            # Evict if over limits
            self._evict()

    def invalidate(self, key: str) -> None:
        """Remove all sizes of a specific file from cache."""
        with self._lock:
            to_remove = [k for k in self._cache if k.startswith(f"{key}:")]
            for k in to_remove:
                entry = self._cache.pop(k)
                self._current_memory -= entry.memory_bytes

    def clear(self) -> None:
        """Clear entire cache."""
        with self._lock:
            self._cache.clear()
            self._current_memory = 0

    @property
    def stats(self) -> dict:
        """Return cache statistics."""
        with self._lock:
            total = self._hits + self._misses
            return {
                "items": len(self._cache),
                "memory_mb": self._current_memory / (1024 * 1024),
                "hits": self._hits,
                "misses": self._misses,
                "hit_rate": self._hits / total if total > 0 else 0.0,
                "max_items": self._max_items,
                "max_memory_mb": self._max_memory / (1024 * 1024),
            }

    def _evict(self) -> None:
        """Evict oldest entries until within budget. Caller holds lock."""
        while (len(self._cache) > self._max_items or
               self._current_memory > self._max_memory):
            if not self._cache:
                break
            _, entry = self._cache.popitem(last=False)
            self._current_memory -= entry.memory_bytes
