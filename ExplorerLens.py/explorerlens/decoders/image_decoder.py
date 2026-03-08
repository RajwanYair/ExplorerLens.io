# ExplorerLens.py — Image Decoder
# Copyright (c) 2026 ExplorerLens Project
"""
Decodes raster images, HDR, RAW, SVG, PSD, and modern formats (AVIF, HEIC,
JXL, WebP, QOI). Uses Pillow as primary backend with optional pillow-heif,
jxlpy, rawpy, and cairosvg for extended format support.
"""

from __future__ import annotations

import logging
from pathlib import Path
from typing import Optional

from PIL import Image

from .base import BaseDecoder

logger = logging.getLogger("explorerlens.decoders.image")

# Standard Pillow-supported extensions
_PILLOW_EXTS = [
    ".bmp",
    ".gif",
    ".jpg",
    ".jpeg",
    ".png",
    ".tiff",
    ".tif",
    ".ico",
    ".tga",
    ".ppm",
    ".pgm",
    ".pbm",
    ".pcx",
    ".dds",
    ".webp",
    ".xpm",
    ".sgi",
]

# Extensions requiring optional libraries
_OPTIONAL_EXTS = {
    ".heic": "pillow_heif",
    ".heif": "pillow_heif",
    ".avif": "pillow_avif",
    ".jxl": "jxlpy",
    ".svg": "cairosvg",
    ".psd": "psd_tools",
    ".hdr": "imageio",
    ".exr": "imageio",
    ".qoi": "qoi",
    ".xcf": "PIL",
    ".ora": "zipfile",
    ".jxr": "imageio",
    ".eps": "PIL",
    ".jp2": "PIL",
    ".ktx": "PIL",
    ".vtf": "PIL",
    ".farbfeld": "PIL",
}

_RAW_EXTS = [
    ".cr2",
    ".cr3",
    ".nef",
    ".arw",
    ".dng",
    ".orf",
    ".rw2",
    ".pef",
    ".srw",
    ".raf",
    ".raw",
    ".3fr",
    ".dcr",
    ".kdc",
    ".mrw",
    ".nrw",
    ".rwl",
    ".sr2",
    ".srf",
    ".x3f",
]


class ImageDecoder(BaseDecoder):
    """Handles all raster, vector, RAW, and modern image formats."""

    @property
    def name(self) -> str:
        return "ImageDecoder"

    def supported_extensions(self) -> list[str]:
        exts = list(_PILLOW_EXTS)
        exts.extend(_OPTIONAL_EXTS.keys())
        exts.extend(_RAW_EXTS)
        return exts

    def decode(self, path: Path, size: int) -> Optional[Image.Image]:
        ext = path.suffix.lower()

        if ext in _RAW_EXTS:
            return self._decode_raw(path, size)
        if ext == ".svg":
            return self._decode_svg(path, size)
        if ext == ".psd":
            return self._decode_psd(path, size)
        if ext in (".heic", ".heif"):
            return self._decode_heif(path, size)
        if ext == ".avif":
            return self._decode_avif(path, size)
        if ext == ".jxl":
            return self._decode_jxl(path, size)
        if ext in (".hdr", ".exr"):
            return self._decode_hdr(path, size)
        if ext == ".qoi":
            return self._decode_qoi(path, size)

        # Fallback: standard Pillow open
        return self._decode_pillow(path, size)

    # ── Backend-specific decoders ────────────────────────────────────

    @staticmethod
    def _decode_pillow(path: Path, _size: int) -> Optional[Image.Image]:
        try:
            img: Image.Image = Image.open(path)
            img.load()
            if img.mode not in ("RGB", "RGBA"):
                img = img.convert("RGBA" if img.mode == "PA" else "RGB")
            return img
        except (OSError, ValueError, SyntaxError) as exc:
            logger.debug("Pillow decode failed for %s: %s", path, exc)
            return None

    @staticmethod
    def _decode_raw(path: Path, size: int) -> Optional[Image.Image]:
        try:
            import rawpy

            with rawpy.imread(str(path)) as raw:
                rgb = raw.postprocess(
                    use_camera_wb=True,
                    half_size=(size <= 512),
                    output_bps=8,
                )
            return Image.fromarray(rgb)
        except ImportError:
            logger.warning("rawpy not installed — skipping RAW: %s", path)
            return None
        except (OSError, ValueError, RuntimeError) as exc:
            logger.debug("RAW decode failed for %s: %s", path, exc)
            return None

    @staticmethod
    def _decode_svg(path: Path, size: int) -> Optional[Image.Image]:
        try:
            import io

            import cairosvg

            png_data = cairosvg.svg2png(
                url=str(path),
                output_width=size,
                output_height=size,
            )
            return Image.open(io.BytesIO(png_data))
        except ImportError:
            logger.warning("cairosvg not installed — skipping SVG: %s", path)
            return None
        except (OSError, ValueError, RuntimeError) as exc:
            logger.debug("SVG decode failed for %s: %s", path, exc)
            return None

    @staticmethod
    def _decode_psd(path: Path, _size: int) -> Optional[Image.Image]:
        try:
            from psd_tools import PSDImage

            psd = PSDImage.open(path)
            return psd.composite()
        except ImportError:
            logger.warning("psd-tools not installed — skipping PSD: %s", path)
            return None
        except (OSError, ValueError, RuntimeError) as exc:
            logger.debug("PSD decode failed for %s: %s", path, exc)
            return None

    @staticmethod
    def _decode_heif(path: Path, _size: int) -> Optional[Image.Image]:
        try:
            from pillow_heif import register_heif_opener

            register_heif_opener()
            return Image.open(path)
        except ImportError:
            logger.warning("pillow-heif not installed — skipping HEIF: %s", path)
            return None
        except (OSError, ValueError, RuntimeError) as exc:
            logger.debug("HEIF decode failed for %s: %s", path, exc)
            return None

    @staticmethod
    def _decode_avif(path: Path, _size: int) -> Optional[Image.Image]:
        try:
            from pillow_heif import register_avif_opener

            register_avif_opener()
            return Image.open(path)
        except ImportError:
            try:
                import pillow_avif  # noqa: F401  # type: ignore[import-untyped]

                return Image.open(path)
            except ImportError:
                logger.warning("No AVIF decoder — skipping: %s", path)
                return None
        except (OSError, ValueError) as exc:
            logger.debug("AVIF decode failed for %s: %s", path, exc)
            return None

    @staticmethod
    def _decode_jxl(path: Path, _size: int) -> Optional[Image.Image]:
        try:
            import jxlpy  # noqa: F401  # type: ignore[import-untyped]

            return Image.open(path)
        except ImportError:
            logger.warning("jxlpy not installed — skipping JXL: %s", path)
            return None
        except (OSError, ValueError) as exc:
            logger.debug("JXL decode failed for %s: %s", path, exc)
            return None

    @staticmethod
    def _decode_hdr(path: Path, _size: int) -> Optional[Image.Image]:
        try:
            import imageio.v3 as iio
            import numpy as np

            data = iio.imread(str(path))
            # Tone-map HDR to 8-bit LDR
            if data.dtype in (np.float32, np.float64):
                data = np.clip(data * 255, 0, 255).astype(np.uint8)
            return Image.fromarray(data)
        except ImportError:
            logger.warning("imageio not installed — skipping HDR: %s", path)
            return None
        except (OSError, ValueError, RuntimeError) as exc:
            logger.debug("HDR decode failed for %s: %s", path, exc)
            return None

    @staticmethod
    def _decode_qoi(path: Path, _size: int) -> Optional[Image.Image]:
        try:
            import qoi

            img = qoi.read(str(path))
            return Image.fromarray(img)
        except ImportError:
            logger.warning("qoi not installed — skipping QOI: %s", path)
            return None
        except (OSError, ValueError, RuntimeError) as exc:
            logger.debug("QOI decode failed for %s: %s", path, exc)
            return None
