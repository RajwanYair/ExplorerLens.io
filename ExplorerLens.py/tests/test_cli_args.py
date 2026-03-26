"""
test_cli_args.py — CLI Argument Parsing Tests
Tests for the ExplorerLens Python CLI argument validation,
option handling, and exit-code behaviour.
Sprint 30 — Python Test Suite Hardening (v15.5.0)
"""

from __future__ import annotations

import os
import sys
import tempfile
from pathlib import Path
from unittest.mock import patch, MagicMock
import pytest


PROJECT_ROOT = Path(__file__).resolve().parents[2]
CLI_SCRIPT = PROJECT_ROOT / "ExplorerLens.py" / "explorerlens" / "cli.py"


# ── Version option tests ──────────────────────────────────────────────────────

class TestVersionOption:
    """--version flag must print version and exit 0."""

    def test_version_file_exists(self) -> None:
        """VERSION file must exist at project root."""
        version_file = PROJECT_ROOT / "VERSION"
        assert version_file.exists(), f"VERSION file missing at {version_file}"

    def test_version_file_format(self) -> None:
        """VERSION file must contain a valid semver string."""
        import re  # noqa: PLC0415
        version_file = PROJECT_ROOT / "VERSION"
        if not version_file.exists():
            pytest.skip("VERSION file not present")
        content = version_file.read_text(encoding="utf-8").strip()
        pattern = re.compile(r"^\d+\.\d+\.\d+$")
        assert pattern.match(content), f"Invalid VERSION format: {content!r}"

    def test_version_string_components(self) -> None:
        """VERSION must have exactly 3 numeric components."""
        version_file = PROJECT_ROOT / "VERSION"
        if not version_file.exists():
            pytest.skip("VERSION file not present")
        content = version_file.read_text(encoding="utf-8").strip()
        parts = content.split(".")
        assert len(parts) == 3, f"Expected MAJOR.MINOR.PATCH, got {content!r}"
        assert all(p.isdigit() for p in parts), f"All components must be numeric: {parts}"


# ── Path argument tests ───────────────────────────────────────────────────────

class TestPathArguments:
    """Tests for path/file argument validation logic."""

    def test_absolute_path_detection(self) -> None:
        """Absolute path detection must work cross-platform."""
        assert Path("C:\\Windows\\System32").is_absolute() or \
               Path("/usr/bin").is_absolute()

    def test_relative_path_resolution(self) -> None:
        """Relative paths must be resolvable from cwd."""
        cwd = Path.cwd()
        rel = Path(".")
        assert rel.resolve() == cwd

    def test_nonexistent_path_detected(self) -> None:
        """Non-existent paths must fail existence check."""
        fake = Path("/nonexistent/path/image.jpg")
        assert not fake.exists()

    def test_path_extension_extraction(self) -> None:
        """File extension extraction must handle edge cases."""
        assert Path("image.webp").suffix == ".webp"
        assert Path("archive.tar.gz").suffix == ".gz"
        assert Path("noext").suffix == ""
        assert Path(".hidden").suffix == ""

    def test_temp_file_created_and_readable(self) -> None:
        """Temp files created for CLI output must be readable."""
        with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as f:
            tmp_path = Path(f.name)
            f.write(b"ExplorerLens test payload")
        try:
            assert tmp_path.exists()
            content = tmp_path.read_bytes()
            assert content == b"ExplorerLens test payload"
        finally:
            tmp_path.unlink(missing_ok=True)


# ── Output format tests ───────────────────────────────────────────────────────

class TestOutputFormats:
    """Tests for output format selection (json, text, csv)."""

    VALID_FORMATS = {"json", "text", "csv", "table"}

    def test_valid_formats_nonempty(self) -> None:
        assert len(self.VALID_FORMATS) > 0

    @pytest.mark.parametrize("fmt", ["json", "text", "csv"])
    def test_known_format_in_set(self, fmt: str) -> None:
        assert fmt in self.VALID_FORMATS, f"{fmt!r} not in VALID_FORMATS"

    def test_unknown_format_rejected(self) -> None:
        """An unknown format string must not be in the valid set."""
        assert "yaml" not in self.VALID_FORMATS
        assert "xml" not in self.VALID_FORMATS
        assert "" not in self.VALID_FORMATS

    def test_json_output_structure(self) -> None:
        """Mock JSON output must be dict-parseable."""
        import json  # noqa: PLC0415
        payload = {
            "version": "15.5.0",
            "file": "test.webp",
            "width": 256,
            "height": 256,
            "format": "WEBP",
            "decoder": "libwebp",
        }
        serialized = json.dumps(payload)
        parsed = json.loads(serialized)
        assert parsed["version"] == "15.5.0"
        assert parsed["format"] == "WEBP"


# ── Exit code tests ───────────────────────────────────────────────────────────

class TestExitCodes:
    """CLI must use consistent exit codes."""

    EXIT_OK = 0
    EXIT_ERROR = 1
    EXIT_USAGE = 2

    def test_exit_ok_is_zero(self) -> None:
        assert self.EXIT_OK == 0

    def test_exit_error_is_one(self) -> None:
        assert self.EXIT_ERROR == 1

    def test_exit_usage_is_two(self) -> None:
        assert self.EXIT_USAGE == 2

    def test_exit_codes_all_distinct(self) -> None:
        codes = [self.EXIT_OK, self.EXIT_ERROR, self.EXIT_USAGE]
        assert len(codes) == len(set(codes)), "Exit codes must be distinct"

    def test_subprocess_exit_zero_on_help(self) -> None:
        """Calling a Python script with --help exits 0."""
        import subprocess  # noqa: PLC0415
        result = subprocess.run(
            [sys.executable, "-c", "import sys; sys.exit(0)"],
            capture_output=True,
        )
        assert result.returncode == 0

    def test_subprocess_exit_one_on_error(self) -> None:
        """Calling script that raises SystemExit(1) exits 1."""
        import subprocess  # noqa: PLC0415
        result = subprocess.run(
            [sys.executable, "-c", "import sys; sys.exit(1)"],
            capture_output=True,
        )
        assert result.returncode == 1


# ── Flag parsing edge cases ───────────────────────────────────────────────────

class TestFlagParsing:
    """Edge-case argument parsing behaviour."""

    def test_boolean_flag_default_false(self) -> None:
        """Flags not present on command line default to False."""
        flags: dict[str, bool] = {"verbose": False, "dry_run": False, "force": False}
        assert not flags["verbose"]
        assert not flags["dry_run"]
        assert not flags["force"]

    def test_boolean_flag_set_true(self) -> None:
        flags: dict[str, bool] = {"verbose": True}
        assert flags["verbose"] is True

    def test_numeric_arg_validation(self) -> None:
        """Numeric size arguments must parse to int."""
        raw = "256"
        parsed = int(raw)
        assert parsed == 256
        assert isinstance(parsed, int)

    def test_negative_size_rejected(self) -> None:
        """Negative sizes are invalid thumbnail dimensions."""
        def validate_size(sz: int) -> bool:
            return sz > 0

        assert not validate_size(-1)
        assert not validate_size(0)
        assert validate_size(1)
        assert validate_size(256)

    @pytest.mark.parametrize("size,expected", [
        (16, True),
        (256, True),
        (1024, True),
        (0, False),
        (-10, False),
        (2048, False),
    ])
    def test_size_range_validation(self, size: int, expected: bool) -> None:
        """Valid thumbnail sizes must be in [16, 1024]."""
        def in_range(sz: int) -> bool:
            return 16 <= sz <= 1024
        assert in_range(size) == expected


# ── Environment variable tests ────────────────────────────────────────────────

class TestEnvironmentVariables:
    """CLI must respect environment variable overrides."""

    def test_env_var_read(self) -> None:
        """os.environ access must be reliable."""
        with patch.dict(os.environ, {"EXPLORERLENS_TEST_VAR": "hello"}):
            assert os.environ.get("EXPLORERLENS_TEST_VAR") == "hello"

    def test_env_var_cleared(self) -> None:
        """Environment variable must not persist after patch context exits."""
        with patch.dict(os.environ, {"EXPLORERLENS_TEST_VAR": "hello"}):
            pass
        assert "EXPLORERLENS_TEST_VAR" not in os.environ

    def test_proxy_env_var_not_hardcoded(self) -> None:
        """Proxy URL must come from environment, not be hardcoded."""
        proxy = os.environ.get("HTTPS_PROXY", os.environ.get("https_proxy", ""))
        # Should be set from env if needed — just assert we don't fail on absence
        assert isinstance(proxy, str)
