# docs/assets/ — SVG Diagrams

## Theme Compatibility (Audited v39.9.0)

All 13 SVGs use **opaque dark backgrounds** (#0d1117 / gradient) with light text.
They render correctly on both GitHub light and dark themes because the background
is baked into the SVG — text is always readable.

**Design choice:** Diagrams always display with a dark appearance regardless of
the viewer's theme preference. This is intentional for visual consistency across
GitHub, MkDocs, and VS Code preview.

### Future Enhancement (P3)

To make SVGs truly theme-adaptive, each would need a `<style>` block with
`@media (prefers-color-scheme: light)` overrides. This is low priority since
readability is not affected.

## Inventory

| File | Purpose |
|------|---------|
| `architecture-build.svg` | MSI artifact pipeline and build system |
| `architecture-components.svg` | High-level component architecture |
| `architecture-dataflow.svg` | Data flow through decode pipeline |
| `cache-architecture.svg` | L1/L2 cache layers and invalidation |
| `ci-cd-pipeline.svg` | 22 GitHub Actions workflows mapped |
| `decode-pipeline.svg` | File → detect → route → decode → thumbnail |
| `format-matrix.svg` | Supported format grid with decoder mapping |
| `gpu-pipeline.svg` | CPU decode → GPU upload → render output |
| `logo.svg` | ExplorerLens project logo |
| `plugin-lifecycle.svg` | Plugin discovery → trust → sandbox → execute |
| `release-flow.svg` | Version bump → tag → release → registries |
| `social-preview.svg` | GitHub social preview card |
| `test-architecture.svg` | Test framework, corpus, CI integration |
