# ExplorerLens.py — Benchmark Utility
# Copyright (c) 2026 ExplorerLens Project
"""
Benchmarks thumbnail generation across multiple files and formats.
Mirrors the performance testing from ExplorerLens.io Engine/Tests/.
"""

from __future__ import annotations

import time
from pathlib import Path
from typing import Sequence

from ..config import Config
from ..engine import DecodeStatus, ThumbnailEngine, ThumbnailRequest


def run_benchmark(
    files: Sequence[str | Path],
    size: int = 256,
    iterations: int = 1,
    config: Config | None = None,
) -> dict:
    """
    Run a thumbnail generation benchmark.

    Returns:
        Dictionary with timing results and per-format breakdown.
    """
    cfg = config or Config()
    engine = ThumbnailEngine(cfg)

    results: dict = {
        "files": len(files),
        "iterations": iterations,
        "size": size,
        "per_file": [],
        "per_format": {},
        "total_ms": 0.0,
        "succeeded": 0,
        "failed": 0,
    }

    total_start = time.perf_counter()

    for _iteration in range(iterations):
        for f in files:
            path = Path(f)
            req = ThumbnailRequest(path=path, size=size)
            result = engine.generate(req)

            ext = path.suffix.lower()
            entry = {
                "file": path.name,
                "ext": ext,
                "status": result.status.name,
                "elapsed_ms": result.elapsed_ms,
                "decoder": result.decoder_name,
            }
            results["per_file"].append(entry)

            if ext not in results["per_format"]:
                results["per_format"][ext] = {
                    "count": 0, "total_ms": 0.0,
                    "min_ms": float("inf"), "max_ms": 0.0,
                }
            fmt = results["per_format"][ext]
            fmt["count"] += 1
            fmt["total_ms"] += result.elapsed_ms
            fmt["min_ms"] = min(fmt["min_ms"], result.elapsed_ms)
            fmt["max_ms"] = max(fmt["max_ms"], result.elapsed_ms)

            if result.status == DecodeStatus.Success:
                results["succeeded"] += 1
            else:
                results["failed"] += 1

    results["total_ms"] = (time.perf_counter() - total_start) * 1000

    # Compute averages
    for fmt in results["per_format"].values():
        if fmt["count"] > 0:
            fmt["avg_ms"] = fmt["total_ms"] / fmt["count"]

    total = results["succeeded"] + results["failed"]
    if total > 0:
        results["avg_ms"] = results["total_ms"] / total
        results["images_per_sec"] = (
            results["succeeded"] / (results["total_ms"] / 1000)
            if results["total_ms"] > 0 else 0.0
        )

    engine.shutdown()
    return results


def print_benchmark_report(results: dict) -> None:
    """Print a formatted benchmark report."""
    print(f"\n{'='*60}")
    print(f"  ExplorerLens.py Benchmark Report")
    print(f"{'='*60}")
    print(f"  Files:       {results['files']}")
    print(f"  Iterations:  {results['iterations']}")
    print(f"  Thumb size:  {results['size']}px")
    print(f"  Succeeded:   {results['succeeded']}")
    print(f"  Failed:      {results['failed']}")
    print(f"  Total time:  {results['total_ms']:.1f}ms")

    if "avg_ms" in results:
        print(f"  Avg/image:   {results['avg_ms']:.1f}ms")
    if "images_per_sec" in results:
        print(f"  Throughput:  {results['images_per_sec']:.1f} img/sec")

    if results["per_format"]:
        print(f"\n  Per-format breakdown:")
        print(f"  {'Format':<10} {'Count':>6} {'Avg(ms)':>10} "
              f"{'Min(ms)':>10} {'Max(ms)':>10}")
        print(f"  {'-'*46}")
        for ext, fmt in sorted(results["per_format"].items()):
            avg = fmt.get("avg_ms", 0)
            print(f"  {ext:<10} {fmt['count']:>6} {avg:>10.1f} "
                  f"{fmt['min_ms']:>10.1f} {fmt['max_ms']:>10.1f}")

    print(f"{'='*60}\n")
