---
mode: agent
description: "Generate a standardized SVG architecture or workflow diagram for ExplorerLens using the project's brand palette and conventions"
---

# SVG Diagram Generator — ExplorerLens

Create a publication-quality SVG diagram for the ExplorerLens project.

## Inputs

- **Diagram type:** `${input:diagramType:architecture}` (architecture | workflow | pipeline | component | sequence)
- **Subject:** `${input:subject}`
- **Output file:** `${input:outputPath:docs/assets/diagram.svg}`

## Brand Palette

All ExplorerLens diagrams use this consistent color palette:

| Role | Color | Hex | Usage |
|------|-------|-----|-------|
| Primary | Blue | `#2563EB` | Main components, headers, active states |
| Accent | Amber | `#F59E0B` | Highlights, callouts, version chips |
| Success | Green | `#10B981` | Passing tests, healthy states, completed |
| Error | Red | `#EF4444` | Failures, errors, warnings |
| Neutral | Gray | `#6B7280` | Borders, secondary text, inactive |
| Background | White | `#FFFFFF` | Canvas background |
| Dark text | Slate | `#1E293B` | Primary text |
| Light text | White | `#FFFFFF` | Text on dark backgrounds |

## Typography

```xml
<!-- Standard font stack for all text elements -->
font-family="'Segoe UI', system-ui, -apple-system, sans-serif"

<!-- Size scale -->
<!-- Title: 18px bold -->
<!-- Subtitle: 14px semibold -->
<!-- Body: 12px regular -->
<!-- Label: 10px regular -->
<!-- Caption: 9px light -->
```

## Diagram Templates

### Architecture Box

```xml
<rect x="10" y="10" width="200" height="60" rx="8"
      fill="#2563EB" stroke="#1E40AF" stroke-width="1.5"/>
<text x="110" y="35" text-anchor="middle"
      fill="#FFFFFF" font-size="14" font-weight="600"
      font-family="'Segoe UI', system-ui, sans-serif">
  Component Name
</text>
<text x="110" y="52" text-anchor="middle"
      fill="#BFDBFE" font-size="10"
      font-family="'Segoe UI', system-ui, sans-serif">
  subtitle or size
</text>
```

### Connection Arrow

```xml
<defs>
  <marker id="arrowhead" markerWidth="10" markerHeight="7"
          refX="10" refY="3.5" orient="auto" fill="#6B7280">
    <polygon points="0 0, 10 3.5, 0 7"/>
  </marker>
</defs>
<line x1="210" y1="40" x2="310" y2="40"
      stroke="#6B7280" stroke-width="1.5"
      marker-end="url(#arrowhead)"/>
```

### Version Chip

```xml
<rect x="10" y="10" width="80" height="24" rx="12"
      fill="#F59E0B" stroke="none"/>
<text x="50" y="26" text-anchor="middle"
      fill="#1E293B" font-size="11" font-weight="600"
      font-family="'Segoe UI', system-ui, sans-serif">
  v36.7.0
</text>
```

### Status Badge

```xml
<!-- Pass -->
<rect fill="#10B981" rx="4" width="60" height="20"/>
<text fill="#FFFFFF" font-size="10">PASS</text>

<!-- Fail -->
<rect fill="#EF4444" rx="4" width="60" height="20"/>
<text fill="#FFFFFF" font-size="10">FAIL</text>
```

## Rules

1. **SVG, never PNG** — all diagrams must be scalable vector graphics.
2. **Max 50 KB** — optimize with SVGO if the SVG exceeds this limit.
3. **Accessibility** — every diagram must have a `<title>` and `<desc>` element.
4. **No embedded raster images** — pure vector only.
5. **Consistent spacing** — use 20px grid for element alignment.
6. **Dark mode aware** — use explicit fill colors (no `currentColor` or CSS variables).
7. **`viewBox` required** — always set `viewBox` for responsive scaling.
8. **No inline styles** — use XML attributes for styling.
9. **File location** — save to `docs/assets/` unless a different path is specified.
10. **Version chips** — if the diagram includes version numbers, register it in
    `version-bump.instructions.md` so `Bump-Version.ps1` can patch it.

## Output

Generate the complete SVG file at `${input:outputPath:docs/assets/diagram.svg}`.

Include:
- `<svg>` with `xmlns`, `viewBox`, `width`, `height`
- `<title>` and `<desc>` for accessibility
- All components using the brand palette
- Connection arrows with proper markers
- Labels for all elements
