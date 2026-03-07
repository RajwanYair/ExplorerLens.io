# ExplorerLens.py — Video Decoder
# Copyright (c) 2026 ExplorerLens Project
"""
Extracts thumbnail frames from video files using ffmpeg.
Supports all 34+ video formats from ExplorerLens.io including:
AVI, WMV, ASF, MPG, MPEG, M1V, M2V, TS, M2TS, MTS, M2T,
MP4, M4V, MP4V, MOV, 3G2, 3GP, 3GP2, 3GPP, MKV, MK3D,
WEBM, FLV, F4V, OGM, OGV, RM, RMVB, DV, MXF, IVF, EVO, 264, VIDEO.
"""

from __future__ import annotations

import io
import logging
import shutil
import subprocess
from pathlib import Path
from typing import Optional

from PIL import Image

from .base import BaseDecoder

logger = logging.getLogger("explorerlens.decoders.video")

_VIDEO_EXTS = [
    ".avi",
    ".wmv",
    ".asf",
    ".mpg",
    ".mpeg",
    ".m1v",
    ".m2v",
    ".ts",
    ".m2ts",
    ".mts",
    ".m2t",
    ".mp4",
    ".m4v",
    ".mp4v",
    ".mov",
    ".3g2",
    ".3gp",
    ".3gp2",
    ".3gpp",
    ".mkv",
    ".mk3d",
    ".webm",
    ".flv",
    ".f4v",
    ".ogm",
    ".ogv",
    ".rm",
    ".rmvb",
    ".dv",
    ".mxf",
    ".ivf",
    ".evo",
    ".264",
    ".video",
]


class VideoDecoder(BaseDecoder):
    """Extracts thumbnail frames from video files via ffmpeg."""

    _ffmpeg_path: str | None = None
    _ffprobe_path: str | None = None

    @property
    def name(self) -> str:
        return "VideoDecoder"

    def supported_extensions(self) -> list[str]:
        return list(_VIDEO_EXTS)

    def decode(self, path: Path, size: int) -> Optional[Image.Image]:
        ffmpeg = self._find_ffmpeg()
        if ffmpeg is None:
            logger.warning("ffmpeg not found in PATH — skipping video: %s", path)
            return self._generate_placeholder(path, size)

        # Try to grab a frame at 10% of duration for a representative thumb
        img = self._extract_frame(ffmpeg, path, size, seek_pct=0.1)
        if img is None:
            # Fallback: grab at 5 seconds in
            img = self._extract_frame_at(ffmpeg, path, size, seek_sec=5.0)
        if img is None:
            # Last resort: grab the very first frame
            img = self._extract_frame(ffmpeg, path, size, seek_pct=0.0)
        return img

    # ── Internal ─────────────────────────────────────────────────────

    @classmethod
    def _find_ffmpeg(cls) -> str | None:
        """Locate ffmpeg executable."""
        if cls._ffmpeg_path is not None:
            return cls._ffmpeg_path
        found = shutil.which("ffmpeg")
        if found:
            cls._ffmpeg_path = found
        return found

    @classmethod
    def _find_ffprobe(cls) -> str | None:
        """Locate ffprobe executable."""
        if cls._ffprobe_path is not None:
            return cls._ffprobe_path
        found = shutil.which("ffprobe")
        if found:
            cls._ffprobe_path = found
        return found

    @classmethod
    def _get_duration(cls, path: Path) -> float | None:
        """Get video duration in seconds using ffprobe."""
        ffprobe = cls._find_ffprobe()
        if not ffprobe:
            return None
        try:
            result = subprocess.run(
                [
                    ffprobe,
                    "-v",
                    "error",
                    "-show_entries",
                    "format=duration",
                    "-of",
                    "default=noprint_wrappers=1:nokey=1",
                    str(path),
                ],
                capture_output=True,
                text=True,
                timeout=10,
            )
            return float(result.stdout.strip())
        except Exception:
            return None

    @classmethod
    def _extract_frame(
        cls, ffmpeg: str, path: Path, size: int, seek_pct: float
    ) -> Optional[Image.Image]:
        """Extract a single frame as PNG via ffmpeg pipe."""
        cmd = [ffmpeg, "-v", "error"]

        if seek_pct > 0:
            duration = cls._get_duration(path)
            if duration and duration > 0:
                seek_sec = duration * seek_pct
                cmd += ["-ss", f"{seek_sec:.2f}"]

        cmd += [
            "-i",
            str(path),
            "-frames:v",
            "1",
            "-vf",
            f"scale={size}:{size}:force_original_aspect_ratio=decrease",
            "-f",
            "image2pipe",
            "-vcodec",
            "png",
            "-",
        ]

        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                timeout=15,
            )
            if result.returncode != 0 or not result.stdout:
                return None
            return Image.open(io.BytesIO(result.stdout))
        except subprocess.TimeoutExpired:
            logger.debug("ffmpeg timeout for %s", path)
            return None
        except Exception as exc:
            logger.debug("Video frame extraction failed for %s: %s", path, exc)
            return None

    @classmethod
    def _extract_frame_at(
        cls, ffmpeg: str, path: Path, size: int, seek_sec: float
    ) -> Optional[Image.Image]:
        """Extract a frame at an exact timestamp."""
        cmd = [
            ffmpeg,
            "-v",
            "error",
            "-ss",
            f"{seek_sec:.2f}",
            "-i",
            str(path),
            "-frames:v",
            "1",
            "-vf",
            f"scale={size}:{size}:force_original_aspect_ratio=decrease",
            "-f",
            "image2pipe",
            "-vcodec",
            "png",
            "-",
        ]
        try:
            result = subprocess.run(cmd, capture_output=True, timeout=15)
            if result.returncode != 0 or not result.stdout:
                return None
            return Image.open(io.BytesIO(result.stdout))
        except Exception:
            return None

    @staticmethod
    def _generate_placeholder(path: Path, size: int) -> Image.Image:
        """Generate a video-style placeholder when ffmpeg is unavailable."""
        from PIL import ImageDraw, ImageFont

        canvas = Image.new("RGB", (size, size), (20, 20, 30))
        draw = ImageDraw.Draw(canvas)

        # Play button triangle
        cx, cy = size // 2, size // 2
        r = size // 5
        triangle = [
            (cx - r // 2, cy - r),
            (cx - r // 2, cy + r),
            (cx + r, cy),
        ]
        draw.polygon(triangle, fill=(200, 200, 220))

        # Extension label
        ext_text = path.suffix.upper().lstrip(".")
        try:
            font = ImageFont.truetype("segoeui.ttf", size // 10)
        except Exception:
            font = ImageFont.load_default()
        bbox = draw.textbbox((0, 0), ext_text, font=font)
        tw = bbox[2] - bbox[0]
        draw.text(
            ((size - tw) // 2, size - size // 5),
            ext_text,
            fill=(160, 170, 200),
            font=font,
        )

        return canvas
