#!/usr/bin/env python3
"""Detect and remove duplicate #include lines from EngineTests.cpp.

Usage:
  python fix_duplicates.py           # report + fix duplicates
  python fix_duplicates.py --dry-run # report only, no writes
"""
import sys
from pathlib import Path
from collections import Counter

ROOT = Path(__file__).parent.parent.parent
fp = ROOT / "Engine" / "Tests" / "EngineTests.cpp"

lines = fp.read_text(encoding="utf-8").splitlines()
print(f"Loaded: {len(lines)} lines")

# Dynamically detect duplicate #include lines; keep first occurrence, drop extras
seen: set[str] = set()
remove: set[int] = set()
for i, line in enumerate(lines):
    if line.startswith("#include"):
        if line in seen:
            remove.add(i)
        else:
            seen.add(line)

if not remove:
    print("No duplicate includes found.")
    sys.exit(0)

print(f"Found {len(remove)} duplicate include lines:")
for i in sorted(remove):
    print(f"  Line {i+1}: {lines[i]}")

result = [line for i, line in enumerate(lines) if i not in remove]

# Quick stats
test_defs = sum(1 for l in result if l.startswith("TEST("))
run_tests = sum(1 for l in result if l.lstrip().startswith("RUN_TEST("))
inc_counts = Counter(l for l in result if l.startswith("#include"))
dup_incs = {k: v for k, v in inc_counts.items() if v > 1}
print(f"Result: {len(result)} lines | TEST(): {test_defs} | RUN_TEST(): {run_tests} | Remaining dup includes: {len(dup_incs)}")

if "--dry-run" in sys.argv:
    print("[DryRun] Not writing.")
    sys.exit(0)

fp.write_text("\n".join(result) + "\n", encoding="utf-8")
print("Written OK")
