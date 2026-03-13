# ExplorerLens.py — macOS Quick Look Thumbnail Generator
# Copyright (c) 2026 ExplorerLens Project
"""
macOS Quick Look thumbnail generator.

Generates thumbnails compatible with macOS Quick Look by producing
PNG files and optionally installing a Quick Look generator bundle.

macOS Quick Look supports two integration paths:
  1. Quick Look Generator plugin (.qlgenerator bundle) — deep integration
  2. Thumbnail extension (App Extension) — modern macOS 12+ approach

This module provides a Python-based generator that can be wrapped
by either approach, plus a CLI fallback for direct invocation.

Usage:
    from explorerlens.shell.macos_quicklook import (
        generate_thumbnail, install_quicklook, uninstall_quicklook
    )
"""

from __future__ import annotations

import logging
import os
import plistlib
import shutil
import sys
from pathlib import Path
from typing import Optional

logger = logging.getLogger("explorerlens.shell.macos")

# Quick Look generator paths
_SYSTEM_QL_DIR = Path("/Library/QuickLook")
_USER_QL_DIR = Path.home() / "Library" / "QuickLook"

# Thumbnail cache (macOS uses QuickLookThumbnailing framework)
_CACHE_DIR = Path.home() / "Library" / "Caches" / "com.explorerlens.thumbnails"


def generate_thumbnail(
    input_path: str,
    output_path: str,
    size: int = 256,
) -> bool:
    """
    Generate a thumbnail for a file and save as PNG.

    Called by the Quick Look generator wrapper or directly via CLI.

    Args:
        input_path: Source file to thumbnail.
        output_path: Destination path for the PNG thumbnail.
        size: Maximum thumbnail dimension in pixels.

    Returns True on success.
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
        logger.error("Quick Look error for %s: %s", input_path, exc)
        return False


def _get_supported_utis() -> list[str]:
    """
    Return Uniform Type Identifiers for supported formats.

    macOS uses UTIs instead of MIME types for Quick Look registration.
    """
    return [
        # Images
        "public.png",
        "public.jpeg",
        "com.compuserve.gif",
        "public.tiff",
        "com.microsoft.bmp",
        "public.svg-image",
        "org.webmproject.webp",
        "public.avif",
        "public.heic",
        "public.heif",
        "public.jxl",
        "com.adobe.photoshop-image",
        "com.truevision.tga-image",
        "com.microsoft.dds",
        "public.radiance",
        "com.ilm.openexr-image",
        # RAW camera
        "com.adobe.raw-image",
        "com.canon.cr2-raw-image",
        "com.canon.cr3-raw-image",
        "com.nikon.nrw-raw-image",
        "com.sony.arw-raw-image",
        "com.olympus.or-raw-image",
        "com.fuji.raw-image",
        "com.panasonic.rw2-raw-image",
        # Video
        "public.mpeg-4",
        "public.avi",
        "org.matroska.mkv",
        "com.apple.quicktime-movie",
        "com.microsoft.windows-media-wmv",
        "org.webmproject.webm",
        # Audio
        "public.mp3",
        "com.microsoft.waveform-audio",
        "org.xiph.flac",
        "public.aac-audio",
        # Archives
        "com.pkware.zip-archive",
        "com.rarlab.rar-archive",
        "org.7-zip.7-zip-archive",
        "public.tar-archive",
        # Comics & E-books
        "public.zip-archive",  # CBZ
        "org.idpf.epub-container",
        # Documents
        "com.adobe.pdf",
        "org.openxmlformats.wordprocessingml.document",
        "org.openxmlformats.presentationml.presentation",
        "org.openxmlformats.spreadsheetml.sheet",
        # Fonts
        "public.truetype-ttf-font",
        "public.opentype-font",
        # 3D models
        "public.geometry-definition-format",
        "public.standard-tesselated-geometry-format",
    ]


def _generate_info_plist() -> dict:
    """Generate the Info.plist dictionary for the qlgenerator bundle."""
    return {
        "CFBundleDevelopmentRegion": "en",
        "CFBundleDisplayName": "ExplorerLens Quick Look",
        "CFBundleDocumentTypes": [
            {
                "CFBundleTypeRole": "QLGenerator",
                "LSItemContentTypes": _get_supported_utis(),
            }
        ],
        "CFBundleExecutable": "explorerlens-ql",
        "CFBundleIdentifier": "io.explorerlens.quicklook",
        "CFBundleInfoDictionaryVersion": "6.0",
        "CFBundleName": "ExplorerLens Quick Look Generator",
        "CFBundlePackageType": "BNDL",
        "CFBundleShortVersionString": "15.0.0",
        "CFBundleVersion": "1",
        "NSHumanReadableCopyright": "Copyright 2026 ExplorerLens Project",
        "QLNeedsToBeRunInMainThread": False,
        "QLPreviewHeight": 600,
        "QLPreviewWidth": 800,
        "QLSupportsConcurrentRequests": True,
        "QLThumbnailMinimumSize": 17,
    }


def install_quicklook(
    system_wide: bool = False,
) -> Optional[Path]:
    """
    Install the Quick Look generator bundle.

    Creates a .qlgenerator bundle with Info.plist and a shell wrapper
    that invokes the Python thumbnail generator.

    Args:
        system_wide: Install to /Library/QuickLook/ (requires root)
                     or ~/Library/QuickLook/ (user-only).

    Returns the path to the installed bundle, or None on failure.
    """
    target_dir = _SYSTEM_QL_DIR if system_wide else _USER_QL_DIR
    bundle_path = target_dir / "ExplorerLens.qlgenerator"

    try:
        # Create bundle directory structure
        contents_dir = bundle_path / "Contents"
        macos_dir = contents_dir / "MacOS"
        macos_dir.mkdir(parents=True, exist_ok=True)

        # Write Info.plist
        plist_path = contents_dir / "Info.plist"
        with open(plist_path, "wb") as f:
            plistlib.dump(_generate_info_plist(), f)

        # Write executable wrapper script
        python_exe = sys.executable
        exec_path = macos_dir / "explorerlens-ql"
        exec_path.write_text(
            "#!/bin/bash\n"
            "# ExplorerLens Quick Look Generator\n"
            f'exec "{python_exe}" -m explorerlens.shell.macos_quicklook'
            ' "$@"\n',
            encoding="utf-8",
        )
        exec_path.chmod(0o755)

        # Refresh Quick Look server
        os.system("qlmanage -r 2>/dev/null")  # noqa: S605

        logger.info("Installed Quick Look generator to %s", bundle_path)
        return bundle_path

    except OSError as exc:
        logger.error("Failed to install Quick Look generator: %s", exc)
        return None


def uninstall_quicklook(system_wide: bool = False) -> bool:
    """
    Remove the Quick Look generator bundle.

    Returns True on success.
    """
    target_dir = _SYSTEM_QL_DIR if system_wide else _USER_QL_DIR
    bundle_path = target_dir / "ExplorerLens.qlgenerator"

    try:
        if bundle_path.exists():
            shutil.rmtree(bundle_path)
            logger.info("Removed Quick Look generator from %s", bundle_path)

            # Refresh Quick Look server
            os.system("qlmanage -r 2>/dev/null")  # noqa: S605

        return True
    except OSError as exc:
        logger.error("Failed to remove Quick Look generator: %s", exc)
        return False


def get_quicklook_status() -> dict[str, object]:
    """Return installation status of the Quick Look generator."""
    info: dict[str, object] = {}
    user_bundle = _USER_QL_DIR / "ExplorerLens.qlgenerator"
    sys_bundle = _SYSTEM_QL_DIR / "ExplorerLens.qlgenerator"
    info["user_installed"] = user_bundle.exists()
    info["system_installed"] = sys_bundle.exists()
    info["supported_utis"] = len(_get_supported_utis())
    return info


# ── CLI entry point ──────────────────────────────────────────────────


def main() -> None:
    """Entry point for Quick Look generator invocation."""
    import argparse

    parser = argparse.ArgumentParser(
        description="ExplorerLens macOS Quick Look thumbnail generator",
    )
    parser.add_argument(
        "-s",
        "--size",
        type=int,
        default=256,
        help="Thumbnail size (default: 256)",
    )
    parser.add_argument(
        "input",
        help="Input file path",
    )
    parser.add_argument(
        "output",
        help="Output PNG path",
    )
    args = parser.parse_args()

    ok = generate_thumbnail(args.input, args.output, args.size)
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
