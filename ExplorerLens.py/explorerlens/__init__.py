# ExplorerLens.py — Python Thumbnail Provider for Windows
# Copyright (c) 2026 ExplorerLens Project
#
# Pure-Python port of ExplorerLens.io v15.0.0 "Zenith"
# Standalone side-project: full Python-based thumbnail generation,
# Windows Shell Extension, and GUI manager.

__version__ = "1.0.0"
__codename__ = "Zenith-Py"

from explorerlens.config import Config
from explorerlens.engine import ThumbnailEngine

__all__ = ["ThumbnailEngine", "Config", "__version__", "__codename__"]
