# ExplorerLens.py — COM Shell Extension Server
# Copyright (c) 2026 ExplorerLens Project
"""
Implements IThumbnailProvider as a COM in-process server using comtypes.
Registers file extension handlers in the Windows registry so Explorer
calls our Python code to generate thumbnails.

Requires admin privileges for registration.
"""

from __future__ import annotations

import ctypes
import logging
import os
import sys
import winreg
from pathlib import Path
from typing import Optional

logger = logging.getLogger("explorerlens.shell")

# Python-specific CLSID — separate from C++ version to allow coexistence
COM_CLSID_PY = "{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}"
COM_PROGID = "ExplorerLens.PythonThumbnailProvider"
COM_DESCRIPTION = "ExplorerLens.py Thumbnail Provider"


def is_admin() -> bool:
    """Check if running with admin privileges."""
    try:
        return ctypes.windll.shell32.IsUserAnAdmin() != 0
    except Exception:
        return False


def register(extensions: set[str] | None = None) -> bool:
    """
    Register ExplorerLens.py as a thumbnail provider for the given
    extensions. Requires admin privileges.

    Returns True on success, False on failure.
    """
    if not is_admin():
        logger.error("Registration requires admin privileges")
        return False

    if extensions is None:
        from ..config import ALL_EXTENSIONS
        extensions = ALL_EXTENSIONS

    try:
        # Register the COM class
        _register_com_class()

        # Register each file extension
        for ext in sorted(extensions):
            _register_extension(ext)

        logger.info("Registered %d extensions", len(extensions))
        return True
    except Exception as exc:
        logger.error("Registration failed: %s", exc)
        return False


def unregister(extensions: set[str] | None = None) -> bool:
    """
    Remove ExplorerLens.py thumbnail provider registrations.
    Requires admin privileges.
    """
    if not is_admin():
        logger.error("Unregistration requires admin privileges")
        return False

    if extensions is None:
        from ..config import ALL_EXTENSIONS
        extensions = ALL_EXTENSIONS

    try:
        for ext in sorted(extensions):
            _unregister_extension(ext)

        _unregister_com_class()
        logger.info("Unregistered %d extensions", len(extensions))
        return True
    except Exception as exc:
        logger.error("Unregistration failed: %s", exc)
        return False


def get_registration_status() -> dict[str, bool]:
    """Check which extensions are currently registered."""
    from ..config import ALL_EXTENSIONS

    status: dict[str, bool] = {}
    for ext in sorted(ALL_EXTENSIONS):
        try:
            key_path = f"Software\\Classes\\{ext}\\ShellEx\\{{e357fccd-a995-4576-b01f-234630154e96}}"
            with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, key_path):
                status[ext] = True
        except FileNotFoundError:
            status[ext] = False
    return status


# ── Internal registration helpers ─────────────────────────────────


def _register_com_class() -> None:
    """Register the COM class in HKCR\\CLSID."""
    clsid_path = f"Software\\Classes\\CLSID\\{COM_CLSID_PY}"

    with winreg.CreateKeyEx(winreg.HKEY_LOCAL_MACHINE, clsid_path,
                            0, winreg.KEY_WRITE) as key:
        winreg.SetValueEx(key, "", 0, winreg.REG_SZ, COM_DESCRIPTION)

    # Point to Python script as LocalServer32
    python_exe = sys.executable
    server_cmd = f'"{python_exe}" -m explorerlens --com-server'
    server_path = f"{clsid_path}\\LocalServer32"

    with winreg.CreateKeyEx(winreg.HKEY_LOCAL_MACHINE, server_path,
                            0, winreg.KEY_WRITE) as key:
        winreg.SetValueEx(key, "", 0, winreg.REG_SZ, server_cmd)

    # Register ProgID
    progid_path = f"Software\\Classes\\{COM_PROGID}"
    with winreg.CreateKeyEx(winreg.HKEY_LOCAL_MACHINE, progid_path,
                            0, winreg.KEY_WRITE) as key:
        winreg.SetValueEx(key, "", 0, winreg.REG_SZ, COM_DESCRIPTION)

    clsid_ref = f"{progid_path}\\CLSID"
    with winreg.CreateKeyEx(winreg.HKEY_LOCAL_MACHINE, clsid_ref,
                            0, winreg.KEY_WRITE) as key:
        winreg.SetValueEx(key, "", 0, winreg.REG_SZ, COM_CLSID_PY)


def _unregister_com_class() -> None:
    """Remove COM class registration."""
    try:
        _delete_key_tree(
            winreg.HKEY_LOCAL_MACHINE,
            f"Software\\Classes\\CLSID\\{COM_CLSID_PY}"
        )
        _delete_key_tree(
            winreg.HKEY_LOCAL_MACHINE,
            f"Software\\Classes\\{COM_PROGID}"
        )
    except FileNotFoundError:
        pass


def _register_extension(ext: str) -> None:
    """Register thumbnail handler for a single extension."""
    # IThumbnailProvider GUID: {e357fccd-a995-4576-b01f-234630154e96}
    key_path = (f"Software\\Classes\\{ext}\\ShellEx\\"
                f"{{e357fccd-a995-4576-b01f-234630154e96}}")
    with winreg.CreateKeyEx(winreg.HKEY_LOCAL_MACHINE, key_path,
                            0, winreg.KEY_WRITE) as key:
        winreg.SetValueEx(key, "", 0, winreg.REG_SZ, COM_CLSID_PY)


def _unregister_extension(ext: str) -> None:
    """Remove thumbnail handler for a single extension."""
    key_path = (f"Software\\Classes\\{ext}\\ShellEx\\"
                f"{{e357fccd-a995-4576-b01f-234630154e96}}")
    try:
        _delete_key_tree(winreg.HKEY_LOCAL_MACHINE, key_path)
    except FileNotFoundError:
        pass


def _delete_key_tree(root: int, path: str) -> None:
    """Recursively delete a registry key and all subkeys."""
    try:
        with winreg.OpenKey(root, path, 0,
                            winreg.KEY_ALL_ACCESS) as key:
            # Enumerate and delete subkeys first
            while True:
                try:
                    subkey_name = winreg.EnumKey(key, 0)
                    _delete_key_tree(root, f"{path}\\{subkey_name}")
                except OSError:
                    break
        winreg.DeleteKey(root, path)
    except FileNotFoundError:
        pass
