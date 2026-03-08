# ExplorerLens.py — Platform Thumbnail Provider
# Copyright (c) 2026 ExplorerLens Project
"""
Cross-platform thumbnail provider abstraction.

Detects the current OS and delegates to the appropriate backend:
- Windows: COM IThumbnailProvider via shell.com_server
- Linux:   Freedesktop.org .thumbnailer via shell.linux_thumbnailer

Usage:
    from explorerlens.shell.platform_provider import (
        register_provider, unregister_provider, get_platform
    )
    register_provider(extensions)
"""

from __future__ import annotations

import logging
import sys
from pathlib import Path
from typing import Optional

logger = logging.getLogger("explorerlens.shell.platform")


class Platform:
    """Detected platform constants."""

    WINDOWS = "windows"
    LINUX = "linux"
    MACOS = "macos"
    UNKNOWN = "unknown"


def get_platform() -> str:
    """Detect the current operating system."""
    if sys.platform == "win32":
        return Platform.WINDOWS
    if sys.platform == "linux":
        return Platform.LINUX
    if sys.platform == "darwin":
        return Platform.MACOS
    return Platform.UNKNOWN


def register_provider(
    extensions: set[str] | None = None,
    system_wide: bool = False,
) -> bool:
    """
    Register the thumbnail provider for the current platform.

    Args:
        extensions: File extensions to register (None = all).
        system_wide: Install system-wide (requires elevated privileges).

    Returns True on success.
    """
    platform = get_platform()

    if platform == Platform.WINDOWS:
        from .com_server import register

        return register(extensions)

    if platform == Platform.LINUX:
        from .linux_thumbnailer import install_thumbnailer

        result = install_thumbnailer(extensions, system_wide)
        return result is not None

    if platform == Platform.MACOS:
        logger.warning(
            "macOS Quick Look plugin not yet implemented. "
            "Use the CLI: python -m explorerlens --thumbnail FILE"
        )
        return False

    logger.error("Unsupported platform: %s", sys.platform)
    return False


def unregister_provider(system_wide: bool = False) -> bool:
    """
    Remove the thumbnail provider registration.

    Returns True on success.
    """
    platform = get_platform()

    if platform == Platform.WINDOWS:
        from .com_server import unregister

        return unregister()

    if platform == Platform.LINUX:
        from .linux_thumbnailer import uninstall_thumbnailer

        return uninstall_thumbnailer(system_wide)

    if platform == Platform.MACOS:
        logger.info("macOS Quick Look: no cleanup needed")
        return True

    return False


def get_provider_status() -> dict[str, object]:
    """
    Return registration status for the current platform.

    Returns a dict with platform info and registration details.
    """
    platform = get_platform()
    info: dict[str, object] = {"platform": platform}

    if platform == Platform.WINDOWS:
        from .com_server import get_registration_status

        info["registered_extensions"] = get_registration_status()

    elif platform == Platform.LINUX:
        from .linux_thumbnailer import _THUMBNAILER_DIR, _USER_THUMBNAILER_DIR

        user_file = _USER_THUMBNAILER_DIR / "explorerlens.thumbnailer"
        sys_file = _THUMBNAILER_DIR / "explorerlens.thumbnailer"
        info["user_thumbnailer"] = user_file.exists()
        info["system_thumbnailer"] = sys_file.exists()

    return info


def get_thumbnail_path(
    file_path: str,
    size: int = 256,
) -> Optional[Path]:
    """
    Get the cached thumbnail path for a file (Linux XDG only).

    On Windows, thumbnails are managed by Explorer's thumbcache.
    """
    platform = get_platform()

    if platform == Platform.LINUX:
        from urllib.parse import quote

        from .linux_thumbnailer import get_thumb_path

        abs_path = str(Path(file_path).resolve())
        uri = "file://" + quote(abs_path)
        return get_thumb_path(uri, size)

    return None
