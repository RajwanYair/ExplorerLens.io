# ExplorerLens.py — Configuration Management
# Copyright (c) 2026 ExplorerLens Project
"""
Manages application settings with JSON persistence and Windows registry
integration. Mirrors CRegManager from ExplorerLens.io.
"""

from __future__ import annotations

import json
import os
from dataclasses import dataclass, field, asdict
from pathlib import Path
from typing import Any

from platformdirs import user_config_dir

APP_NAME = "ExplorerLens.py"
APP_AUTHOR = "ExplorerLens"
CONFIG_DIR = Path(user_config_dir(APP_NAME, APP_AUTHOR))
CONFIG_FILE = CONFIG_DIR / "config.json"

# COM CLSID — must match ExplorerLens.io for shared registration
COM_CLSID = "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}"
COM_CLSID_PY = "{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}"  # Python-specific

# Format categories mirroring LENSTYPE from LENSArchive.h
FORMAT_CATEGORIES: dict[str, list[str]] = {
    "comic": [".cbz", ".cbr", ".cb7", ".cbt"],
    "archive": [".zip", ".rar", ".7z", ".tar", ".tar.gz", ".tar.bz2",
                ".tar.xz", ".tar.zst", ".gz", ".bz2", ".xz", ".zst",
                ".lz4", ".cpio", ".iso", ".cab", ".ar", ".deb"],
    "ebook": [".epub", ".mobi", ".fb2", ".azw", ".azw3", ".phz"],
    "image": [".webp", ".avif", ".heic", ".heif", ".jxl", ".tiff", ".tif",
              ".svg", ".psd", ".dds", ".hdr", ".exr", ".ppm", ".pgm", ".pbm",
              ".ico", ".qoi", ".tga", ".bmp", ".gif", ".png", ".jpg", ".jpeg",
              ".jp2", ".eps", ".pcx", ".sgi", ".xpm", ".xcf", ".ora",
              ".jxr", ".ktx", ".vtf", ".farbfeld"],
    "raw": [".cr2", ".cr3", ".nef", ".arw", ".dng", ".orf", ".rw2",
            ".pef", ".srw", ".raf", ".raw", ".3fr", ".dcr", ".kdc",
            ".mrw", ".nrw", ".rwl", ".sr2", ".srf", ".x3f"],
    "video": [".avi", ".wmv", ".asf", ".mpg", ".mpeg", ".m1v", ".m2v",
              ".ts", ".m2ts", ".mts", ".m2t", ".mp4", ".m4v", ".mp4v",
              ".mov", ".3g2", ".3gp", ".3gp2", ".3gpp", ".mkv", ".mk3d",
              ".webm", ".flv", ".f4v", ".ogm", ".ogv", ".rm", ".rmvb",
              ".dv", ".mxf", ".ivf", ".evo", ".264", ".video"],
    "audio": [".mp3", ".wav", ".m4a", ".ape", ".flac", ".ogg", ".mka",
              ".mpc", ".opus", ".tak", ".wv", ".wma", ".aac"],
    "document": [".pdf", ".djvu", ".chm", ".docx", ".pptx", ".xlsx",
                 ".doc", ".ppt", ".xls", ".odt", ".odp"],
    "font": [".ttf", ".otf", ".woff", ".woff2"],
    "model": [".obj", ".stl", ".ply", ".gltf", ".glb", ".fbx", ".3ds"],
}

ALL_EXTENSIONS: set[str] = set()
for exts in FORMAT_CATEGORIES.values():
    ALL_EXTENSIONS.update(exts)


@dataclass
class CacheConfig:
    """Cache settings mirroring SubMillisecondCacheEngine."""
    enabled: bool = True
    memory_limit_mb: int = 256
    disk_limit_mb: int = 1024
    max_items: int = 10000
    ttl_seconds: int = 3600


@dataclass
class GPUConfig:
    """GPU acceleration settings."""
    enabled: bool = False
    backend: str = "auto"  # auto, cuda, cpu


@dataclass
class PerformanceConfig:
    """Performance tuning."""
    max_workers: int = 4
    thumbnail_timeout_ms: int = 5000
    batch_size: int = 50
    use_simd: bool = True


@dataclass
class Config:
    """Main configuration — mirrors ExplorerLens.io's registry-based settings."""
    enabled_categories: dict[str, bool] = field(default_factory=lambda: {
        cat: True for cat in FORMAT_CATEGORIES
    })
    thumbnail_size: int = 256
    dark_mode: str = "auto"  # auto, dark, light
    cache: CacheConfig = field(default_factory=CacheConfig)
    gpu: GPUConfig = field(default_factory=GPUConfig)
    performance: PerformanceConfig = field(default_factory=PerformanceConfig)
    plugin_dir: str = ""
    log_level: str = "INFO"
    show_tray_icon: bool = True
    auto_register: bool = False

    def get_enabled_extensions(self) -> set[str]:
        """Return the set of currently enabled file extensions."""
        result: set[str] = set()
        for cat, enabled in self.enabled_categories.items():
            if enabled and cat in FORMAT_CATEGORIES:
                result.update(FORMAT_CATEGORIES[cat])
        return result

    def save(self, path: Path | None = None) -> None:
        """Save config to JSON file."""
        target = path or CONFIG_FILE
        target.parent.mkdir(parents=True, exist_ok=True)
        with open(target, "w", encoding="utf-8") as f:
            json.dump(asdict(self), f, indent=2)

    @classmethod
    def load(cls, path: Path | None = None) -> Config:
        """Load config from JSON file, or return defaults."""
        target = path or CONFIG_FILE
        if not target.exists():
            cfg = cls()
            cfg.save(target)
            return cfg
        try:
            with open(target, "r", encoding="utf-8") as f:
                data = json.load(f)
            return cls._from_dict(data)
        except (json.JSONDecodeError, KeyError, TypeError):
            return cls()

    @classmethod
    def _from_dict(cls, data: dict[str, Any]) -> Config:
        cfg = cls()
        cfg.enabled_categories = data.get("enabled_categories",
                                          cfg.enabled_categories)
        cfg.thumbnail_size = data.get("thumbnail_size", 256)
        cfg.dark_mode = data.get("dark_mode", "auto")
        cfg.log_level = data.get("log_level", "INFO")
        cfg.show_tray_icon = data.get("show_tray_icon", True)
        cfg.auto_register = data.get("auto_register", False)
        cfg.plugin_dir = data.get("plugin_dir", "")
        if "cache" in data:
            cfg.cache = CacheConfig(**data["cache"])
        if "gpu" in data:
            cfg.gpu = GPUConfig(**data["gpu"])
        if "performance" in data:
            cfg.performance = PerformanceConfig(**data["performance"])
        return cfg

    def export_json(self, path: str) -> None:
        """Export config to an arbitrary JSON file."""
        self.save(Path(path))

    @classmethod
    def import_json(cls, path: str) -> Config:
        """Import config from an arbitrary JSON file."""
        return cls.load(Path(path))

    def reset_defaults(self) -> None:
        """Reset to factory defaults."""
        default = Config()
        self.__dict__.update(default.__dict__)
