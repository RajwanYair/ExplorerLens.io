# ExplorerLens.py — Shell Diagnostics
# Copyright (c) 2026 ExplorerLens Project
"""
Diagnostic tools for troubleshooting shell extension registration.
Collects system info, registration status, and generates a portable report.
"""

from __future__ import annotations

import json
import logging
import os
import platform
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

logger = logging.getLogger("explorerlens.shell.diagnostics")


def collect_diagnostics() -> dict[str, Any]:
    """Collect comprehensive diagnostic information."""
    info: dict[str, Any] = {
        "timestamp": datetime.now(timezone.utc).isoformat(),
        "system": _system_info(),
        "python": _python_info(),
        "registration": _registration_info(),
        "dependencies": _dependency_info(),
        "config": _config_info(),
    }
    return info


def export_diagnostics(output_path: Path | None = None) -> Path:
    """Export diagnostics to a JSON file."""
    info = collect_diagnostics()
    if output_path is None:
        output_path = (
            Path.cwd() / f"explorerlens-diag-{datetime.now():%Y%m%d-%H%M%S}.json"
        )

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(info, f, indent=2, default=str)

    logger.info("Diagnostics exported to %s", output_path)
    return output_path


def _system_info() -> dict:
    return {
        "os": platform.platform(),
        "os_version": platform.version(),
        "architecture": platform.machine(),
        "processor": platform.processor(),
        "hostname": platform.node(),
    }


def _python_info() -> dict:
    return {
        "version": platform.python_version(),
        "executable": sys.executable,
        "prefix": sys.prefix,
        "is_64bit": sys.maxsize > 2**32,
    }


def _registration_info() -> dict:
    """Check COM registration status."""
    try:
        from .com_server import COM_CLSID_PY, get_registration_status

        status = get_registration_status()
        registered_count = sum(1 for v in status.values() if v)
        return {
            "clsid": COM_CLSID_PY,
            "total_extensions": len(status),
            "registered": registered_count,
            "unregistered": len(status) - registered_count,
        }
    except Exception as exc:
        return {"error": str(exc)}


def _dependency_info() -> dict:
    """Check availability of key dependencies."""
    deps: dict[str, str | None] = {}
    for mod_name in [
        "PIL",
        "rawpy",
        "fitz",
        "cairosvg",
        "mutagen",
        "py7zr",
        "rarfile",
        "fonttools",
        "trimesh",
        "comtypes",
        "win32api",
        "pystray",
        "darkdetect",
        "platformdirs",
        "watchdog",
        "psutil",
    ]:
        try:
            mod = __import__(mod_name)
            ver = getattr(mod, "__version__", getattr(mod, "VERSION", "installed"))
            deps[mod_name] = str(ver)
        except ImportError:
            deps[mod_name] = None
    return deps


def _config_info() -> dict:
    """Load current config summary."""
    try:
        from ..config import Config

        cfg = Config.load()
        enabled = cfg.get_enabled_extensions()
        return {
            "thumbnail_size": cfg.thumbnail_size,
            "dark_mode": cfg.dark_mode,
            "enabled_extensions": len(enabled),
            "cache_enabled": cfg.cache.enabled,
            "cache_memory_mb": cfg.cache.memory_limit_mb,
            "cache_disk_mb": cfg.cache.disk_limit_mb,
            "max_workers": cfg.performance.max_workers,
        }
    except Exception as exc:
        return {"error": str(exc)}
