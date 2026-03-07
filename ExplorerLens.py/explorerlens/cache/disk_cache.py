# ExplorerLens.py — Disk Cache
# Copyright (c) 2026 ExplorerLens Project
"""
SQLite-backed persistent thumbnail cache. Stores thumbnails as PNG blobs
with file path, size, mtime metadata for cache validation.
"""

from __future__ import annotations

import hashlib
import io
import logging
import sqlite3
import time
from pathlib import Path
from typing import Optional

from PIL import Image

from platformdirs import user_cache_dir

logger = logging.getLogger("explorerlens.cache.disk")

_APP_NAME = "ExplorerLens.py"
_DEFAULT_DB = Path(user_cache_dir(_APP_NAME)) / "thumbnails.db"


class DiskCache:
    """
    Persistent thumbnail cache using SQLite.

    Schema stores file hash + size as key, PNG blob as value,
    with mtime-based invalidation.
    """

    def __init__(self, db_path: Path | None = None,
                 max_size_mb: int = 1024) -> None:
        self._db_path = db_path or _DEFAULT_DB
        self._max_size = max_size_mb * 1024 * 1024
        self._db_path.parent.mkdir(parents=True, exist_ok=True)
        self._conn: sqlite3.Connection | None = None
        self._init_db()

    def _init_db(self) -> None:
        """Create the cache table if needed."""
        self._conn = sqlite3.connect(str(self._db_path), check_same_thread=False)
        self._conn.execute("PRAGMA journal_mode=WAL")
        self._conn.execute("PRAGMA synchronous=NORMAL")
        self._conn.execute("""
            CREATE TABLE IF NOT EXISTS thumbnails (
                key TEXT PRIMARY KEY,
                size INTEGER,
                mtime REAL,
                created REAL,
                accessed REAL,
                access_count INTEGER DEFAULT 0,
                blob_size INTEGER,
                data BLOB
            )
        """)
        self._conn.execute("""
            CREATE INDEX IF NOT EXISTS idx_accessed
            ON thumbnails(accessed)
        """)
        self._conn.commit()

    def get(self, key: str, size: int) -> Optional[Image.Image]:
        """Retrieve a cached thumbnail. Returns None on miss."""
        cache_key = self._make_key(key, size)
        try:
            row = self._conn.execute(
                "SELECT data, mtime FROM thumbnails WHERE key = ?",
                (cache_key,)
            ).fetchone()
            if row is None:
                return None

            # Validate mtime
            path = Path(key)
            if path.exists():
                current_mtime = path.stat().st_mtime
                if abs(current_mtime - row[1]) > 0.001:
                    # File changed — invalidate
                    self._conn.execute(
                        "DELETE FROM thumbnails WHERE key = ?", (cache_key,)
                    )
                    self._conn.commit()
                    return None

            # Update access time
            now = time.time()
            self._conn.execute(
                "UPDATE thumbnails SET accessed = ?, access_count = access_count + 1 WHERE key = ?",
                (now, cache_key)
            )
            self._conn.commit()

            return Image.open(io.BytesIO(row[0]))
        except Exception as exc:
            logger.debug("Disk cache read error: %s", exc)
            return None

    def put(self, key: str, size: int, image: Image.Image) -> None:
        """Store a thumbnail in the disk cache."""
        cache_key = self._make_key(key, size)
        try:
            buf = io.BytesIO()
            image.save(buf, format="PNG", optimize=True)
            data = buf.getvalue()

            mtime = 0.0
            path = Path(key)
            if path.exists():
                mtime = path.stat().st_mtime

            now = time.time()
            self._conn.execute(
                """INSERT OR REPLACE INTO thumbnails
                   (key, size, mtime, created, accessed, access_count, blob_size, data)
                   VALUES (?, ?, ?, ?, ?, 0, ?, ?)""",
                (cache_key, size, mtime, now, now, len(data), data)
            )
            self._conn.commit()

            # Check if we need to evict
            self._maybe_evict()
        except Exception as exc:
            logger.debug("Disk cache write error: %s", exc)

    def invalidate(self, key: str) -> None:
        """Remove all cached sizes for a file."""
        try:
            self._conn.execute(
                "DELETE FROM thumbnails WHERE key LIKE ?",
                (f"{self._hash(key)}:%",)
            )
            self._conn.commit()
        except Exception as exc:
            logger.debug("Disk cache invalidation error: %s", exc)

    def clear(self) -> None:
        """Clear entire disk cache."""
        try:
            self._conn.execute("DELETE FROM thumbnails")
            self._conn.execute("VACUUM")
            self._conn.commit()
        except Exception as exc:
            logger.debug("Disk cache clear error: %s", exc)

    @property
    def stats(self) -> dict:
        """Return cache statistics."""
        try:
            row = self._conn.execute(
                "SELECT COUNT(*), COALESCE(SUM(blob_size), 0) FROM thumbnails"
            ).fetchone()
            return {
                "items": row[0],
                "size_mb": row[1] / (1024 * 1024),
                "db_path": str(self._db_path),
                "max_size_mb": self._max_size / (1024 * 1024),
            }
        except Exception:
            return {"items": 0, "size_mb": 0, "db_path": str(self._db_path)}

    def close(self) -> None:
        """Close the database connection."""
        if self._conn:
            self._conn.close()
            self._conn = None

    # ── Private ──────────────────────────────────────────────────────

    @staticmethod
    def _hash(key: str) -> str:
        return hashlib.sha256(key.encode("utf-8")).hexdigest()[:16]

    def _make_key(self, key: str, size: int) -> str:
        return f"{self._hash(key)}:{size}"

    def _maybe_evict(self) -> None:
        """Evict LRU entries if total size exceeds budget."""
        try:
            row = self._conn.execute(
                "SELECT COALESCE(SUM(blob_size), 0) FROM thumbnails"
            ).fetchone()
            total = row[0]
            if total <= self._max_size:
                return

            # Delete oldest 10% by access time
            count = self._conn.execute(
                "SELECT COUNT(*) FROM thumbnails"
            ).fetchone()[0]
            to_delete = max(1, count // 10)
            self._conn.execute(
                "DELETE FROM thumbnails WHERE key IN "
                "(SELECT key FROM thumbnails ORDER BY accessed ASC LIMIT ?)",
                (to_delete,)
            )
            self._conn.commit()
        except Exception as exc:
            logger.debug("Disk cache eviction error: %s", exc)
