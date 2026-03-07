# ExplorerLens.py — Registry Manager
# Copyright (c) 2026 ExplorerLens Project
"""
Windows registry utilities for thumbnail handler registration.
Mirrors CRegManager from ExplorerLens.io.
"""

from __future__ import annotations

import ctypes
import logging
import winreg
from typing import Optional

logger = logging.getLogger("explorerlens.registry")


def is_admin() -> bool:
    """Check if running with admin privileges."""
    try:
        return ctypes.windll.shell32.IsUserAnAdmin() != 0
    except Exception:
        return False


def get_registered_extensions() -> dict[str, str]:
    """
    Scan HKLM\\Software\\Classes for extensions that have an
    IThumbnailProvider ShellEx handler pointing to our CLSID.
    """
    from .shell.com_server import COM_CLSID_PY

    result: dict[str, str] = {}

    try:
        classes_key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, "Software\\Classes")
        idx = 0
        while True:
            try:
                name = winreg.EnumKey(classes_key, idx)
                idx += 1
                if not name.startswith("."):
                    continue
                handler_path = (
                    f"Software\\Classes\\{name}\\ShellEx\\"
                    f"{{e357fccd-a995-4576-b01f-234630154e96}}"
                )
                try:
                    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, handler_path) as hk:
                        val, _ = winreg.QueryValueEx(hk, "")
                        if val.upper() == COM_CLSID_PY.upper():
                            result[name] = val
                except FileNotFoundError:
                    pass
            except OSError:
                break
        classes_key.Close()
    except Exception as exc:
        logger.debug("Registry scan error: %s", exc)

    return result


def set_thumbnail_cache_enabled(enabled: bool) -> bool:
    """Enable or disable Windows thumbnail caching via registry."""
    if not is_admin():
        return False
    try:
        path = "Software\\Microsoft\\Windows\\CurrentVersion\\" "Explorer\\Advanced"
        with winreg.OpenKey(winreg.HKEY_CURRENT_USER, path, 0, winreg.KEY_WRITE) as key:
            winreg.SetValueEx(
                key, "DisableThumbnailCache", 0, winreg.REG_DWORD, 0 if enabled else 1
            )
        return True
    except Exception as exc:
        logger.error("Failed to set thumbnail cache: %s", exc)
        return False


def flush_thumbnail_cache() -> bool:
    """Clear Windows thumbnail cache by deleting thumbcache files."""
    import glob
    import os

    cache_dir = os.path.expandvars(r"%LOCALAPPDATA%\Microsoft\Windows\Explorer")
    try:
        for f in glob.glob(os.path.join(cache_dir, "thumbcache_*.db")):
            try:
                os.remove(f)
            except PermissionError:
                pass  # In use by Explorer
        return True
    except Exception as exc:
        logger.error("Failed to flush thumbnail cache: %s", exc)
        return False


def restart_explorer() -> None:
    """Restart explorer.exe to reload shell extensions."""
    import subprocess

    subprocess.run(
        ["taskkill", "/f", "/im", "explorer.exe"],
        capture_output=True,
    )
    subprocess.Popen(["explorer.exe"])


def detect_conflicts() -> dict[str, str]:
    """
    Detect other IThumbnailProvider handlers registered for our extensions.
    Returns a dict of { extension: existing_clsid } for extensions that
    already have a handler that isn't ours.
    """
    from .config import ALL_EXTENSIONS
    from .shell.com_server import COM_CLSID_PY

    conflicts: dict[str, str] = {}
    for ext in sorted(ALL_EXTENSIONS):
        handler_path = (
            f"Software\\Classes\\{ext}\\ShellEx\\"
            f"{{e357fccd-a995-4576-b01f-234630154e96}}"
        )
        try:
            with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, handler_path) as hk:
                val, _ = winreg.QueryValueEx(hk, "")
                if val.upper() != COM_CLSID_PY.upper():
                    conflicts[ext] = val
        except FileNotFoundError:
            pass
    return conflicts


def backup_registrations(output_path: str) -> bool:
    """Export current extension registration state to a JSON file."""
    import json

    status = get_registered_extensions()
    data = {
        "extensions": status,
        "timestamp": __import__("datetime").datetime.now().isoformat(),
    }
    try:
        with open(output_path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2)
        return True
    except Exception as exc:
        logger.error("Backup failed: %s", exc)
        return False


def restore_registrations(input_path: str) -> bool:
    """Restore registrations from a backup JSON file."""
    import json

    if not is_admin():
        logger.error("Restore requires admin privileges")
        return False
    try:
        with open(input_path, "r", encoding="utf-8") as f:
            data = json.load(f)
        extensions = set(data.get("extensions", {}).keys())
        if not extensions:
            logger.warning("No extensions found in backup")
            return False
        from .shell.com_server import register

        return register(extensions)
    except Exception as exc:
        logger.error("Restore failed: %s", exc)
        return False
