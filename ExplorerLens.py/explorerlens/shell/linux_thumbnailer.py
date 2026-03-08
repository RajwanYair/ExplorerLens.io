# ExplorerLens.py — Linux Thumbnailer
# Copyright (c) 2026 ExplorerLens Project
"""
Freedesktop.org-compliant thumbnailer for Linux desktop environments
(GNOME, KDE, XFCE, etc.).

Implements the Thumbnail Managing Standard:
https://specifications.freedesktop.org/thumbnail-spec/latest/

Generates .thumbnailer files for /usr/share/thumbnailers/ and handles
the XDG thumbnail cache (~/.cache/thumbnails/).
"""

from __future__ import annotations

import hashlib
import logging
import os
import sys
from pathlib import Path
from typing import Optional

logger = logging.getLogger("explorerlens.shell.linux")

# XDG thumbnail directories
_XDG_CACHE = Path(os.environ.get("XDG_CACHE_HOME", Path.home() / ".cache"))
_THUMB_DIR_NORMAL = _XDG_CACHE / "thumbnails" / "normal"
_THUMB_DIR_LARGE = _XDG_CACHE / "thumbnails" / "large"
_THUMB_DIR_XLARGE = _XDG_CACHE / "thumbnails" / "x-large"
_THUMB_DIR_XXLARGE = _XDG_CACHE / "thumbnails" / "xx-large"

# Freedesktop thumbnailer install location
_THUMBNAILER_DIR = Path("/usr/share/thumbnailers")
_USER_THUMBNAILER_DIR = Path.home() / ".local" / "share" / "thumbnailers"


def _get_thumb_dir(size: int) -> Path:
    """Pick the correct cache directory by thumbnail size."""
    if size <= 128:
        return _THUMB_DIR_NORMAL
    if size <= 256:
        return _THUMB_DIR_LARGE
    if size <= 512:
        return _THUMB_DIR_XLARGE
    return _THUMB_DIR_XXLARGE


def get_thumb_path(uri: str, size: int) -> Path:
    """
    Compute the freedesktop thumbnail cache path for a file URI.

    Per spec: MD5(uri) + ".png" stored in the correct size bucket.
    """
    md5 = hashlib.md5(uri.encode("utf-8")).hexdigest()  # noqa: S324
    return _get_thumb_dir(size) / f"{md5}.png"


def generate_thumbnail(input_path: str, output_path: str, size: int) -> bool:
    """
    Generate a thumbnail and save it as PNG.

    Called by the desktop environment via the .thumbnailer entry.
    Conforms to: Exec=explorerlens-thumb -s %s %u %o
    """
    from ..config import Config
    from ..engine import DecodeStatus, ThumbnailEngine, ThumbnailRequest

    try:
        config = Config.load()
        engine = ThumbnailEngine(config)
        req = ThumbnailRequest(path=Path(input_path), size=size)
        result = engine.generate(req)
        engine.shutdown()

        if result.status == DecodeStatus.Success and result.image:
            out = Path(output_path)
            out.parent.mkdir(parents=True, exist_ok=True)
            result.image.save(str(out), "PNG")
            return True

        logger.debug(
            "Thumbnail generation failed for %s: %s",
            input_path,
            result.error,
        )
        return False
    except (OSError, ValueError, RuntimeError) as exc:
        logger.error("Thumbnailer error for %s: %s", input_path, exc)
        return False


def generate_thumbnailer_file(
    extensions: set[str] | None = None,
) -> str:
    """
    Generate the contents of a .thumbnailer file for freedesktop.

    Returns the file content as a string.
    """
    if extensions is None:
        from ..config import ALL_EXTENSIONS

        extensions = ALL_EXTENSIONS

    mime_types = _extensions_to_mimetypes(extensions)
    python_exe = sys.executable

    return (
        "[Thumbnailer Entry]\n"
        "TryExec=explorerlens-thumb\n"
        f"Exec={python_exe} -m explorerlens.shell.linux_thumbnailer"
        " -s %s %u %o\n"
        f"MimeType={';'.join(sorted(mime_types))};\n"
    )


def install_thumbnailer(
    extensions: set[str] | None = None,
    system_wide: bool = False,
) -> Optional[Path]:
    """
    Install a .thumbnailer file for the desktop environment.

    Args:
        extensions: Set of file extensions to register.
        system_wide: If True, install to /usr/share/thumbnailers/
                     (requires root). Otherwise ~/.local/share/thumbnailers/.

    Returns the path of the installed file, or None on failure.
    """
    target_dir = _THUMBNAILER_DIR if system_wide else _USER_THUMBNAILER_DIR

    try:
        target_dir.mkdir(parents=True, exist_ok=True)
        content = generate_thumbnailer_file(extensions)
        target = target_dir / "explorerlens.thumbnailer"
        target.write_text(content, encoding="utf-8")
        logger.info("Installed thumbnailer to %s", target)
        return target
    except OSError as exc:
        logger.error("Failed to install thumbnailer: %s", exc)
        return None


def uninstall_thumbnailer(system_wide: bool = False) -> bool:
    """Remove the .thumbnailer file."""
    target_dir = _THUMBNAILER_DIR if system_wide else _USER_THUMBNAILER_DIR
    target = target_dir / "explorerlens.thumbnailer"
    try:
        if target.exists():
            target.unlink()
            logger.info("Removed thumbnailer from %s", target)
        return True
    except OSError as exc:
        logger.error("Failed to remove thumbnailer: %s", exc)
        return False


# ── MIME type mapping ────────────────────────────────────────────────

_EXT_TO_MIME: dict[str, str] = {
    # Images
    ".png": "image/png",
    ".jpg": "image/jpeg",
    ".jpeg": "image/jpeg",
    ".gif": "image/gif",
    ".bmp": "image/bmp",
    ".tiff": "image/tiff",
    ".tif": "image/tiff",
    ".webp": "image/webp",
    ".avif": "image/avif",
    ".heic": "image/heic",
    ".heif": "image/heif",
    ".jxl": "image/jxl",
    ".svg": "image/svg+xml",
    ".ico": "image/x-icon",
    ".psd": "image/vnd.adobe.photoshop",
    ".tga": "image/x-tga",
    ".dds": "image/x-dds",
    ".hdr": "image/vnd.radiance",
    ".exr": "image/x-exr",
    ".qoi": "image/x-qoi",
    ".pcx": "image/x-pcx",
    ".ppm": "image/x-portable-pixmap",
    ".pgm": "image/x-portable-graymap",
    ".pbm": "image/x-portable-bitmap",
    # RAW camera
    ".cr2": "image/x-canon-cr2",
    ".cr3": "image/x-canon-cr3",
    ".nef": "image/x-nikon-nef",
    ".arw": "image/x-sony-arw",
    ".dng": "image/x-adobe-dng",
    ".orf": "image/x-olympus-orf",
    ".rw2": "image/x-panasonic-rw2",
    ".pef": "image/x-pentax-pef",
    ".raf": "image/x-fuji-raf",
    ".srw": "image/x-samsung-srw",
    # Video
    ".mp4": "video/mp4",
    ".avi": "video/x-msvideo",
    ".mkv": "video/x-matroska",
    ".mov": "video/quicktime",
    ".wmv": "video/x-ms-wmv",
    ".webm": "video/webm",
    ".flv": "video/x-flv",
    ".mpg": "video/mpeg",
    ".mpeg": "video/mpeg",
    ".3gp": "video/3gpp",
    ".ts": "video/mp2t",
    ".m2ts": "video/mp2t",
    ".ogv": "video/ogg",
    # Audio
    ".mp3": "audio/mpeg",
    ".wav": "audio/wav",
    ".flac": "audio/flac",
    ".ogg": "audio/ogg",
    ".opus": "audio/opus",
    ".m4a": "audio/mp4",
    ".wma": "audio/x-ms-wma",
    ".aac": "audio/aac",
    ".ape": "audio/x-ape",
    # Archives
    ".zip": "application/zip",
    ".rar": "application/vnd.rar",
    ".7z": "application/x-7z-compressed",
    ".tar": "application/x-tar",
    ".gz": "application/gzip",
    ".bz2": "application/x-bzip2",
    ".xz": "application/x-xz",
    ".zst": "application/zstd",
    # Comics / E-books
    ".cbz": "application/vnd.comicbook+zip",
    ".cbr": "application/vnd.comicbook-rar",
    ".epub": "application/epub+zip",
    # Documents
    ".pdf": "application/pdf",
    ".docx": "application/vnd.openxmlformats-officedocument"
    ".wordprocessingml.document",
    ".pptx": "application/vnd.openxmlformats-officedocument"
    ".presentationml.presentation",
    ".xlsx": "application/vnd.openxmlformats-officedocument" ".spreadsheetml.sheet",
    ".odt": "application/vnd.oasis.opendocument.text",
    ".odp": "application/vnd.oasis.opendocument.presentation",
    # Fonts
    ".ttf": "font/ttf",
    ".otf": "font/otf",
    ".woff": "font/woff",
    ".woff2": "font/woff2",
    # 3D models
    ".obj": "model/obj",
    ".stl": "model/stl",
    ".gltf": "model/gltf+json",
    ".glb": "model/gltf-binary",
}


def _extensions_to_mimetypes(extensions: set[str]) -> set[str]:
    """Convert file extensions to MIME types."""
    mimes: set[str] = set()
    for ext in extensions:
        mime = _EXT_TO_MIME.get(ext.lower())
        if mime:
            mimes.add(mime)
        else:
            # Fallback: application/x-ext-{ext}
            clean = ext.lstrip(".").lower()
            mimes.add(f"application/x-ext-{clean}")
    return mimes


# ── CLI entry point for the thumbnailer ──────────────────────────────


def main() -> None:
    """Entry point when called as explorerlens-thumb."""
    import argparse

    parser = argparse.ArgumentParser(
        description="ExplorerLens freedesktop thumbnailer",
    )
    parser.add_argument(
        "-s",
        "--size",
        type=int,
        default=256,
        help="Thumbnail size",
    )
    parser.add_argument("input", help="Input file URI or path")
    parser.add_argument("output", help="Output thumbnail path")
    args = parser.parse_args()

    # Handle file:// URIs
    input_path = args.input
    if input_path.startswith("file://"):
        from urllib.parse import unquote, urlparse

        parsed = urlparse(input_path)
        input_path = unquote(parsed.path)

    ok = generate_thumbnail(input_path, args.output, args.size)
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
