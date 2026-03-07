# ExplorerLens.py — Plugin Loader
# Copyright (c) 2026 ExplorerLens Project
"""
Discovers and loads third-party decoder plugins from a plugin directory.
Plugins are Python modules that define a `create_decoder()` factory function
returning a BaseDecoder instance.

Mirrors the Plugin SDK from ExplorerLens.io (plugin_api.h / ICADDecoderPlugin).
"""

from __future__ import annotations

import importlib
import importlib.util
import logging
from pathlib import Path
from typing import Optional

from ..decoders.base import BaseDecoder

logger = logging.getLogger("explorerlens.plugins")


class PluginLoader:
    """
    Discovers .py plugin files in a directory and loads decoders from them.

    Each plugin module must expose:
        def create_decoder() -> BaseDecoder
    """

    def __init__(self, plugin_dir: str | Path) -> None:
        self._plugin_dir = Path(plugin_dir)
        self._loaded: dict[str, BaseDecoder] = {}

    def discover(self) -> list[BaseDecoder]:
        """Scan plugin directory and load all valid plugins."""
        if not self._plugin_dir.is_dir():
            logger.info("Plugin directory not found: %s", self._plugin_dir)
            return []

        decoders: list[BaseDecoder] = []
        for py_file in sorted(self._plugin_dir.glob("*.py")):
            if py_file.name.startswith("_"):
                continue
            decoder = self._load_plugin(py_file)
            if decoder is not None:
                decoders.append(decoder)
                self._loaded[py_file.stem] = decoder
                logger.info("Loaded plugin: %s → %s",
                            py_file.name, decoder.name)

        return decoders

    @staticmethod
    def _load_plugin(path: Path) -> Optional[BaseDecoder]:
        """Load a single plugin module and extract its decoder."""
        try:
            spec = importlib.util.spec_from_file_location(
                f"explorerlens_plugin_{path.stem}", path
            )
            if spec is None or spec.loader is None:
                return None

            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)

            factory = getattr(module, "create_decoder", None)
            if factory is None:
                logger.warning("Plugin %s has no create_decoder()", path.name)
                return None

            decoder = factory()
            if not isinstance(decoder, BaseDecoder):
                logger.warning("Plugin %s: create_decoder() did not return "
                               "BaseDecoder", path.name)
                return None

            return decoder
        except Exception as exc:
            logger.error("Failed to load plugin %s: %s", path.name, exc)
            return None

    @property
    def loaded_plugins(self) -> dict[str, BaseDecoder]:
        return dict(self._loaded)
