# ExplorerLens.py — Audio Decoder
# Copyright (c) 2026 ExplorerLens Project
"""
Extracts embedded album art from audio files using mutagen.
Falls back to a waveform visualization or stylized music-note placeholder.
Supports all audio formats from ExplorerLens.io: MP3, WAV, M4A, APE, FLAC,
OGG, MKA, MPC, OPUS, TAK, WV, WMA, AAC.
"""

from __future__ import annotations

import io
import logging
import struct
from pathlib import Path
from typing import Optional

from PIL import Image, ImageDraw, ImageFont

from .base import BaseDecoder

logger = logging.getLogger("explorerlens.decoders.audio")

_AUDIO_EXTS = [
    ".mp3",
    ".wav",
    ".m4a",
    ".ape",
    ".flac",
    ".ogg",
    ".mka",
    ".mpc",
    ".opus",
    ".tak",
    ".wv",
    ".wma",
    ".aac",
]


class AudioDecoder(BaseDecoder):
    """Extracts album art or generates audio-style thumbnails."""

    @property
    def name(self) -> str:
        return "AudioDecoder"

    def supported_extensions(self) -> list[str]:
        return list(_AUDIO_EXTS)

    def decode(self, path: Path, size: int) -> Optional[Image.Image]:
        # Try embedded album art first
        art = self._extract_album_art(path)
        if art is not None:
            return art

        # Try waveform visualization for WAV files
        if path.suffix.lower() == ".wav":
            waveform = self._generate_waveform(path, size)
            if waveform is not None:
                return waveform

        # Fallback: generate a stylized placeholder
        return self._generate_placeholder(path, size)

    # ── Album art extraction ─────────────────────────────────────────

    @staticmethod
    def _extract_album_art(path: Path) -> Optional[Image.Image]:
        """Try to extract embedded album art using mutagen."""
        try:
            import mutagen
            from mutagen.flac import FLAC
            from mutagen.mp3 import MP3
            from mutagen.mp4 import MP4
            from mutagen.oggopus import OggOpus
            from mutagen.oggvorbis import OggVorbis

            ext = path.suffix.lower()

            if ext == ".mp3":
                audio = MP3(str(path))
                if audio.tags:
                    for tag in audio.tags.values():
                        if hasattr(tag, "data") and hasattr(tag, "mime"):
                            return Image.open(io.BytesIO(tag.data))

            elif ext == ".flac":
                audio = FLAC(str(path))
                if audio.pictures:
                    return Image.open(io.BytesIO(audio.pictures[0].data))

            elif ext in (".m4a", ".aac", ".mp4"):
                audio = MP4(str(path))
                if audio.tags:
                    covr = audio.tags.get("covr")
                    if covr:
                        return Image.open(io.BytesIO(bytes(covr[0])))

            elif ext == ".ogg":
                audio = OggVorbis(str(path))
                pics = audio.get("metadata_block_picture")
                if pics:
                    import base64

                    from mutagen.flac import Picture

                    pic = Picture(base64.b64decode(pics[0]))
                    return Image.open(io.BytesIO(pic.data))

            elif ext == ".opus":
                audio = OggOpus(str(path))
                pics = audio.get("metadata_block_picture")
                if pics:
                    import base64

                    from mutagen.flac import Picture

                    pic = Picture(base64.b64decode(pics[0]))
                    return Image.open(io.BytesIO(pic.data))

            elif ext == ".wma":
                audio = mutagen.File(str(path))
                if audio is not None and audio.tags:
                    # WMA/ASF stores pictures in WM/Picture tag
                    for key in audio.tags:
                        if "picture" in key.lower():
                            tag = audio.tags[key]
                            if hasattr(tag, "value"):
                                for v in tag if isinstance(tag, list) else [tag]:
                                    if hasattr(v, "value"):
                                        try:
                                            return Image.open(io.BytesIO(v.value))
                                        except Exception:
                                            continue

            else:
                # Generic mutagen fallback for APE, MPC, WV, TAK, MKA
                audio = mutagen.File(str(path))
                if audio is not None and audio.tags:
                    for tag in audio.tags.values():
                        if hasattr(tag, "data") and hasattr(tag, "mime"):
                            return Image.open(io.BytesIO(tag.data))

        except ImportError:
            logger.warning("mutagen not installed — no album art extraction")
        except Exception as exc:
            logger.debug("Album art extraction failed for %s: %s", path, exc)

        return None

    # ── Waveform visualization ───────────────────────────────────────

    @staticmethod
    def _generate_waveform(path: Path, size: int) -> Optional[Image.Image]:
        """Generate a waveform visualization for WAV files."""
        try:
            with open(path, "rb") as f:
                # Read WAV header
                riff = f.read(4)
                if riff != b"RIFF":
                    return None
                f.read(4)  # file size
                wave = f.read(4)
                if wave != b"WAVE":
                    return None

                # Find data chunk
                channels = 1
                sample_width = 2
                while True:
                    chunk_id = f.read(4)
                    if len(chunk_id) < 4:
                        return None
                    chunk_size_data = f.read(4)
                    if len(chunk_size_data) < 4:
                        return None
                    chunk_size = struct.unpack("<I", chunk_size_data)[0]

                    if chunk_id == b"fmt ":
                        fmt_data = f.read(chunk_size)
                        if len(fmt_data) >= 4:
                            channels = struct.unpack("<H", fmt_data[2:4])[0]
                        if len(fmt_data) >= 8:
                            sample_width = struct.unpack("<H", fmt_data[14:16])[0] // 8
                    elif chunk_id == b"data":
                        # Read samples
                        max_bytes = min(chunk_size, 1024 * 1024)  # 1MB cap
                        raw = f.read(max_bytes)
                        break
                    else:
                        f.seek(chunk_size, 1)

            if not raw:
                return None

            # Parse int16 samples
            num_samples = len(raw) // (sample_width * channels)
            if num_samples == 0:
                return None

            fmt_char = "<h" if sample_width == 2 else "<b"
            max_val = 32767 if sample_width == 2 else 127

            # Downsample to 'size' bars
            bars = size
            samples_per_bar = max(1, num_samples // bars)
            amplitudes = []
            for i in range(bars):
                start = i * samples_per_bar * sample_width * channels
                end = start + samples_per_bar * sample_width * channels
                chunk = raw[start:end]
                if len(chunk) < sample_width:
                    break
                peak = 0
                for j in range(0, len(chunk), sample_width * channels):
                    if j + sample_width <= len(chunk):
                        val = struct.unpack(fmt_char, chunk[j : j + sample_width])[0]
                        peak = max(peak, abs(val))
                amplitudes.append(peak / max_val)

            if not amplitudes:
                return None

            # Render waveform
            canvas = Image.new("RGB", (size, size), (25, 25, 40))
            draw = ImageDraw.Draw(canvas)
            mid_y = size // 2
            bar_w = max(1, size // len(amplitudes))

            for i, amp in enumerate(amplitudes):
                h = int(amp * (size // 2 - 10))
                x = i * bar_w
                color = (
                    int(60 + 140 * amp),
                    int(130 + 100 * (1.0 - amp)),
                    255,
                )
                draw.rectangle(
                    [x, mid_y - h, x + bar_w - 1, mid_y + h],
                    fill=color,
                )

            return canvas

        except Exception as exc:
            logger.debug("Waveform generation failed for %s: %s", path, exc)
            return None

    # ── Placeholder generation ───────────────────────────────────────

    @staticmethod
    def _generate_placeholder(path: Path, size: int) -> Image.Image:
        """Generate a music-note-style placeholder thumbnail."""
        canvas = Image.new("RGB", (size, size), (30, 30, 45))
        draw = ImageDraw.Draw(canvas)

        # Draw a simple music note icon
        cx, cy = size // 2, size // 2
        r = size // 6

        # Note head (filled ellipse)
        draw.ellipse(
            [cx - r, cy + r // 2, cx + r, cy + r + r],
            fill=(100, 140, 255),
        )
        # Stem
        draw.line(
            [cx + r - 2, cy + r, cx + r - 2, cy - r * 2],
            fill=(100, 140, 255),
            width=max(2, size // 64),
        )

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
            fill=(180, 180, 200),
            font=font,
        )

        return canvas
