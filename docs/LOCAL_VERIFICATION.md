# Local Documentation Verification — ExplorerLens

Use this guide to verify the docs site, `index.html`, and all SVG diagrams render correctly before
pushing a documentation change.

---

## Quick Start

### Option A — VS Code Task (recommended)

1. Open the **Run Task** palette (`Ctrl+Shift+B` or `Terminal → Run Task`)
2. Select **Serve Local Site**
3. Open a browser and navigate to <http://localhost:8080>

### Option B — PowerShell directly

```powershell
.\build-scripts\Serve-Docs.ps1          # default port 8080
.\build-scripts\Serve-Docs.ps1 -Port 9000
```

Press **Ctrl-C** to stop the server.

---

## Prerequisites

| Tool | Minimum version | Install |
|------|----------------|---------|
| Python 3 | 3.9+ | https://python.org |
| (Optional) MkDocs | 1.5+ | `pip install mkdocs mkdocs-material` |

---

## Verification Checklist

Run through these checks after every documentation change:

### Root Site

- [ ] `http://localhost:8080/index.html` loads without 404 or console errors
- [ ] Version chip shows the current version (e.g., `38.4.0`)
- [ ] All navigation links reach valid targets

### Architecture SVGs (`docs/assets/`)

Check each diagram loads and is readable in both browser light mode and dark mode:

| Diagram | URL |
|---------|-----|
| System context | `/docs/assets/system-context.svg` |
| Decode pipeline | `/docs/assets/decode-pipeline.svg` |
| Cache architecture | `/docs/assets/cache-architecture.svg` |
| Format matrix | `/docs/assets/format-matrix.svg` |
| GPU pipeline | `/docs/assets/gpu-pipeline.svg` |
| Plugin lifecycle | `/docs/assets/plugin-lifecycle.svg` |
| Release flow | `/docs/assets/release-flow.svg` |
| Architecture build | `/docs/assets/architecture-build.svg` |
| Social preview | `/docs/assets/social-preview.svg` |
| Test architecture | `/docs/assets/test-architecture.svg` |

### Markdown Docs

```powershell
# Verify mkdocs builds cleanly (strict mode = treat warnings as errors)
python -m mkdocs build --strict --config-file docs/mkdocs.yml
```

Expected: `INFO - Documentation built in …s`, exit code 0, no warnings.

### CHANGELOG

- [ ] `http://localhost:8080/CHANGELOG.md` is accessible
- [ ] Top entry matches the current version in `VERSION`

---

## CI Equivalent

The local checks above mirror the **docs-validation** CI job in
`.github/workflows/docs-validation.yml`.  If the local build passes, CI should pass.

---

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| "Address already in use" | Another process owns the port — use `-Port 9000` |
| Python not found | Install Python 3.9+, ensure it is on `PATH` |
| SVG not found (404) | Check the file exists in `docs/assets/`; run `git status` |
| `mkdocs build` errors | Run `pip install mkdocs mkdocs-material` to install missing plugins |
