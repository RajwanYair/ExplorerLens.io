# ExplorerLens.py — Configuration Management
# Copyright (c) 2026 ExplorerLens Project
"""
Manages application settings with JSON persistence and Windows registry
integration. Mirrors CRegManager from ExplorerLens.io.
"""

from __future__ import annotations

import json
from dataclasses import asdict, dataclass, field
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

# Per-format definitions mirroring LENSTYPE + IDC_CB_* from LENSManager
# Each entry: (key, display_name, extensions, tooltip)
FORMAT_REGISTRY: dict[str, dict] = {
    # Comic Book Formats
    "cbz": {
        "name": "CBZ",
        "exts": [".cbz"],
        "group": "comic",
        "tip": "Comic Book ZIP Archive",
    },
    "cbr": {
        "name": "CBR",
        "exts": [".cbr"],
        "group": "comic",
        "tip": "Comic Book RAR Archive — Requires UnRAR",
    },
    "cb7": {
        "name": "CB7",
        "exts": [".cb7"],
        "group": "comic",
        "tip": "Comic Book 7-Zip Archive",
    },
    "cbt": {
        "name": "CBT",
        "exts": [".cbt"],
        "group": "comic",
        "tip": "Comic Book TAR Archive",
    },
    # E-Book Formats
    "epub": {
        "name": "EPUB",
        "exts": [".epub"],
        "group": "ebook",
        "tip": "Electronic Publication — Cover extraction from metadata",
    },
    "mobi": {
        "name": "MOBI",
        "exts": [".mobi"],
        "group": "ebook",
        "tip": "Mobipocket E-Book",
    },
    "azw": {
        "name": "AZW",
        "exts": [".azw"],
        "group": "ebook",
        "tip": "Amazon Kindle Format",
    },
    "azw3": {
        "name": "AZW3",
        "exts": [".azw3"],
        "group": "ebook",
        "tip": "Kindle Format 8 (KF8)",
    },
    # Archive Formats
    "zip": {
        "name": "ZIP",
        "exts": [".zip"],
        "group": "archive",
        "tip": "Standard ZIP Archive — Thumbnails from first image inside",
    },
    "rar": {
        "name": "RAR",
        "exts": [".rar"],
        "group": "archive",
        "tip": "WinRAR Archive — Requires UnRAR",
    },
    "7z": {
        "name": "7Z",
        "exts": [".7z"],
        "group": "archive",
        "tip": "7-Zip Archive — High compression ratio",
    },
    "tar": {
        "name": "TAR",
        "exts": [".tar", ".tar.gz", ".tar.bz2", ".tar.xz", ".tar.zst"],
        "group": "archive",
        "tip": "Tape Archive — Supports compressed variants",
    },
    # Photo Album & Other
    "phz": {
        "name": "PHZ",
        "exts": [".phz"],
        "group": "photo",
        "tip": "Photo ZIP Album — Optimized for photo collections",
    },
    "fb2": {
        "name": "FB2",
        "exts": [".fb2"],
        "group": "photo",
        "tip": "FictionBook E-Book — XML-based format",
    },
    # Modern Image Formats
    "webp": {
        "name": "WebP",
        "exts": [".webp"],
        "group": "image",
        "tip": "Modern Image Format — Fast, built-in decoder",
    },
    "heif": {
        "name": "HEIF/HEIC",
        "exts": [".heif", ".heic"],
        "group": "image",
        "tip": "High Efficiency Image — iPhone photos",
    },
    "avif": {
        "name": "AVIF",
        "exts": [".avif"],
        "group": "image",
        "tip": "AV1 Image Format — Fast via WIC",
    },
    "jxl": {
        "name": "JPEG XL",
        "exts": [".jxl"],
        "group": "image",
        "tip": "Next-Gen Image Format",
    },
    # Media & Documents
    "video": {
        "name": "Video",
        "exts": [
            ".mp4",
            ".m4v",
            ".mp4v",
            ".avi",
            ".mkv",
            ".mk3d",
            ".mov",
            ".wmv",
            ".asf",
            ".webm",
            ".flv",
            ".f4v",
            ".mpg",
            ".mpeg",
            ".m1v",
            ".m2v",
            ".ts",
            ".m2ts",
            ".mts",
            ".m2t",
            ".3gp",
            ".3g2",
            ".3gp2",
            ".3gpp",
            ".ogm",
            ".ogv",
            ".rm",
            ".rmvb",
            ".dv",
            ".mxf",
            ".ivf",
            ".evo",
            ".264",
        ],
        "group": "media",
        "tip": "Video Files — Keyframe extraction via FFmpeg",
    },
    "pdf": {
        "name": "PDF",
        "exts": [".pdf"],
        "group": "media",
        "tip": "PDF Documents — Uses embedded thumbnails when available",
    },
    "tiff": {
        "name": "TIFF",
        "exts": [".tif", ".tiff"],
        "group": "media",
        "tip": "TIFF Images — Supports multi-page",
    },
    "svg": {
        "name": "SVG",
        "exts": [".svg", ".svgz"],
        "group": "media",
        "tip": "SVG Vector Graphics — Scalable vector format",
    },
    "raw": {
        "name": "RAW",
        "exts": [
            ".cr2",
            ".cr3",
            ".nef",
            ".arw",
            ".dng",
            ".orf",
            ".rw2",
            ".pef",
            ".raf",
            ".srw",
            ".nrw",
        ],
        "group": "media",
        "tip": "RAW Camera Photos — Requires LibRaw",
    },
    # Professional Image Formats
    "psd": {
        "name": "PSD",
        "exts": [".psd"],
        "group": "pro_image",
        "tip": "Adobe Photoshop Document",
    },
    "dds": {
        "name": "DDS",
        "exts": [".dds"],
        "group": "pro_image",
        "tip": "DirectDraw Surface — GPU texture format",
    },
    "hdr": {
        "name": "HDR",
        "exts": [".hdr"],
        "group": "pro_image",
        "tip": "Radiance HDR Image",
    },
    "exr": {
        "name": "EXR",
        "exts": [".exr"],
        "group": "pro_image",
        "tip": "OpenEXR — Industry-standard HDR format",
    },
    "ppm": {
        "name": "PPM",
        "exts": [".ppm", ".pgm", ".pbm"],
        "group": "pro_image",
        "tip": "NetPBM Portable Image Formats",
    },
    "ico": {
        "name": "ICO",
        "exts": [".ico"],
        "group": "pro_image",
        "tip": "Windows Icon Format",
    },
    "qoi": {
        "name": "QOI",
        "exts": [".qoi"],
        "group": "pro_image",
        "tip": "Quite OK Image — Fast lossless format",
    },
    "tga": {
        "name": "TGA",
        "exts": [".tga"],
        "group": "pro_image",
        "tip": "Truevision TGA — Legacy image format",
    },
    # Specialized Formats
    "audio": {
        "name": "Audio",
        "exts": [
            ".mp3",
            ".wav",
            ".m4a",
            ".flac",
            ".ogg",
            ".opus",
            ".wma",
            ".aac",
            ".ape",
            ".mka",
            ".mpc",
            ".tak",
            ".wv",
        ],
        "group": "specialized",
        "tip": "Audio Files — Cover art / waveform extraction",
    },
    "document": {
        "name": "Document",
        "exts": [
            ".docx",
            ".pptx",
            ".xlsx",
            ".doc",
            ".ppt",
            ".xls",
            ".odt",
            ".odp",
        ],
        "group": "specialized",
        "tip": "Office Documents — Embedded preview extraction",
    },
    "font": {
        "name": "Font",
        "exts": [".ttf", ".otf", ".woff", ".woff2"],
        "group": "specialized",
        "tip": "Font Files — Glyph preview rendering",
    },
    "model": {
        "name": "3D Model",
        "exts": [".obj", ".stl", ".ply", ".gltf", ".glb", ".fbx", ".3ds"],
        "group": "specialized",
        "tip": "3D Models — Wireframe/rendered preview",
    },
    # Extended Formats
    "ext_image": {
        "name": "Extended Image",
        "exts": [
            ".bmp",
            ".gif",
            ".png",
            ".jpg",
            ".jpeg",
            ".jp2",
            ".eps",
            ".pcx",
            ".xcf",
            ".jxr",
            ".ktx",
            ".vtf",
        ],
        "group": "extended",
        "tip": "Additional image formats via WIC/fallback",
    },
    "texture": {
        "name": "Texture",
        "exts": [".ktx", ".vtf", ".dds"],
        "group": "extended",
        "tip": "Game/GPU Texture formats",
    },
    "ext_archive": {
        "name": "Extended Archive",
        "exts": [
            ".gz",
            ".bz2",
            ".xz",
            ".zst",
            ".lz4",
            ".cpio",
            ".iso",
            ".cab",
            ".ar",
            ".deb",
        ],
        "group": "extended",
        "tip": "Additional archive formats via libarchive",
    },
    "ext_document": {
        "name": "Extended Document",
        "exts": [".djvu", ".chm"],
        "group": "extended",
        "tip": "Additional document formats",
    },
}

# Group display order and labels (mirrors LENSManager groupbox layout)
FORMAT_GROUPS: dict[str, str] = {
    "comic": "Comic Book Formats",
    "ebook": "E-Book Formats",
    "archive": "Archive Formats",
    "photo": "Photo & Other",
    "image": "Modern Image Formats",
    "media": "Media & Documents",
    "pro_image": "Professional Image Formats",
    "specialized": "Specialized Formats",
    "extended": "Extended Formats",
}

# Legacy category mapping for backward compatibility
FORMAT_CATEGORIES: dict[str, list[str]] = {}
for _key, _info in FORMAT_REGISTRY.items():
    _group = _info["group"]
    if _group not in FORMAT_CATEGORIES:
        FORMAT_CATEGORIES[_group] = []
    FORMAT_CATEGORIES[_group].extend(_info["exts"])

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
    """Main configuration — mirrors ExplorerLens.io registry settings."""

    # Per-format enabled state (mirrors per-checkbox state in LENSManager)
    enabled_formats: dict[str, bool] = field(
        default_factory=lambda: {key: True for key in FORMAT_REGISTRY}
    )
    # Legacy category map (derived on load for backward compat)
    enabled_categories: dict[str, bool] = field(
        default_factory=lambda: {cat: True for cat in FORMAT_CATEGORIES}
    )
    thumbnail_size: int = 256
    dark_mode: str = "auto"  # auto, dark, light
    collage_mode: int = 1  # 1=single, 4=2x2, 9=3x3, 16=4x4
    sort_thumbnails: bool = False
    show_archive_icon: bool = True
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
        for fmt_key, enabled in self.enabled_formats.items():
            if enabled and fmt_key in FORMAT_REGISTRY:
                result.update(FORMAT_REGISTRY[fmt_key]["exts"])
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
        cfg.enabled_formats = data.get("enabled_formats", cfg.enabled_formats)
        cfg.enabled_categories = data.get("enabled_categories", cfg.enabled_categories)
        cfg.thumbnail_size = data.get("thumbnail_size", 256)
        cfg.dark_mode = data.get("dark_mode", "auto")
        cfg.collage_mode = data.get("collage_mode", 1)
        cfg.sort_thumbnails = data.get("sort_thumbnails", False)
        cfg.show_archive_icon = data.get("show_archive_icon", True)
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
