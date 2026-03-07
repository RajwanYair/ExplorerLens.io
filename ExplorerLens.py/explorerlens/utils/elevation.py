# ExplorerLens.py — Elevation Utility
# Copyright (c) 2026 ExplorerLens Project
"""UAC elevation helpers for Windows admin operations."""

from __future__ import annotations

import ctypes
import sys


def is_admin() -> bool:
    """Check if the current process has admin privileges."""
    try:
        return ctypes.windll.shell32.IsUserAnAdmin() != 0
    except Exception:
        return False


def elevate_self() -> bool:
    """
    Re-launch the current script with admin privileges via UAC prompt.
    Returns True if elevation was initiated (current process should exit).
    Returns False if elevation failed or was cancelled.
    """
    if is_admin():
        return False  # Already admin

    try:
        # Use ShellExecuteW with "runas" verb to trigger UAC prompt
        ret = ctypes.windll.shell32.ShellExecuteW(
            None,                   # hwnd
            "runas",                # lpOperation
            sys.executable,         # lpFile
            " ".join(f'"{a}"' for a in sys.argv),  # lpParameters
            None,                   # lpDirectory
            1,                      # nShowCmd (SW_SHOWNORMAL)
        )
        # ShellExecuteW returns >32 on success
        return ret > 32
    except Exception:
        return False
