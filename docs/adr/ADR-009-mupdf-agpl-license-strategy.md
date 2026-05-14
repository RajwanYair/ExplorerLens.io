# ADR-009: MuPDF AGPL-3.0 License Strategy

**Status:** Accepted  
**Date:** 2026-04-21  
**Version:** v36.2.0 "Antares"  
**Deciders:** Project maintainers  
**ROADMAP ref:** R10, D25

## Context

ExplorerLens is licensed under the **MIT License**. MuPDF 1.24.11, used for PDF
thumbnail generation, is licensed under **AGPL-3.0**.

AGPL-3.0 requires that any software linking to MuPDF must either:

1. Release the entire combined work under AGPL-3.0, or
1. Obtain a commercial license from Artifex Software.

This creates a license incompatibility: MIT-licensed ExplorerLens cannot distribute
binaries that statically link AGPL-3.0 MuPDF without violating one of the licenses.

### Current Usage

MuPDF is used exclusively in the PDF decoder (`Engine/Decoders/`) for:

- First-page rasterization at thumbnail resolution (256×256)
- PDF metadata extraction (page count, title)

## Decision

**Keep MuPDF with documented AGPL compliance and evaluate PDFium as a Phase 3 replacement.**

### Immediate actions (v36.2.0)

1. **Document the AGPL dependency** prominently in README.md, LICENSE, and SBOM
1. **Isolate MuPDF behind an interface** (`IPdfDecoder`) to make swapping trivial
1. **Source code availability:** Ensure all ExplorerLens source is available (it already is — public GitHub repo)

### Phase 3 evaluation

1. **Evaluate PDFium** (BSD-3-Clause, Google/Chromium) as a drop-in replacement
1. **Evaluate Poppler** (GPL-2.0 — still problematic but less restrictive than AGPL)
1. **If PDFium works:** migrate and remove MuPDF dependency entirely
1. **If PDFium insufficient:** obtain Artifex commercial license ($pricing TBD)

## Rationale

- MuPDF is the best PDF renderer for quality and performance at thumbnail sizes
- PDFium (used by Chrome) is the most viable BSD-licensed alternative
- Since ExplorerLens source is already public on GitHub, AGPL's source disclosure
  requirement is de facto satisfied for the current distribution model
- The interface abstraction (`IPdfDecoder`) makes future migration a single-file change
- Delaying the decision to Phase 3 avoids blocking Phase 1 foundation work

## Consequences

### Positive

- No immediate disruption to the working PDF pipeline
- Clear migration path documented
- Interface abstraction improves testability regardless of outcome

### Negative

- Binary distributions technically require AGPL compliance until MuPDF is replaced
- Users who fork and modify without publishing source may be non-compliant
- PDFium integration requires building Chromium's PDF library (non-trivial build)

## Alternatives Considered

| Alternative | License | Quality | Build Complexity | Verdict |
| ------------- | --------- | --------- | ----------------- | --------- |
| **MuPDF (keep)** | AGPL-3.0 | Excellent | Low (already integrated) | Current choice with compliance docs |
| **PDFium** | BSD-3-Clause | Very good | High (Chromium build system) | Phase 3 evaluation target |
| **Poppler** | GPL-2.0 | Good | Medium | Still GPL — not MIT-compatible |
| **pdf.js (via wasm)** | Apache-2.0 | Good | High (WASM bridge) | Overkill for thumbnails |
| **Artifex commercial** | Commercial | Excellent | Low | Cost TBD; eliminates license concern |
| **Windows PDF API** | N/A (OS) | Basic | Low | Windows 10+ only; limited control |
