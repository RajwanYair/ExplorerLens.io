#!/usr/bin/env python3
"""Remove duplicate includes and dead entries from EngineTests.cpp."""
import sys
from pathlib import Path

ROOT = Path(__file__).parent.parent.parent
fp = ROOT / "Engine" / "Tests" / "EngineTests.cpp"

lines = fp.read_text(encoding="utf-8").splitlines()
print(f"Loaded: {len(lines)} lines")

# 0-indexed line numbers to remove (second occurrences of duplicate includes)
remove = set([
    260,   # PipelineStateCacheV2.h  (line 261)
    261,   # CacheWarmingService.h   (line 262)
    479,   # BitmapPool.h            (line 480)
    503,   # DeadCodeAudit.h         (line 504)
    504,   # DeadCodeAuditor.h       (line 505)
    679,   # AdaptiveQualityScaler.h (line 680)
    760,   # PerceptualHashEngine.h  (line 761)
    786,   # PredictivePrefetchEngine.h (line 787)
    805,   # PluginDependencyResolver.h (line 806)
    1253,  # FLIFDecoder.h           (line 1254)
    1264,  # ShellContextMenuV2.h    (line 1265)
    1321,  # CacheEncryptionLayer.h  (line 1322)
] + list(range(27528, 27538)))  # Duplicate NPU include block (blank + 9 includes)

print(f"Removing {len(remove)} duplicate lines")

result = [line for i, line in enumerate(lines) if i not in remove]

# Quick stats
test_defs  = sum(1 for l in result if l.startswith("TEST("))
run_tests  = sum(1 for l in result if l.lstrip().startswith("RUN_TEST("))
from collections import Counter
inc_counts = Counter(l for l in result if l.startswith("#include"))
dup_incs   = {k: v for k, v in inc_counts.items() if v > 1}

print(f"Result: {len(result)} lines | TEST(): {test_defs} | RUN_TEST(): {run_tests} | Dup includes: {len(dup_incs)}")
if dup_incs:
    for k, v in sorted(dup_incs.items()):
        print(f"  x{v}: {k}")
    sys.exit(1)

if "--dry-run" in sys.argv:
    print("[DryRun] Not writing.")
    sys.exit(0)

fp.write_text("\n".join(result) + "\n", encoding="utf-8")
print("Written OK")
