# ExplorerLens — GitHub Sprint & Issue Tracking Log

> This file is the authoritative local record of sprint deliveries and issue resolutions.
> It is updated after each sprint release and used as the source of truth when the GitHub
> API is unavailable (network timeout, firewall, etc.).
>
> **Format:** Each entry lists the sprint, version, git commit hash, deliverables, and
> GH closure commands to run when the API is reachable.
>
> Last updated: 2026-04-10

---

## How to Close Issues When GH API Is Available

```powershell
# Close a specific issue with a reference to the fix commit
gh issue close <N> --comment "Fixed in commit <hash> (vX.Y.Z)"

# Batch-close all issues in the sprint list:
@(1, 2, 3) | ForEach-Object {
    gh issue close $_ --comment "Resolved in vX.Y.Z — see CHANGELOG.md"
}
```

---

## Sprint Delivery Record (v35.x — Vega Series)

### v35.0.0 "Vega" — Sprint 1281-1290 — Adaptive Fidelity + Roadmap Bootstrap

| Item | Status | Commit |
|------|--------|--------|
| `AdaptiveFidelitySelector.h/.cpp` | ✅ Delivered | `f41436c0` |
| `PartialDecodeStateCache.h/.cpp` | ✅ Delivered | `f41436c0` |
| `ThumbnailETagValidator.h/.cpp` | ✅ Delivered | `f41436c0` |
| `SmartViewportCrop.h/.cpp` | ✅ Delivered | `f41436c0` |
| `FormatCapabilityMatrix.h/.cpp` | ✅ Delivered | `f41436c0` |
| `docs/ROADMAP_V35.md` | ✅ Created | `f41436c0` |
| 10 new tests → 4,674 total | ✅ | `f41436c0` |
| v35.0.0 tag pushed | ✅ | `f41436c0` |
| CI release.yml triggered | ✅ | tag `v35.0.0` |
| 5-registry publish triggered | ✅ | tag `v35.0.0` |

**GH close commands (run when API is available):**
```
# No open issues were associated — sprint was a clean delivery
```

---

### v35.1.0 "Vega-R" — Sprint 1291-1300 — Real-Time Collaboration & Live Edit Sync

| Item | Status | Commit |
|------|--------|--------|
| `LiveSyncTokenManager.h/.cpp` | ✅ Delivered | `58e12150` |
| `CollaborativeCacheCoordinator.h/.cpp` | ✅ Delivered | `58e12150` |
| `ThumbnailDeltaEncoder.h/.cpp` | ✅ Delivered | `58e12150` |
| `ConflictResolutionEngine.h/.cpp` | ✅ Delivered | `58e12150` |
| `RealTimePreviewPipeline.h/.cpp` | ✅ Delivered | `58e12150` |
| 10 new tests → 4,684 total | ✅ | `58e12150` |
| v35.1.0 tag pushed | ✅ | `58e12150` |
| CI release.yml triggered | ✅ | tag `v35.1.0` |
| 5-registry publish triggered | ✅ | tag `v35.1.0` |

**Issues fixed this sprint:**
- Session token budget issue (test bodies not appended) — recovered via session summary; bodies appended to `EngineTests_Late.cpp` at resume
- `EngineTestsExterns.h` architecture documented for first time

**GH close commands:**
```
# No open issues — sprint was clean
```

---

### v35.2.0 "Vega-S" — Sprint 1301-1310 — Network-Aware Streaming Cache

| Item | Status | Commit |
|------|--------|--------|
| `NetworkTopologyProbe.h/.cpp` | ✅ Delivered | `0813a5e8` |
| `StreamingCacheTierPolicy.h/.cpp` | ✅ Delivered | `0813a5e8` |
| `BandwidthThrottleGuard.h/.cpp` | ✅ Delivered | `0813a5e8` |
| `RemoteFileManifestCache.h/.cpp` | ✅ Delivered | `0813a5e8` |
| `CachePrefetchScheduler.h/.cpp` | ✅ Delivered | `0813a5e8` |
| 10 new tests → 4,694 total | ✅ | `0813a5e8` |
| v35.2.0 tag pushed | ✅ | `0813a5e8` |
| CI release.yml triggered | ✅ | tag `v35.2.0` |
| 5-registry publish triggered | ✅ | tag `v35.2.0` |

**Issues fixed this sprint:**
- SBOMGenerator.h file lock: second run of Bump-Version.ps1 succeeded; documented in lessons-learned.md §11.1

**GH close commands:**
```
# No open issues — sprint was clean
```

---

### v35.3.0 "Vega-T" — Sprint 1311-1320 — Zero-Trust Thumbnail Security

| Item | Status | Commit |
|------|--------|--------|
| `ThumbnailManifestSigner.h/.cpp` | ✅ Delivered | `b2965d69` |
| `ZeroTrustDecodeWorker.h/.cpp` | ✅ Delivered | `b2965d69` |
| `TokenBoundCacheEntry.h/.cpp` | ✅ Delivered | `b2965d69` |
| `ThumbnailAuditLog.h/.cpp` | ✅ Delivered | `b2965d69` |
| `FIPSCryptoAdapter.h/.cpp` | ✅ Delivered | `b2965d69` |
| 10 new tests → 4,704 total | ✅ | `b2965d69` |
| v35.3.0 tag pushed | ✅ | `b2965d69` |
| CI release.yml triggered | ✅ | tag `v35.3.0` |
| 5-registry publish triggered | ✅ | tag `v35.3.0` |

**Issues fixed this sprint:**
- SBOMGenerator.h file lock (again): second run resolved; pattern now documented
- docs/lessons-learned.md updated with all Vega series lessons (§11.1–§11.7)
- copilot-instructions.md corrected: rule 18 now references `EngineTestsExterns.h`
- file-size-policy.instructions.md updated with `EngineTestsExterns.h` entry

**GH close commands:**
```
# No open issues — sprint was clean
```

---

## Known Open Items (as of v35.3.0)

| Item | Kind | Priority | Notes |
|------|------|----------|-------|
| Next sprint: v35.4.0 "Vega-U" | Feature | High | Sprint 1321-1330: WASM/Browser Extension Pipeline |
| `EngineTestsExterns.h` approaching 500 KB | Monitor | Low | Currently ~233 KB; split when > 400 KB |
| v35.3.0 GH Release artifacts | Verify | High | Confirm all .dll/.msi/.zip/.sbom attached once CI completes |

---

## Standing Procedures for Issue Closure

After each sprint, verify:

```powershell
# 1. Confirm tag is pushed
git tag -l "v35.*"

# 2. Check CI Release workflow fired (when GH API available)
gh run list --workflow release.yml --limit 5

# 3. Check publish-packages.yml fired
gh run list --workflow publish-packages.yml --limit 5

# 4. Close any issues linked to the sprint
gh issue list --state open --label "sprint-1311-1320"
# then: gh issue close <N> --comment "Resolved in v35.3.0 commit b2965d69"
```

---

## Historical Sprint Summary (v34.x — Arcturus Series)

| Version | Codename | Sprint | Commit | Theme |
|---------|----------|--------|--------|-------|
| v34.0.0 | Arcturus | — | `89634315` | Platform Abstraction Layer (PAL) |
| v34.1.0 | Arcturus-R | 1221-1230 | `5dbac4b6` | Live Preview / Scrubber |
| v34.2.0 | Arcturus-S | 1231-1240 | `1c65ad9b` | Multi-Tenant Cache |
| v34.3.0 | Arcturus-T | 1241-1250 | `8b93fc08` | AI Scene Understanding V2 |
| v34.4.0 | Arcturus-U | 1251-1260 | `c9e241d2` | GPU Decode Acceleration V2 |
| v34.5.0 | Arcturus-V | 1261-1265 | `761ff66f` | Enterprise Policy Engine V2 |
| v34.6.0 | Arcturus-W | 1266-1270 | `eb34b9a2` | Plugin Trust Chain + Sandbox |
| v34.7.0 | Arcturus-X | 1271-1280 | `314f29a0` | Performance Hardening + LTS Gate |

All v34.x issues: closed. No open v34.x items.
