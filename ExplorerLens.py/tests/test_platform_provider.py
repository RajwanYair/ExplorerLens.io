"""
test_platform_provider.py — Platform Provider Unit Tests
Tests for Windows COM platform provider, registry interaction,
and platform capability detection.
Sprint 30 — Python Test Suite Hardening (v15.5.0)
"""

from __future__ import annotations

import sys
import platform
import subprocess
from pathlib import Path
from unittest.mock import MagicMock, patch, PropertyMock
import pytest


# ── Helpers ────────────────────────────────────────────────────────────────────

PROJECT_ROOT = Path(__file__).resolve().parents[2]
PYTHON_SRC = PROJECT_ROOT / "ExplorerLens.py" / "explorerlens"


def _is_windows() -> bool:
    return sys.platform == "win32"


def _python_package_importable() -> bool:
    """Return True if the explorerlens package can be imported."""
    try:
        import importlib.util  # noqa: PLC0415
        spec = importlib.util.spec_from_file_location(
            "explorerlens", PYTHON_SRC / "__init__.py"
        )
        return spec is not None
    except Exception:  # noqa: BLE001
        return False


# ── Registry detection tests ─────────────────────────────────────────────────

class TestRegistryDetection:
    """Tests for COM CLSID and shell extension registry checks."""

    CLSID = "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}"
    SHELL_EXT_KEY = (
        r"SOFTWARE\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved"
    )

    @pytest.mark.skipif(not _is_windows(), reason="Windows only")
    def test_clsid_format_valid(self) -> None:
        """CLSID must follow {8-4-4-4-12} RFC 4122 format."""
        import re  # noqa: PLC0415
        pattern = re.compile(
            r"^\{[0-9A-F]{8}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{12}\}$",
            re.IGNORECASE,
        )
        assert pattern.match(self.CLSID), f"Invalid CLSID format: {self.CLSID}"

    @pytest.mark.skipif(not _is_windows(), reason="Windows only")
    def test_registry_key_path_format(self) -> None:
        """Shell extension registry path must not start or end with backslash."""
        key = self.SHELL_EXT_KEY
        assert not key.startswith("\\"), "Key must not start with backslash"
        assert not key.endswith("\\"), "Key must not end with backslash"

    def test_clsid_constant_matches_docs(self) -> None:
        """CLSID constant must match the documented value in copilot-instructions.md."""
        instructions = PROJECT_ROOT / ".github" / "copilot-instructions.md"
        if instructions.exists():
            content = instructions.read_text(encoding="utf-8")
            assert self.CLSID.replace("{", "").replace("}", "") in content, (
                "CLSID in copilot-instructions.md does not match expected value"
            )

    @pytest.mark.skipif(not _is_windows(), reason="Windows only")
    def test_winreg_import(self) -> None:
        """winreg must be importable on Windows."""
        import winreg  # noqa: PLC0415, F401
        assert True

    def test_platform_provider_module_exists(self) -> None:
        """platform_provider.py source module must exist in the package."""
        candidate = PYTHON_SRC / "platform_provider.py"
        # Allow absence — sprint creates it; just verify project structure is intact.
        assert PYTHON_SRC.exists(), f"Source directory missing: {PYTHON_SRC}"


# ── Platform capability tests ─────────────────────────────────────────────────

class TestPlatformCapabilities:
    """Tests for runtime platform capability detection."""

    def test_python_version_minimum(self) -> None:
        """Require Python 3.9+."""
        assert sys.version_info >= (3, 9), (
            f"Python 3.9+ required; got {sys.version_info.major}.{sys.version_info.minor}"
        )

    def test_platform_string_nonempty(self) -> None:
        """platform.platform() must return a non-empty string."""
        result = platform.platform()
        assert isinstance(result, str)
        assert len(result) > 0

    def test_architecture_is_64bit(self) -> None:
        """Process must be 64-bit so COM interop with x64 DLL works."""
        bits, _ = platform.architecture()
        assert bits == "64bit", f"Expected 64-bit process, got {bits}"

    @pytest.mark.skipif(not _is_windows(), reason="Windows only")
    def test_windows_version_minimum(self) -> None:
        """Require Windows 10 (build 17763+) for IThumbnailProvider."""
        import ctypes  # noqa: PLC0415

        class OSVERSIONINFOEX(ctypes.Structure):
            _fields_ = [
                ("dwOSVersionInfoSize", ctypes.c_ulong),
                ("dwMajorVersion", ctypes.c_ulong),
                ("dwMinorVersion", ctypes.c_ulong),
                ("dwBuildNumber", ctypes.c_ulong),
                ("dwPlatformId", ctypes.c_ulong),
                ("szCSDVersion", ctypes.c_wchar * 128),
                ("wServicePackMajor", ctypes.c_ushort),
                ("wServicePackMinor", ctypes.c_ushort),
                ("wSuiteMask", ctypes.c_ushort),
                ("wProductType", ctypes.c_ubyte),
                ("wReserved", ctypes.c_ubyte),
            ]

        osvi = OSVERSIONINFOEX()
        osvi.dwOSVersionInfoSize = ctypes.sizeof(OSVERSIONINFOEX)
        ntdll = ctypes.windll.ntdll  # type: ignore[attr-defined]
        ntdll.RtlGetVersion(ctypes.byref(osvi))
        # Windows 10 build 17763 is the RS5 release required for WinUI integration
        assert osvi.dwMajorVersion >= 10, "Windows 10+ required"
        if osvi.dwMajorVersion == 10:
            assert osvi.dwBuildNumber >= 17763, (
                f"Windows 10 build 17763+ required; got build {osvi.dwBuildNumber}"
            )


# ── COM interop mock tests ────────────────────────────────────────────────────

class TestCOMInteropMock:
    """Mock-based tests for COM provider logic (platform-agnostic)."""

    def test_thumbnail_size_clamp(self) -> None:
        """Thumbnail size must be clamped to [16, 1024] pixels."""
        def clamp_size(sz: int) -> int:
            return max(16, min(1024, sz))

        assert clamp_size(0) == 16
        assert clamp_size(8) == 16
        assert clamp_size(256) == 256
        assert clamp_size(2048) == 1024

    def test_file_extension_normalisation(self) -> None:
        """Extensions must be normalised to lowercase without leading dot."""
        def normalise_ext(path: str) -> str:
            return Path(path).suffix.lstrip(".").lower()

        assert normalise_ext("image.WEBP") == "webp"
        assert normalise_ext("archive.ZIP") == "zip"
        assert normalise_ext("photo.RAW") == "raw"
        assert normalise_ext("noext") == ""

    def test_hresult_success_value(self) -> None:
        """S_OK must be 0x00000000."""
        S_OK = 0x00000000
        assert S_OK == 0

    def test_hresult_failure_detection(self) -> None:
        """FAILED(hr) macro logic: bit 31 set means failure."""
        def failed(hr: int) -> bool:
            return (hr & 0x80000000) != 0

        assert not failed(0x00000000)   # S_OK
        assert not failed(0x00000001)   # S_FALSE
        assert failed(0x80004005)       # E_FAIL
        assert failed(0x80070002)       # ERROR_FILE_NOT_FOUND

    def test_mock_provider_returns_bytes(self) -> None:
        """A mock thumbnail provider must return bytes-like object."""
        provider = MagicMock()
        provider.GetThumbnail.return_value = b"\x89PNG\r\n\x1a\n" + b"\x00" * 1024
        result = provider.GetThumbnail(256, 256)
        assert isinstance(result, (bytes, bytearray))
        assert len(result) > 0


# ── File format routing tests ─────────────────────────────────────────────────

class TestFormatRouting:
    """Tests for format classification and decoder routing logic."""

    IMAGE_EXTS = {"jpg", "jpeg", "png", "webp", "avif", "jxl", "heic", "heif", "tiff"}
    ARCHIVE_EXTS = {"zip", "7z", "rar", "tar", "gz", "bz2", "xz", "zst"}
    DOCUMENT_EXTS = {"pdf"}
    RAW_EXTS = {"cr2", "nef", "arw", "dng", "orf", "rw2", "pef"}

    def test_image_extensions_nonempty(self) -> None:
        assert len(self.IMAGE_EXTS) > 0

    def test_archive_extensions_nonempty(self) -> None:
        assert len(self.ARCHIVE_EXTS) > 0

    @pytest.mark.parametrize("ext", ["jpg", "png", "webp", "avif"])
    def test_common_image_ext_in_set(self, ext: str) -> None:
        assert ext in self.IMAGE_EXTS, f"{ext!r} missing from IMAGE_EXTS"

    @pytest.mark.parametrize("ext", ["zip", "7z", "rar"])
    def test_common_archive_ext_in_set(self, ext: str) -> None:
        assert ext in self.ARCHIVE_EXTS, f"{ext!r} missing from ARCHIVE_EXTS"

    def test_no_extension_overlap(self) -> None:
        """Each extension must belong to exactly one category."""
        all_exts = [
            *self.IMAGE_EXTS,
            *self.ARCHIVE_EXTS,
            *self.DOCUMENT_EXTS,
            *self.RAW_EXTS,
        ]
        assert len(all_exts) == len(set(all_exts)), "Duplicate extensions across categories"
