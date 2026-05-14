# ExplorerLens Architecture Diagrams

Visual architecture documentation for the ExplorerLens project.

## SVG Diagrams

Dark-theme SVG diagrams in [`docs/assets/`](../assets/):

| Diagram | Description |
| --------- | ------------- |
| [System Components](../assets/architecture-components.svg) | High-level component overview |
| [Data Flow](../assets/architecture-dataflow.svg) | Thumbnail generation data flow |
| [Build Pipeline](../assets/architecture-build.svg) | CMake + MSBuild + packaging pipeline |
| [Decode Pipeline](../assets/decode-pipeline.svg) | 5-stage file → thumbnail decode pipeline |
| [CI/CD Pipeline](../assets/ci-cd-pipeline.svg) | GitHub Actions workflow visualization |
| [Test Architecture](../assets/test-architecture.svg) | Test framework (custom macros + Catch2) |
| [Release Flow](../assets/release-flow.svg) | Bump-Version → tag → GitHub Release → registries |
| [Format Matrix](../assets/format-matrix.svg) | 200+ extensions × decoder family grid |
| [Cache Architecture](../assets/cache-architecture.svg) | L1 memory → L2 disk → invalidation flow |
| [Plugin Lifecycle](../assets/plugin-lifecycle.svg) | Discovery → trust → sandbox → execute |
| [GPU Pipeline](../assets/gpu-pipeline.svg) | CPU decode → upload → GPU compute → readback |

## Mermaid Diagrams

Interactive Mermaid diagrams (rendered natively on GitHub):

| Diagram | Description |
| --------- | ------------- |
| [system-overview.md](system-overview.md) | High-level system components and data flow |
| [decode-pipeline.md](decode-pipeline.md) | Thumbnail decode pipeline stages |
| [plugin-architecture.md](plugin-architecture.md) | Plugin ecosystem and trust chain |

## Viewing

- **SVGs:** Open directly in any browser, or view inline on GitHub
- **Mermaid:** GitHub renders natively in `.md` files; or use the [VS Code Mermaid Preview](https://marketplace.visualstudio.com/items?itemName=bierner.markdown-mermaid)
  extension
