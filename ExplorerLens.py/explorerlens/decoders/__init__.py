# ExplorerLens.py — Decoder Package
# Copyright (c) 2026 ExplorerLens Project
"""Decoder registry — collects all format-specific decoders."""

from __future__ import annotations

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .base import BaseDecoder


def get_all_decoders() -> list[BaseDecoder]:
    """Instantiate and return all available decoders."""
    from .image_decoder import ImageDecoder
    from .archive_decoder import ArchiveDecoder
    from .video_decoder import VideoDecoder
    from .audio_decoder import AudioDecoder
    from .document_decoder import DocumentDecoder
    from .font_decoder import FontDecoder
    from .model_decoder import ModelDecoder

    return [
        ImageDecoder(),
        ArchiveDecoder(),
        VideoDecoder(),
        AudioDecoder(),
        DocumentDecoder(),
        FontDecoder(),
        ModelDecoder(),
    ]
