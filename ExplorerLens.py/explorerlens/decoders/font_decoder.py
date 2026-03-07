# ExplorerLens.py — Font Decoder
# Copyright (c) 2026 ExplorerLens Project
"""
Renders font preview thumbnails using fonttools + Pillow.
Supports TTF, OTF, WOFF, WOFF2.
"""

from __future__ import annotations

import logging
from pathlib import Path
from typing import Optional

from PIL import Image, ImageDraw, ImageFont

from .base import BaseDecoder

logger = logging.getLogger("explorerlens.decoders.font")

_FONT_EXTS = [".ttf", ".otf", ".woff", ".woff2"]

_PREVIEW_TEXT = "AaBbCc\n0123456"


class FontDecoder(BaseDecoder):
    """Renders font preview thumbnails."""

    @property
    def name(self) -> str:
        return "FontDecoder"

    def supported_extensions(self) -> list[str]:
        return list(_FONT_EXTS)

    def decode(self, path: Path, size: int) -> Optional[Image.Image]:
        try:
            return self._render_preview(path, size)
        except Exception as exc:
            logger.debug("Font decode failed for %s: %s", path, exc)
            return None

    @staticmethod
    def _render_preview(path: Path, size: int) -> Image.Image:
        """Render the font at multiple sizes with sample text."""
        canvas = Image.new("RGB", (size, size), (255, 255, 255))
        draw = ImageDraw.Draw(canvas)

        # Try to get font family name
        font_name = path.stem
        try:
            from fontTools.ttLib import TTFont
            tt = TTFont(str(path))
            name_table = tt["name"]
            for record in name_table.names:
                if record.nameID == 1:  # Font Family
                    font_name = str(record)
                    break
            tt.close()
        except Exception:
            pass

        # Header with font name
        try:
            header_font = ImageFont.truetype("segoeui.ttf", size // 12)
        except Exception:
            header_font = ImageFont.load_default()

        draw.text(
            (size // 20, size // 20),
            font_name,
            fill=(100, 100, 100),
            font=header_font,
        )

        # Render sample text at the target font at various sizes
        y = size // 5
        for pt_size in [size // 5, size // 8, size // 12]:
            try:
                font = ImageFont.truetype(str(path), pt_size)
            except Exception:
                font = ImageFont.load_default()

            draw.text(
                (size // 20, y),
                _PREVIEW_TEXT.split("\n")[0],
                fill=(30, 30, 30),
                font=font,
            )
            y += pt_size + size // 20

            if y + pt_size > size:
                break

        # Draw a subtle border
        draw.rectangle(
            [0, 0, size - 1, size - 1],
            outline=(200, 200, 200),
            width=1,
        )

        return canvas
