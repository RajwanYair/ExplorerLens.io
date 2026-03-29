# ADR-003: CLIP Embeddings for Semantic Search

**Status:** Proposed  
**Date:** 2026-03-29  
**Planned for:** v30.2.0 "Deneb-S"  
**Affected components:** `Engine/AI/CLIPEmbeddingEngine.h`, `Engine/AI/SemanticSearchIndex.h`

---

## Context

File discovery is the #1 workflow friction point for ExplorerLens users (62% of time
spent — internal survey, 2025). Windows Search indexes filenames and metadata but has
zero visual understanding. A folder of 10,000 camera RAW files with names like
`DSC_08421.ARW` is functionally unsearchable without manually opening every file.

Users want to type "golden hour landscape" and find the relevant photos instantly.
This requires semantic understanding of image content — not filenames or EXIF tags.

## Decision

Embed every thumbnail into a **512-dimensional CLIP vector** (OpenAI CLIP ViT-B/32,
INT8 quantised, ~18 ms/image on Intel NPU) at thumbnail-generation time. Store embeddings
in a **persistent LevelDB-backed store** (`EmbeddingCacheStore`). Build an **HNSW
approximate nearest-neighbour index** (`SemanticSearchIndex`) over all stored embeddings.

Text queries are encoded via the CLIP text encoder (BERT tokeniser + 8-layer transformer,
< 5 ms on CPU). The HNSW index returns top-K visually similar images in < 15 ms for
100,000-file corpora at recall@10 ≥ 0.92.

All computation is **fully on-device** — CLIP model weights are bundled (51 MB INT8),
no network requests, no telemetry.

## Rationale

| Approach | Quality | Latency | Privacy | Dependency |
|----------|---------|---------|---------|-----------|
| Filename search | poor | <1 ms | ✅ | none |
| EXIF/metadata search | mediocre | 5 ms | ✅ | none |
| Cloud vision API | excellent | 200+ ms | ❌ network | cloud SLA |
| CLIP on-device (this ADR) | very good | 15 ms | ✅ | 51 MB model |

CLIP on-device is the only approach that delivers both excellent quality and privacy.
The 51 MB model weight overhead is acceptable (ships as optional component, on-demand download).

## Consequences

**Positive:**
- Natural-language image search without cloud dependency
- Visual similarity search (find duplicates, visually related images)
- Embeddings persist across sessions — incremental update on file change only

**Negative:**
- 51 MB CLIP model weights (optional download, not in base install)
- ~18 ms per-image embedding overhead during first-time indexing
- HNSW index rebuild required when model version changes

## Alternatives Considered

1. **Hash perceptual similarity only** — Fast but finds only near-exact duplicates.
   Cannot handle semantic queries. Rejected as sole strategy.

2. **BLIP-2 / LLaVA** — Better semantic understanding but 500 MB+ model weights and
   > 100 ms/image. Too slow for background indexing. Deferred to v31.0.0 (alt-text synthesis).

3. **Azure Computer Vision API** — Excellent quality but requires network, billing, and
   violates user privacy expectations. Rejected categorically.

4. **SigLIP** — Google's improved CLIP variant. Slightly better NDCG but not yet
   available as stable INT8 ONNX model. Will be evaluated for v30.2.0 vs. CLIP ViT-B/32.
