# ExplorerLens.py — Test Suite
# Copyright (c) 2026 ExplorerLens Project
"""
Unit tests mirroring ExplorerLens.io EngineTests.cpp patterns.
Run with: python -m pytest tests/ -v
"""

from __future__ import annotations

import io
import os
import tempfile
from pathlib import Path

import pytest
from PIL import Image

# ── Config Tests ─────────────────────────────────────────────────────


class TestConfig:
    def test_default_config(self):
        from explorerlens.config import Config
        cfg = Config()
        assert cfg.thumbnail_size == 256
        assert cfg.dark_mode == "auto"
        assert cfg.cache.enabled is True
        assert cfg.performance.max_workers == 4

    def test_all_categories_enabled_by_default(self):
        from explorerlens.config import Config, FORMAT_CATEGORIES
        cfg = Config()
        for cat in FORMAT_CATEGORIES:
            assert cfg.enabled_categories[cat] is True

    def test_get_enabled_extensions(self):
        from explorerlens.config import Config, FORMAT_CATEGORIES
        cfg = Config()
        exts = cfg.get_enabled_extensions()
        assert len(exts) > 100  # Should have 200+ extensions

    def test_save_load_roundtrip(self, tmp_path):
        from explorerlens.config import Config
        cfg = Config()
        cfg.thumbnail_size = 512
        cfg.dark_mode = "dark"
        cfg.enabled_categories["video"] = False

        config_file = tmp_path / "test_config.json"
        cfg.save(config_file)

        loaded = Config.load(config_file)
        assert loaded.thumbnail_size == 512
        assert loaded.dark_mode == "dark"
        assert loaded.enabled_categories["video"] is False
        assert loaded.enabled_categories["image"] is True

    def test_reset_defaults(self):
        from explorerlens.config import Config
        cfg = Config()
        cfg.thumbnail_size = 999
        cfg.reset_defaults()
        assert cfg.thumbnail_size == 256

    def test_format_categories_complete(self):
        """Verify all 46 requested extensions are in FORMAT_CATEGORIES."""
        from explorerlens.config import FORMAT_CATEGORIES, ALL_EXTENSIONS

        required = {
            ".avi", ".wmv", ".asf", ".mpg", ".mpeg", ".m1v", ".m2v",
            ".ts", ".m2ts", ".mts", ".m2t", ".mp4", ".m4v", ".mp4v",
            ".mov", ".3g2", ".3gp", ".3gp2", ".3gpp", ".mkv", ".mk3d",
            ".webm", ".flv", ".f4v", ".ogm", ".ogv", ".rm", ".rmvb",
            ".dv", ".mxf", ".ivf", ".evo", ".264", ".video",
            ".cbr", ".cbz", ".cb7",
            ".mp3", ".wav", ".m4a", ".ape", ".flac", ".ogg", ".mka",
            ".mpc", ".opus", ".tak", ".wv",
            ".webp",
        }
        for ext in required:
            assert ext in ALL_EXTENSIONS, f"Missing extension: {ext}"


# ── Engine Tests ─────────────────────────────────────────────────────


class TestEngine:
    def test_engine_init(self):
        from explorerlens.engine import ThumbnailEngine
        engine = ThumbnailEngine()
        exts = engine.get_supported_extensions()
        assert len(exts) > 50
        engine.shutdown()

    def test_decode_status_enum(self):
        from explorerlens.engine import DecodeStatus
        assert DecodeStatus.Success == 0
        assert DecodeStatus.FileNotFound.name == "FileNotFound"

    def test_generate_missing_file(self):
        from explorerlens.engine import ThumbnailEngine, ThumbnailRequest, DecodeStatus
        engine = ThumbnailEngine()
        req = ThumbnailRequest(path=Path("nonexistent.jpg"))
        result = engine.generate(req)
        assert result.status == DecodeStatus.FileNotFound
        engine.shutdown()

    def test_generate_png(self, tmp_path):
        from explorerlens.engine import ThumbnailEngine, ThumbnailRequest, DecodeStatus
        # Create a test PNG
        img = Image.new("RGB", (100, 100), (255, 0, 0))
        test_file = tmp_path / "test.png"
        img.save(test_file)

        engine = ThumbnailEngine()
        req = ThumbnailRequest(path=test_file, size=64)
        result = engine.generate(req)
        assert result.status == DecodeStatus.Success
        assert result.image is not None
        assert max(result.image.size) <= 64
        engine.shutdown()

    def test_generate_batch(self, tmp_path):
        from explorerlens.engine import ThumbnailEngine, ThumbnailRequest
        files = []
        for i in range(5):
            img = Image.new("RGB", (50, 50), (i * 50, 0, 0))
            f = tmp_path / f"test_{i}.png"
            img.save(f)
            files.append(ThumbnailRequest(path=f, size=32))

        engine = ThumbnailEngine()
        results = engine.generate_batch(files)
        assert len(results) == 5
        engine.shutdown()

    def test_engine_stats(self, tmp_path):
        from explorerlens.engine import ThumbnailEngine, ThumbnailRequest
        img = Image.new("RGB", (10, 10))
        f = tmp_path / "stats_test.png"
        img.save(f)

        engine = ThumbnailEngine()
        engine.generate(ThumbnailRequest(path=f))
        stats = engine.get_stats()
        assert stats.total >= 1
        engine.shutdown()

    def test_unsupported_format(self, tmp_path):
        from explorerlens.engine import ThumbnailEngine, ThumbnailRequest, DecodeStatus
        from explorerlens.config import Config
        cfg = Config()
        # Disable all categories
        for cat in cfg.enabled_categories:
            cfg.enabled_categories[cat] = False

        f = tmp_path / "test.xyz_unknown"
        f.write_text("dummy")

        engine = ThumbnailEngine(cfg)
        result = engine.generate(ThumbnailRequest(path=f))
        assert result.status == DecodeStatus.UnsupportedFormat
        engine.shutdown()


# ── Cache Tests ──────────────────────────────────────────────────────


class TestMemoryCache:
    def test_put_get(self):
        from explorerlens.cache import MemoryCache
        cache = MemoryCache()
        img = Image.new("RGB", (64, 64), (255, 0, 0))
        cache.put("test.png", 64, img)
        result = cache.get("test.png", 64)
        assert result is not None
        assert result.size == (64, 64)

    def test_cache_miss(self):
        from explorerlens.cache import MemoryCache
        cache = MemoryCache()
        assert cache.get("missing.png", 64) is None

    def test_different_sizes(self):
        from explorerlens.cache import MemoryCache
        cache = MemoryCache()
        img = Image.new("RGB", (64, 64))
        cache.put("test.png", 64, img)
        cache.put("test.png", 128, img)
        assert cache.get("test.png", 64) is not None
        assert cache.get("test.png", 128) is not None
        assert cache.get("test.png", 256) is None

    def test_eviction(self):
        from explorerlens.cache import MemoryCache
        cache = MemoryCache(max_items=2)
        for i in range(5):
            cache.put(f"file{i}.png", 64, Image.new("RGB", (10, 10)))
        assert cache.stats["items"] <= 2

    def test_invalidate(self):
        from explorerlens.cache import MemoryCache
        cache = MemoryCache()
        cache.put("test.png", 64, Image.new("RGB", (10, 10)))
        cache.put("test.png", 128, Image.new("RGB", (10, 10)))
        cache.invalidate("test.png")
        assert cache.get("test.png", 64) is None
        assert cache.get("test.png", 128) is None

    def test_stats(self):
        from explorerlens.cache import MemoryCache
        cache = MemoryCache()
        cache.put("test.png", 64, Image.new("RGB", (10, 10)))
        cache.get("test.png", 64)
        cache.get("missing.png", 64)
        stats = cache.stats
        assert stats["hits"] == 1
        assert stats["misses"] == 1
        assert stats["items"] == 1


class TestDiskCache:
    def test_put_get(self, tmp_path):
        from explorerlens.cache import DiskCache
        db = tmp_path / "test.db"
        cache = DiskCache(db_path=db)
        img = Image.new("RGB", (64, 64), (0, 255, 0))
        cache.put("test_file.png", 64, img)
        result = cache.get("test_file.png", 64)
        assert result is not None
        cache.close()

    def test_cache_miss(self, tmp_path):
        from explorerlens.cache import DiskCache
        db = tmp_path / "test.db"
        cache = DiskCache(db_path=db)
        assert cache.get("missing.png", 64) is None
        cache.close()

    def test_clear(self, tmp_path):
        from explorerlens.cache import DiskCache
        db = tmp_path / "test.db"
        cache = DiskCache(db_path=db)
        cache.put("a.png", 64, Image.new("RGB", (10, 10)))
        cache.put("b.png", 64, Image.new("RGB", (10, 10)))
        cache.clear()
        assert cache.stats["items"] == 0
        cache.close()


# ── Decoder Tests ────────────────────────────────────────────────────


class TestDecoders:
    def test_all_decoders_registered(self):
        from explorerlens.decoders import get_all_decoders
        decoders = get_all_decoders()
        assert len(decoders) >= 6  # image, archive, video, audio, doc, font

    def test_decoder_names(self):
        from explorerlens.decoders import get_all_decoders
        names = {d.name for d in get_all_decoders()}
        assert "ImageDecoder" in names
        assert "ArchiveDecoder" in names
        assert "VideoDecoder" in names
        assert "AudioDecoder" in names
        assert "DocumentDecoder" in names
        assert "FontDecoder" in names

    def test_image_decoder_png(self, tmp_path):
        from explorerlens.decoders.image_decoder import ImageDecoder
        decoder = ImageDecoder()
        img = Image.new("RGB", (200, 200), (0, 0, 255))
        f = tmp_path / "test.png"
        img.save(f)
        result = decoder.decode(f, 128)
        assert result is not None

    def test_image_decoder_bmp(self, tmp_path):
        from explorerlens.decoders.image_decoder import ImageDecoder
        decoder = ImageDecoder()
        img = Image.new("RGB", (100, 100))
        f = tmp_path / "test.bmp"
        img.save(f)
        result = decoder.decode(f, 64)
        assert result is not None

    def test_image_decoder_webp(self, tmp_path):
        from explorerlens.decoders.image_decoder import ImageDecoder
        decoder = ImageDecoder()
        img = Image.new("RGB", (100, 100), (128, 128, 0))
        f = tmp_path / "test.webp"
        img.save(f, "WEBP")
        result = decoder.decode(f, 64)
        assert result is not None

    def test_archive_decoder_zip(self, tmp_path):
        from explorerlens.decoders.archive_decoder import ArchiveDecoder
        import zipfile
        decoder = ArchiveDecoder()

        # Create a ZIP with an image
        img = Image.new("RGB", (50, 50), (255, 128, 0))
        img_buf = io.BytesIO()
        img.save(img_buf, "PNG")
        img_data = img_buf.getvalue()

        zip_path = tmp_path / "test.zip"
        with zipfile.ZipFile(zip_path, "w") as zf:
            zf.writestr("image.png", img_data)

        result = decoder.decode(zip_path, 128)
        assert result is not None

    def test_archive_decoder_cbz(self, tmp_path):
        from explorerlens.decoders.archive_decoder import ArchiveDecoder
        import zipfile
        decoder = ArchiveDecoder()

        img = Image.new("RGB", (100, 150), (0, 128, 255))
        img_buf = io.BytesIO()
        img.save(img_buf, "JPEG")

        cbz_path = tmp_path / "comic.cbz"
        with zipfile.ZipFile(cbz_path, "w") as zf:
            zf.writestr("page001.jpg", img_buf.getvalue())

        result = decoder.decode(cbz_path, 256)
        assert result is not None

    def test_audio_decoder_placeholder(self, tmp_path):
        from explorerlens.decoders.audio_decoder import AudioDecoder
        decoder = AudioDecoder()

        # Create a dummy audio file (no actual audio data)
        f = tmp_path / "test.mp3"
        f.write_bytes(b"\x00" * 100)

        # Should fall through to placeholder
        result = decoder.decode(f, 128)
        assert result is not None  # Placeholder always generated

    def test_font_decoder_extensions(self):
        from explorerlens.decoders.font_decoder import FontDecoder
        decoder = FontDecoder()
        exts = decoder.supported_extensions()
        assert ".ttf" in exts
        assert ".otf" in exts
        assert ".woff" in exts

    def test_document_decoder_extensions(self):
        from explorerlens.decoders.document_decoder import DocumentDecoder
        decoder = DocumentDecoder()
        exts = decoder.supported_extensions()
        assert ".pdf" in exts
        assert ".docx" in exts
        assert ".pptx" in exts


# ── Plugin Tests ─────────────────────────────────────────────────────


class TestPluginLoader:
    def test_empty_dir(self, tmp_path):
        from explorerlens.plugins import PluginLoader
        loader = PluginLoader(tmp_path)
        decoders = loader.discover()
        assert len(decoders) == 0

    def test_missing_dir(self):
        from explorerlens.plugins import PluginLoader
        loader = PluginLoader(Path("nonexistent_plugin_dir"))
        decoders = loader.discover()
        assert len(decoders) == 0

    def test_valid_plugin(self, tmp_path):
        from explorerlens.plugins import PluginLoader

        # Create a minimal plugin
        plugin_code = '''
from explorerlens.decoders.base import BaseDecoder
from pathlib import Path
from typing import Optional
from PIL import Image

class TestPlugin(BaseDecoder):
    @property
    def name(self): return "TestPlugin"
    def supported_extensions(self): return [".test_ext"]
    def decode(self, path, size): return Image.new("RGB", (10, 10))

def create_decoder():
    return TestPlugin()
'''
        (tmp_path / "test_plugin.py").write_text(plugin_code)

        loader = PluginLoader(tmp_path)
        decoders = loader.discover()
        assert len(decoders) == 1
        assert decoders[0].name == "TestPlugin"


# ── Elevation Tests ──────────────────────────────────────────────────


class TestElevation:
    def test_is_admin_returns_bool(self):
        from explorerlens.utils.elevation import is_admin
        result = is_admin()
        assert isinstance(result, bool)


# ── Integration Tests ────────────────────────────────────────────────


class TestIntegration:
    def test_full_pipeline_png(self, tmp_path):
        """End-to-end: config → engine → decode → cache → result."""
        from explorerlens.config import Config
        from explorerlens.engine import ThumbnailEngine, ThumbnailRequest, DecodeStatus
        from explorerlens.cache import MemoryCache

        cfg = Config()
        engine = ThumbnailEngine(cfg)
        cache = MemoryCache()
        engine.set_cache(cache)

        # Create test image
        img = Image.new("RGBA", (300, 200), (255, 128, 0, 255))
        test_file = tmp_path / "integration.png"
        img.save(test_file)

        # First decode — should miss cache
        result = engine.generate(ThumbnailRequest(path=test_file, size=128))
        assert result.status == DecodeStatus.Success
        assert result.image is not None
        assert result.decoder_name == "ImageDecoder"

        # Second decode — should hit cache
        result2 = engine.generate(ThumbnailRequest(path=test_file, size=128))
        assert result2.status == DecodeStatus.Success
        assert result2.decoder_name == "cache"

        assert cache.stats["hits"] == 1
        engine.shutdown()

    def test_full_pipeline_webp(self, tmp_path):
        """End-to-end for WebP format."""
        from explorerlens.config import Config
        from explorerlens.engine import ThumbnailEngine, ThumbnailRequest, DecodeStatus

        cfg = Config()
        engine = ThumbnailEngine(cfg)

        img = Image.new("RGB", (200, 200), (0, 255, 0))
        test_file = tmp_path / "test.webp"
        img.save(test_file, "WEBP")

        result = engine.generate(ThumbnailRequest(path=test_file, size=64))
        assert result.status == DecodeStatus.Success
        engine.shutdown()

    def test_full_pipeline_zip_archive(self, tmp_path):
        """End-to-end for ZIP archive with embedded images."""
        import zipfile
        from explorerlens.config import Config
        from explorerlens.engine import ThumbnailEngine, ThumbnailRequest, DecodeStatus

        cfg = Config()
        engine = ThumbnailEngine(cfg)

        # Create ZIP with images
        img = Image.new("RGB", (80, 80), (255, 0, 255))
        buf = io.BytesIO()
        img.save(buf, "PNG")
        zip_path = tmp_path / "archive.zip"
        with zipfile.ZipFile(zip_path, "w") as zf:
            zf.writestr("photo.png", buf.getvalue())

        result = engine.generate(ThumbnailRequest(path=zip_path, size=128))
        assert result.status == DecodeStatus.Success
        engine.shutdown()
