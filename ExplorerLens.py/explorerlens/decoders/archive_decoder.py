# ExplorerLens.py — Archive Decoder
# Copyright (c) 2026 ExplorerLens Project
"""
Decodes archive formats (ZIP, RAR, 7Z, TAR, etc.) by extracting embedded
images and compositing a collage thumbnail. Also handles comic book
archives (CBR/CBZ/CB7). Mirrors LENSArchive from ExplorerLens.io.
"""

from __future__ import annotations

import io
import logging
import zipfile
from pathlib import Path
from typing import Optional

from PIL import Image, ImageDraw

from .base import BaseDecoder

logger = logging.getLogger("explorerlens.decoders.archive")

_ARCHIVE_EXTS = [
    ".zip", ".rar", ".7z", ".tar", ".tar.gz", ".tar.bz2",
    ".tar.xz", ".tar.zst", ".gz", ".bz2", ".xz", ".zst",
    ".lz4", ".cpio", ".iso", ".cab", ".ar", ".deb",
]

_COMIC_EXTS = [".cbz", ".cbr", ".cb7", ".cbt"]

_EBOOK_EXTS = [".epub", ".mobi", ".fb2", ".azw", ".azw3", ".phz"]

_IMAGE_SUFFIXES = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp", ".tiff"}


class ArchiveDecoder(BaseDecoder):
    """Handles archives, comic books, and ebooks."""

    @property
    def name(self) -> str:
        return "ArchiveDecoder"

    def supported_extensions(self) -> list[str]:
        return _ARCHIVE_EXTS + _COMIC_EXTS + _EBOOK_EXTS

    def decode(self, path: Path, size: int) -> Optional[Image.Image]:
        ext = path.suffix.lower()

        if ext == ".cbz" or ext == ".zip":
            return self._decode_zip(path, size, is_comic=ext == ".cbz")
        if ext in (".cbr", ".rar"):
            return self._decode_rar(path, size, is_comic=ext == ".cbr")
        if ext in (".cb7", ".7z"):
            return self._decode_7z(path, size, is_comic=ext == ".cb7")
        if ext == ".epub":
            return self._decode_epub(path, size)
        if ext.startswith(".tar") or ext in (".gz", ".bz2", ".xz", ".zst"):
            return self._decode_tar(path, size)

        # Fallback: try libarchive via python-libarchive-c
        return self._decode_libarchive(path, size)

    # ── ZIP / CBZ ────────────────────────────────────────────────────

    def _decode_zip(self, path: Path, size: int,
                    is_comic: bool = False) -> Optional[Image.Image]:
        try:
            with zipfile.ZipFile(path, "r") as zf:
                images = self._extract_images_zip(zf, max_count=4)
                if not images:
                    return None
                if is_comic:
                    return images[0]  # Cover page
                return self._make_collage(images, size)
        except Exception as exc:
            logger.debug("ZIP decode failed for %s: %s", path, exc)
            return None

    @staticmethod
    def _extract_images_zip(zf: zipfile.ZipFile,
                            max_count: int = 4) -> list[Image.Image]:
        """Extract up to max_count images from a ZIP file."""
        found: list[Image.Image] = []
        entries = sorted(zf.namelist())
        for name in entries:
            if len(found) >= max_count:
                break
            suffix = Path(name).suffix.lower()
            if suffix in _IMAGE_SUFFIXES:
                try:
                    data = zf.read(name)
                    img = Image.open(io.BytesIO(data))
                    img.load()
                    found.append(img)
                except Exception:
                    continue
        return found

    # ── RAR / CBR ────────────────────────────────────────────────────

    def _decode_rar(self, path: Path, size: int,
                    is_comic: bool = False) -> Optional[Image.Image]:
        try:
            import rarfile
            with rarfile.RarFile(str(path), "r") as rf:
                images = self._extract_images_rar(rf, max_count=4)
                if not images:
                    return None
                if is_comic:
                    return images[0]
                return self._make_collage(images, size)
        except ImportError:
            logger.warning("rarfile not installed — skipping RAR: %s", path)
            return None
        except Exception as exc:
            logger.debug("RAR decode failed for %s: %s", path, exc)
            return None

    @staticmethod
    def _extract_images_rar(rf, max_count: int = 4) -> list[Image.Image]:
        found: list[Image.Image] = []
        entries = sorted(rf.namelist())
        for name in entries:
            if len(found) >= max_count:
                break
            suffix = Path(name).suffix.lower()
            if suffix in _IMAGE_SUFFIXES:
                try:
                    data = rf.read(name)
                    img = Image.open(io.BytesIO(data))
                    img.load()
                    found.append(img)
                except Exception:
                    continue
        return found

    # ── 7Z / CB7 ────────────────────────────────────────────────────

    def _decode_7z(self, path: Path, size: int,
                   is_comic: bool = False) -> Optional[Image.Image]:
        try:
            import py7zr
            with py7zr.SevenZipFile(str(path), mode="r") as sz:
                images = self._extract_images_7z(sz, max_count=4)
                if not images:
                    return None
                if is_comic:
                    return images[0]
                return self._make_collage(images, size)
        except ImportError:
            logger.warning("py7zr not installed — skipping 7Z: %s", path)
            return None
        except Exception as exc:
            logger.debug("7Z decode failed for %s: %s", path, exc)
            return None

    @staticmethod
    def _extract_images_7z(sz, max_count: int = 4) -> list[Image.Image]:
        found: list[Image.Image] = []
        names = sorted(sz.getnames())
        targets = []
        for name in names:
            if len(targets) >= max_count:
                break
            if Path(name).suffix.lower() in _IMAGE_SUFFIXES:
                targets.append(name)
        if not targets:
            return found
        extracted = sz.read(targets)
        for name in targets:
            bio = extracted.get(name)
            if bio is not None:
                try:
                    img = Image.open(bio)
                    img.load()
                    found.append(img)
                except Exception:
                    continue
        return found

    # ── TAR archives ─────────────────────────────────────────────────

    def _decode_tar(self, path: Path, size: int) -> Optional[Image.Image]:
        try:
            import tarfile
            with tarfile.open(str(path), "r:*") as tf:
                images: list[Image.Image] = []
                members = sorted(tf.getmembers(), key=lambda m: m.name)
                for member in members:
                    if len(images) >= 4:
                        break
                    if (member.isfile() and
                            Path(member.name).suffix.lower() in _IMAGE_SUFFIXES):
                        try:
                            f = tf.extractfile(member)
                            if f:
                                img = Image.open(f)
                                img.load()
                                images.append(img)
                        except Exception:
                            continue
                if not images:
                    return None
                return self._make_collage(images, size)
        except Exception as exc:
            logger.debug("TAR decode failed for %s: %s", path, exc)
            return None

    # ── EPUB ─────────────────────────────────────────────────────────

    def _decode_epub(self, path: Path, size: int) -> Optional[Image.Image]:
        """Extract cover image from EPUB (which is a ZIP internally)."""
        try:
            with zipfile.ZipFile(path, "r") as zf:
                # Look for cover image patterns
                for name in zf.namelist():
                    lower = name.lower()
                    if ("cover" in lower and
                            Path(lower).suffix in _IMAGE_SUFFIXES):
                        data = zf.read(name)
                        return Image.open(io.BytesIO(data))
                # Fallback: first image
                images = self._extract_images_zip(zf, max_count=1)
                return images[0] if images else None
        except Exception as exc:
            logger.debug("EPUB decode failed for %s: %s", path, exc)
            return None

    # ── libarchive fallback ──────────────────────────────────────────

    def _decode_libarchive(self, path: Path,
                           size: int) -> Optional[Image.Image]:
        try:
            import libarchive
            images: list[Image.Image] = []
            with libarchive.file_reader(str(path)) as archive:
                for entry in archive:
                    if len(images) >= 4:
                        break
                    if Path(str(entry)).suffix.lower() in _IMAGE_SUFFIXES:
                        data = b"".join(entry.get_blocks())
                        try:
                            img = Image.open(io.BytesIO(data))
                            img.load()
                            images.append(img)
                        except Exception:
                            continue
            if not images:
                return None
            return self._make_collage(images, size)
        except ImportError:
            return None
        except Exception as exc:
            logger.debug("libarchive decode failed for %s: %s", path, exc)
            return None

    # ── Collage builder ──────────────────────────────────────────────

    @staticmethod
    def _make_collage(images: list[Image.Image],
                      size: int) -> Image.Image:
        """
        Compose a 2x2 collage from up to 4 images, mirroring the
        ExplorerLens.io archive thumbnail style.
        """
        canvas = Image.new("RGB", (size, size), (40, 40, 40))
        n = len(images)
        if n == 0:
            return canvas

        if n == 1:
            images[0].thumbnail((size, size), Image.Resampling.LANCZOS)
            canvas.paste(images[0], (0, 0))
            return canvas

        half = size // 2
        gap = 2
        cell = half - gap

        positions = [(0, 0), (half + gap, 0),
                     (0, half + gap), (half + gap, half + gap)]

        for i, img in enumerate(images[:4]):
            img.thumbnail((cell, cell), Image.Resampling.LANCZOS)
            canvas.paste(img, positions[i])

        return canvas
