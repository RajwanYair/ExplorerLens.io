# ExplorerLens.py — Utils Package
# Copyright (c) 2026 ExplorerLens Project
"""Utility functions: benchmarking, elevation, logging."""

from .benchmark import run_benchmark
from .elevation import elevate_self, is_admin

__all__ = ["run_benchmark", "elevate_self", "is_admin"]
