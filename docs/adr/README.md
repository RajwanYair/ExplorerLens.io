# ExplorerLens — Architecture Decision Records

This directory contains Architecture Decision Records (ADRs) documenting key design
choices made during ExplorerLens development. Each ADR explains the context, decision,
rationale, and consequences of significant architectural choices.

## Index

| ADR | Title | Status | Version |
|-----|-------|--------|---------|
| [ADR-001](ADR-001-WASM-plugin-sandbox.md) | WASM Plugin Sandbox over Native DLL | Accepted | v25.0.0 |
| [ADR-002](ADR-002-NPU-heterogeneous-compute.md) | NPU & Heterogeneous Compute Routing | Accepted | v25.2.0 |
| [ADR-003](ADR-003-CLIP-semantic-search.md) | CLIP Embeddings for Semantic Search | Proposed | v30.2.0 |
| [ADR-008](ADR-008-c23-migration-strategy.md) | C++23 Migration Strategy | Proposed | v30.0.0+ |
| [ADR-009](ADR-009-mupdf-agpl-license-strategy.md) | MuPDF AGPL-3.0 License Strategy | Accepted | v36.2.0 |
| [ADR-010](ADR-010-catch2-test-migration.md) | Catch2 as Primary Test Framework | Accepted | v36.2.0+ |
| [ADR-011](ADR-011-streaming-decoder-interface.md) | IStreamingDecoder Probe-then-Decode Interface | Accepted | v38.4.0+ |
| [ADR-012](ADR-012-engine-directory-consolidation.md) | Engine Directory Consolidation 16→7 | Accepted | v38.4.0 |
| [ADR-013](ADR-013-sqlite-l2-cache.md) | SQLite WAL for L2 Thumbnail Cache Index | Accepted | v38.4.0+ |
| [ADR-014](ADR-014-safe-integer-overflow.md) | Safe Integer Arithmetic for Decode Dimensions | Accepted | v38.7.0 |

## Format

Each ADR uses the following structure:
- **Title** — Short descriptive title
- **Status** — Proposed / Accepted / Deprecated / Superseded
- **Context** — The situation that motivated the decision
- **Decision** — What was decided
- **Rationale** — Why this choice over alternatives
- **Consequences** — Positive and negative consequences
- **Alternatives Considered** — Other options and why they were rejected
