# ExplorerLens.py — Shell Package
# Copyright (c) 2026 ExplorerLens Project
"""
Cross-platform shell thumbnail integration.

- Windows: COM IThumbnailProvider (com_server)
- Linux:   Freedesktop.org thumbnailer (linux_thumbnailer)
- Unified: platform_provider auto-detects the OS
"""

from .platform_provider import (
    get_platform,
    get_provider_status,
    register_provider,
    unregister_provider,
)

__all__ = [
    "get_platform",
    "get_provider_status",
    "register_provider",
    "unregister_provider",
]
