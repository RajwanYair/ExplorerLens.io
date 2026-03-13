# ExplorerLens.py — Shell Platform Tests
# Copyright (c) 2026 ExplorerLens Project
"""
Tests for cross-platform shell integration modules.
Run with: python -m pytest tests/test_shell.py -v
"""

from __future__ import annotations

import sys
from pathlib import Path
from unittest.mock import patch

import pytest

# ── Platform Detection Tests ─────────────────────────────────────────


class TestPlatformProvider:
    def test_get_platform_returns_string(self):
        from explorerlens.shell.platform_provider import get_platform

        result = get_platform()
        assert result in ("windows", "linux", "macos", "unknown")

    def test_platform_constants(self):
        from explorerlens.shell.platform_provider import Platform

        assert Platform.WINDOWS == "windows"
        assert Platform.LINUX == "linux"
        assert Platform.MACOS == "macos"
        assert Platform.UNKNOWN == "unknown"

    @patch("sys.platform", "win32")
    def test_detect_windows(self):
        from explorerlens.shell.platform_provider import get_platform

        assert get_platform() == "windows"

    @patch("sys.platform", "linux")
    def test_detect_linux(self):
        from explorerlens.shell.platform_provider import get_platform

        assert get_platform() == "linux"

    @patch("sys.platform", "darwin")
    def test_detect_macos(self):
        from explorerlens.shell.platform_provider import get_platform

        assert get_platform() == "macos"

    @patch("sys.platform", "freebsd12")
    def test_detect_unknown(self):
        from explorerlens.shell.platform_provider import get_platform

        assert get_platform() == "unknown"

    def test_provider_status_has_platform(self):
        from explorerlens.shell.platform_provider import get_provider_status

        status = get_provider_status()
        assert "platform" in status
        assert isinstance(status["platform"], str)


# ── macOS Quick Look Tests ───────────────────────────────────────────


class TestMacOSQuickLook:
    def test_supported_utis_non_empty(self):
        from explorerlens.shell.macos_quicklook import _get_supported_utis

        utis = _get_supported_utis()
        assert len(utis) > 30
        assert "public.png" in utis
        assert "com.adobe.pdf" in utis

    def test_info_plist_structure(self):
        from explorerlens.shell.macos_quicklook import _generate_info_plist

        plist = _generate_info_plist()
        assert plist["CFBundleIdentifier"] == "io.explorerlens.quicklook"
        assert plist["CFBundlePackageType"] == "BNDL"
        assert plist["QLSupportsConcurrentRequests"] is True
        assert isinstance(plist["CFBundleDocumentTypes"], list)
        assert len(plist["CFBundleDocumentTypes"]) == 1
        doc_type = plist["CFBundleDocumentTypes"][0]
        assert "LSItemContentTypes" in doc_type

    def test_quicklook_status(self):
        from explorerlens.shell.macos_quicklook import get_quicklook_status

        status = get_quicklook_status()
        assert "supported_utis" in status
        assert status["supported_utis"] > 30
        assert "user_installed" in status
        assert "system_installed" in status

    @pytest.mark.skipif(sys.platform != "darwin", reason="macOS only")
    def test_install_uninstall_user(self, tmp_path):
        from explorerlens.shell.macos_quicklook import (
            install_quicklook,
            uninstall_quicklook,
        )

        with patch("explorerlens.shell.macos_quicklook._USER_QL_DIR", tmp_path):
            result = install_quicklook(system_wide=False)
            assert result is not None

            bundle = tmp_path / "ExplorerLens.qlgenerator"
            assert bundle.exists()
            assert (bundle / "Contents" / "Info.plist").exists()
            assert (bundle / "Contents" / "MacOS" / "explorerlens-ql").exists()

            assert uninstall_quicklook(system_wide=False)
            assert not bundle.exists()


# ── Linux Thumbnailer Tests ──────────────────────────────────────────


class TestLinuxThumbnailer:
    def test_ext_to_mimetypes(self):
        from explorerlens.shell.linux_thumbnailer import _extensions_to_mimetypes

        mimes = _extensions_to_mimetypes()
        assert "image/png" in mimes
        assert "image/webp" in mimes

    def test_thumbnailer_file_content(self):
        from explorerlens.shell.linux_thumbnailer import generate_thumbnailer_file

        content = generate_thumbnailer_file()
        assert "[Thumbnailer Entry]" in content
        assert "explorerlens-thumb" in content
        assert "MimeType=" in content

    @pytest.mark.skipif(sys.platform != "linux", reason="Linux only")
    def test_get_thumb_path_returns_path(self):
        from explorerlens.shell.linux_thumbnailer import get_thumb_path

        uri = "file:///tmp/test.png"
        result = get_thumb_path(uri, 256)
        assert result is not None
        assert str(result).endswith(".png")


# ── Package Exports ──────────────────────────────────────────────────


class TestShellExports:
    def test_package_exports(self):
        from explorerlens.shell import (
            get_platform,
            get_provider_status,
            register_provider,
            unregister_provider,
        )

        assert callable(get_platform)
        assert callable(register_provider)
        assert callable(unregister_provider)
        assert callable(get_provider_status)
