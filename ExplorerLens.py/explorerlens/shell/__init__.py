# ExplorerLens.py — Shell Package
# Copyright (c) 2026 ExplorerLens Project
"""Windows Shell Extension integration — COM IThumbnailProvider."""

from .com_server import register, unregister

__all__ = ["register", "unregister"]
