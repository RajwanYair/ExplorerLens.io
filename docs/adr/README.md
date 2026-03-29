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
| [ADR-004](ADR-004-directstorage-pipeline.md) | DirectStorage for GPU Decompression | Proposed | v30.1.0 |
| [ADR-005](ADR-005-platform-abstraction-layer.md) | Platform Abstraction Layer for Cross-Platform | Proposed | v30.0.0 |
| [ADR-006](ADR-006-generative-ai-default-off.md) | Generative AI Features Default Off | Proposed | v31.0.0 |
| [ADR-007](ADR-007-universal-decoder-library.md) | Universal Format Decoder Library Open Source | Proposed | v30.5.0 |
| [ADR-008](ADR-008-c23-migration-strategy.md) | C++23 Migration Strategy | Proposed | v30.0.0+ |

## Format

Each ADR uses the following structure:
- **Title** — Short descriptive title
- **Status** — Proposed / Accepted / Deprecated / Superseded
- **Context** — The situation that motivated the decision
- **Decision** — What was decided
- **Rationale** — Why this choice over alternatives
- **Consequences** — Positive and negative consequences
- **Alternatives Considered** — Other options and why they were rejected
