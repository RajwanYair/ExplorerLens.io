---
name: Performance Issue
about: Report a performance problem (slow execution, high memory, etc.)
title: "[Perf] "
labels: performance
assignees: ''
---

## Summary

Brief description of the performance issue.

## Project / Component

Which component is slow? (e.g., `LENSShell.dll / HEIC decode`, `LENSManager / startup`)

## Observed Performance

- **Operation:** (e.g., decoding 100 HEIC files)
- **Time taken:** (e.g., 4500 ms total)
- **Expected time:** (e.g., ~1000 ms, per benchmark baseline)
- **Memory used:** (e.g., 150 MB peak)

## Profiling Data

<details>
<summary>Profile output (optional)</summary>

```text
Paste ETW trace / EngineTests benchmark / lens benchmark output here
```

</details>

## Environment

- **OS:** (e.g., Windows 11 24H2)
- **CPU:** (e.g., Intel Core i7-12700K)
- **RAM:** (e.g., 32 GB)
- **GPU:** (e.g., Intel Arc A770 / NVIDIA RTX 4090 / CPU fallback)
- **Storage:** (e.g., NVMe SSD / HDD)
- **ExplorerLens Version:** (e.g., v39.9.0)

## Additional Context

Dataset size, file types, specific flags used, etc.
