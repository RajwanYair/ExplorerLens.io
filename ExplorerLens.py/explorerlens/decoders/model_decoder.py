# ExplorerLens.py — 3D Model Decoder
# Copyright (c) 2026 ExplorerLens Project
"""
Renders preview thumbnails for 3D model files (OBJ, STL, PLY, GLTF, GLB,
FBX, 3DS). Uses trimesh + Pillow for offline rendering.
"""

from __future__ import annotations

import logging
from pathlib import Path
from typing import Optional

from PIL import Image, ImageDraw, ImageFont

from .base import BaseDecoder

logger = logging.getLogger("explorerlens.decoders.model")

_MODEL_EXTS = [".obj", ".stl", ".ply", ".gltf", ".glb", ".fbx", ".3ds"]


class ModelDecoder(BaseDecoder):
    """Renders 3D model preview thumbnails."""

    @property
    def name(self) -> str:
        return "ModelDecoder"

    def supported_extensions(self) -> list[str]:
        return list(_MODEL_EXTS)

    def decode(self, path: Path, size: int) -> Optional[Image.Image]:
        # Try trimesh-based rendering first
        img = self._decode_trimesh(path, size)
        if img is not None:
            return img
        # Fallback: styled placeholder
        return self._generate_placeholder(path, size)

    @staticmethod
    def _decode_trimesh(path: Path, size: int) -> Optional[Image.Image]:
        """Render 3D model using trimesh's offscreen renderer."""
        try:
            import trimesh
            import numpy as np

            mesh = trimesh.load(str(path), force="mesh")
            if mesh.is_empty:
                return None

            # Use trimesh's built-in scene rendering
            scene = mesh.scene() if hasattr(mesh, "scene") else trimesh.Scene(mesh)
            png_data = scene.save_image(resolution=(size, size))
            if png_data:
                import io
                return Image.open(io.BytesIO(png_data))
            return None
        except ImportError:
            logger.debug("trimesh not installed — using placeholder for %s", path)
            return None
        except Exception as exc:
            logger.debug("Trimesh render failed for %s: %s", path, exc)
            return None

    @staticmethod
    def _generate_placeholder(path: Path, size: int) -> Image.Image:
        """Generate a 3D-model-style placeholder thumbnail."""
        canvas = Image.new("RGB", (size, size), (35, 35, 50))
        draw = ImageDraw.Draw(canvas)

        # Draw a wireframe cube icon
        cx, cy = size // 2, size // 2
        s = size // 4

        # Front face
        front = [(cx - s, cy - s // 2), (cx + s, cy - s // 2),
                 (cx + s, cy + s), (cx - s, cy + s)]
        draw.polygon(front, outline=(100, 180, 255), width=max(1, size // 128))

        # Top face
        offset = s // 2
        top = [(cx - s, cy - s // 2), (cx - s + offset, cy - s),
               (cx + s + offset, cy - s), (cx + s, cy - s // 2)]
        draw.polygon(top, outline=(80, 150, 220), width=max(1, size // 128))

        # Right face
        right = [(cx + s, cy - s // 2), (cx + s + offset, cy - s),
                 (cx + s + offset, cy + s - offset), (cx + s, cy + s)]
        draw.polygon(right, outline=(60, 120, 200), width=max(1, size // 128))

        # Extension label
        ext_text = path.suffix.upper().lstrip(".")
        try:
            font = ImageFont.truetype("segoeui.ttf", size // 10)
        except Exception:
            font = ImageFont.load_default()
        bbox = draw.textbbox((0, 0), ext_text, font=font)
        tw = bbox[2] - bbox[0]
        draw.text(((size - tw) // 2, size - size // 5), ext_text,
                  fill=(160, 180, 220), font=font)

        return canvas
