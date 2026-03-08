# ExplorerLens.py — Core Thumbnail Engine
# Copyright (c) 2026 ExplorerLens Project
"""
Central decode pipeline that routes files to specialized decoders, manages
concurrency, and returns PIL Image thumbnails. Mirrors ExplorerLensEngine.lib.
"""

from __future__ import annotations

import logging
import time
from concurrent.futures import Future, ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from enum import IntEnum, auto
from pathlib import Path
from typing import Optional, Sequence

from PIL import Image

from .config import Config
from .decoders.base import BaseDecoder

logger = logging.getLogger("explorerlens.engine")


class DecodeStatus(IntEnum):
    """Result status for a thumbnail request."""

    Success = 0
    UnsupportedFormat = auto()
    FileNotFound = auto()
    DecodeFailed = auto()
    Timeout = auto()
    Cancelled = auto()


@dataclass
class ThumbnailRequest:
    """A single thumbnail generation request."""

    path: Path
    size: int = 256
    format: str = "PNG"
    quality: int = 85


@dataclass
class ThumbnailResult:
    """Result of a thumbnail generation request."""

    request: ThumbnailRequest
    status: DecodeStatus = DecodeStatus.Success
    image: Optional[Image.Image] = None
    elapsed_ms: float = 0.0
    decoder_name: str = ""
    error: str = ""


class ThumbnailEngine:
    """
    Core engine — routes files to decoders and manages the decode pipeline.

    Usage:
        engine = ThumbnailEngine(config)
        result = engine.generate(ThumbnailRequest(Path("photo.cr2"), 256))
    """

    def __init__(self, config: Config | None = None) -> None:
        self._config = config or Config()
        self._decoders: dict[str, BaseDecoder] = {}
        self._pool: ThreadPoolExecutor | None = None
        self._stats = EngineStats()
        self._cache: object | None = None
        self._init_decoders()
        if self._config.cache.enabled:
            self._init_cache()

    def _init_decoders(self) -> None:
        """Lazily import and register all decoder implementations."""
        from .decoders import get_all_decoders

        for decoder in get_all_decoders():
            for ext in decoder.supported_extensions():
                self._decoders[ext.lower()] = decoder
        logger.info(
            "Initialized %d decoders covering %d extensions",
            len(set(id(d) for d in self._decoders.values())),
            len(self._decoders),
        )

    def _init_cache(self) -> None:
        """Auto-create a TieredCache from config settings."""
        from .cache import DiskCache, MemoryCache, TieredCache

        memory = MemoryCache(
            max_items=self._config.cache.max_items,
            max_memory_mb=self._config.cache.memory_limit_mb,
        )
        disk = DiskCache(max_size_mb=self._config.cache.disk_limit_mb)
        self._cache = TieredCache(memory=memory, disk=disk)

    def set_cache(self, cache: object) -> None:
        """Attach a cache backend (MemoryCache or DiskCache)."""
        self._cache = cache

    def generate(self, request: ThumbnailRequest) -> ThumbnailResult:
        """Generate a single thumbnail synchronously."""
        start = time.perf_counter()
        result = ThumbnailResult(request=request)

        if not request.path.exists():
            result.status = DecodeStatus.FileNotFound
            result.error = f"File not found: {request.path}"
            self._stats.failed += 1
            return result

        ext = request.path.suffix.lower()
        if not self._is_enabled(ext):
            result.status = DecodeStatus.UnsupportedFormat
            result.error = f"Extension not enabled: {ext}"
            self._stats.skipped += 1
            return result

        # Check cache first
        if self._cache is not None:
            cached = self._cache_get(request)
            if cached is not None:
                result.image = cached
                result.status = DecodeStatus.Success
                result.elapsed_ms = (time.perf_counter() - start) * 1000
                result.decoder_name = "cache"
                self._stats.cache_hits += 1
                return result

        decoder = self._decoders.get(ext)
        if decoder is None:
            result.status = DecodeStatus.UnsupportedFormat
            result.error = f"No decoder for: {ext}"
            self._stats.failed += 1
            return result

        try:
            img = decoder.decode(request.path, request.size)
            if img is not None:
                result.image = self._fit_thumbnail(img, request.size)
                result.status = DecodeStatus.Success
                result.decoder_name = decoder.name
                self._stats.succeeded += 1
                # Store in cache
                if self._cache is not None:
                    self._cache_put(request, result.image)
            else:
                result.status = DecodeStatus.DecodeFailed
                result.error = "Decoder returned None"
                self._stats.failed += 1
        except (OSError, ValueError, RuntimeError) as exc:
            result.status = DecodeStatus.DecodeFailed
            result.error = str(exc)
            result.decoder_name = decoder.name
            self._stats.failed += 1
            logger.warning("Decode failed for %s: %s", request.path, exc)

        result.elapsed_ms = (time.perf_counter() - start) * 1000
        self._stats.total_ms += result.elapsed_ms
        return result

    def generate_batch(
        self, requests: Sequence[ThumbnailRequest]
    ) -> list[ThumbnailResult]:
        """Generate thumbnails in parallel using thread pool."""
        if self._pool is None:
            workers = self._config.performance.max_workers
            self._pool = ThreadPoolExecutor(max_workers=workers)

        futures: dict[Future, int] = {}
        for idx, req in enumerate(requests):
            future = self._pool.submit(self.generate, req)
            futures[future] = idx

        results: list[ThumbnailResult | None] = [None] * len(requests)
        for future in as_completed(futures):
            idx = futures[future]
            try:
                results[idx] = future.result(
                    timeout=self._config.performance.thumbnail_timeout_ms / 1000
                )
            except (OSError, TimeoutError) as exc:
                results[idx] = ThumbnailResult(
                    request=requests[idx],
                    status=DecodeStatus.Timeout,
                    error=str(exc),
                )
                self._stats.failed += 1

        return [r for r in results if r is not None]

    def get_stats(self) -> EngineStats:
        return self._stats

    def get_supported_extensions(self) -> set[str]:
        """Return all extensions this engine can handle."""
        return set(self._decoders.keys())

    def shutdown(self) -> None:
        """Clean up thread pool and cache."""
        if self._pool is not None:
            self._pool.shutdown(wait=False)
            self._pool = None
        if self._cache is not None and hasattr(self._cache, "close"):
            self._cache.close()

    # ── Private helpers ──────────────────────────────────────────────

    def _is_enabled(self, ext: str) -> bool:
        """Check if the extension is enabled via config."""
        enabled = self._config.get_enabled_extensions()
        return ext in enabled

    @staticmethod
    def _fit_thumbnail(img: Image.Image, size: int) -> Image.Image:
        """Resize image to fit within size x size, preserving aspect ratio."""
        img.thumbnail((size, size), Image.Resampling.LANCZOS)
        return img

    def _cache_get(self, req: ThumbnailRequest) -> Image.Image | None:
        """Try to retrieve from cache."""
        try:
            return self._cache.get(str(req.path), req.size)  # type: ignore[union-attr]
        except (OSError, KeyError, TypeError):
            return None

    def _cache_put(self, req: ThumbnailRequest, img: Image.Image) -> None:
        """Store in cache."""
        try:
            self._cache.put(str(req.path), req.size, img)  # type: ignore[union-attr]
        except (OSError, TypeError):
            pass


@dataclass
class EngineStats:
    """Tracks engine performance metrics."""

    succeeded: int = 0
    failed: int = 0
    skipped: int = 0
    cache_hits: int = 0
    total_ms: float = 0.0

    @property
    def total(self) -> int:
        return self.succeeded + self.failed + self.skipped

    @property
    def avg_ms(self) -> float:
        n = self.succeeded + self.failed
        return self.total_ms / n if n > 0 else 0.0

    @property
    def images_per_sec(self) -> float:
        if self.total_ms > 0:
            return self.succeeded / (self.total_ms / 1000)
        return 0.0
