# ExplorerLens.py — Decoder Base Class
# Copyright (c) 2026 ExplorerLens Project
"""Abstract base for all thumbnail decoders."""

from __future__ import annotations

import abc
from pathlib import Path
from typing import Optional

from PIL import Image


class BaseDecoder(abc.ABC):
    """
    Interface that all decoders must implement.
    Mirrors IThumbnailDecoder from ExplorerLens.io Plugin SDK.
    """

    @property
    @abc.abstractmethod
    def name(self) -> str:
        """Human-readable decoder name."""
        ...

    @abc.abstractmethod
    def supported_extensions(self) -> list[str]:
        """Return list of file extensions this decoder handles (with dot)."""
        ...

    @abc.abstractmethod
    def decode(self, path: Path, size: int) -> Optional[Image.Image]:
        """
        Decode the file at *path* and return a thumbnail image,
        or None if decoding fails.
        """
        ...

    def can_decode(self, path: Path) -> bool:
        """Quick check — override for more sophisticated probing."""
        return path.suffix.lower() in self.supported_extensions()
