# Shared Tooling Architecture

> Documents the configuration inheritance model for ExplorerLens and sibling projects
> under the `MyScripts\` workspace. See ROADMAP §11 for the full plan.

## Inheritance Model

```
MyScripts\                              ← SHARED (all projects inherit)
├── .editorconfig                       ← Universal editor rules (cascades natively)
├── pyproject.toml                      ← Shared Python tool config (ruff, mypy)
├── pyrightconfig.json                  ← Shared type-checking baseline
│
├── ExplorerLens.io/                    ← PROJECT-SPECIFIC OVERRIDES
│   ├── .editorconfig                   ← C++ overrides (indent=4, max_line=120)
│   ├── .clang-format                   ← C++20 formatting (Microsoft base style)
│   ├── .clang-tidy                     ← C++ static analysis checks
│   ├── .gitattributes                  ← Line ending rules (CRLF for C++, LF for YAML)
│   ├── .vscode/                        ← Workspace settings, tasks, launch, MCP
│   ├── CMakePresets.json               ← CMake build presets
│   ├── vcpkg.json                      ← C++ dependency manifest
│   ├── scoopfile.json                  ← Dev tool versions (cmake, ninja, nasm, etc.)
│   └── .github/                        ← Repo-specific CI, agents, instructions
│
├── RegiLattice/                        ← Python project (inherits shared Python config)
├── Scripts.DupDetector/                ← Python project
└── ...other projects
```

## Key Rules

1. **Inherit, don't duplicate.** If `MyScripts\.editorconfig` covers a rule, don't repeat it in the project.
2. **Override only what differs.** Project configs contain only the delta from the shared baseline.
3. **`.editorconfig` cascades natively** — the EditorConfig spec defines upward directory search.
4. **VS Code multi-root workspaces** cascade settings from outer `.vscode/` to inner.
5. **Never put secrets or machine-local paths** in shared configs.

## Configuration File Registry

### ExplorerLens-Specific (not shared)

| File | Purpose | Why project-specific |
|------|---------|----------------------|
| `.clang-format` | C++ formatting | Only C++ project needs this |
| `.clang-tidy` | C++ static analysis | Only C++ project needs this |
| `CMakePresets.json` | CMake build presets | Only C++ project uses CMake |
| `vcpkg.json` | C++ package manifest | Only C++ project uses vcpkg |
| `scoopfile.json` | Dev tools manifest | C++ tools (NASM, Meson, WiX) |
| `.vscode/c_cpp_properties.json` | IntelliSense config | MSVC v145 paths |
| `.vscode/mcp.json` | MCP server config | Repo-scoped GitHub + filesystem |
| `.github/` (entire directory) | CI, agents, instructions | Repo-specific |

### Shared at MyScripts Level

| File | Purpose | Inherited by |
|------|---------|--------------|
| `.editorconfig` | Charset, indentation, line endings | All projects |
| `pyproject.toml` | Ruff, mypy, pytest defaults | All Python projects |
| `pyrightconfig.json` | Type checking baseline | All Python projects |

### Override Pattern Example

```ini
# MyScripts\.editorconfig (shared)
[*]
charset = utf-8
indent_style = space
indent_size = 4
end_of_line = lf

# ExplorerLens.io\.editorconfig (override)
# Inherits all shared rules, adds C++ specifics:
[*.{h,cpp}]
indent_size = 4
end_of_line = crlf
max_line_length = 120

[*.{yml,yaml,json}]
indent_size = 2
```

## Tool Version Pinning

Tools are pinned in two places:

| Source | Format | Used by |
|--------|--------|---------|
| `scoopfile.json` | JSON manifest | Local dev: `scoop install` |
| `.github/standards/tool-versions.md` | Markdown table | CI reference + documentation |
| `.devcontainer/devcontainer.json` | JSON | Codespaces / dev containers |

**Rule:** When updating a tool version, update all three locations. The `Bump-Version.ps1`
script handles `tool-versions.md`; `scoopfile.json` and devcontainer must be updated manually.

## Status

| Step | Status |
|------|--------|
| Audit config files across projects | ⏳ In progress |
| Consolidate shared configs at `MyScripts\` | 🔜 Next |
| Create this document (`TOOLING.md`) | ✅ Done |
| Add CI check flagging duplicate configs | 🔜 Phase 2 |
