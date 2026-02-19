# Sprint 177: Version Normalization

## Objective
Update ALL documentation files to consistent version v8.4.0 and add missing CHANGELOG entries for v8.0-v8.4.

## Changes

### Version Updates Applied

| File | Previous Version | New Version |
|---|---|---|
| `README.md` | v7.0.0 / v7.1.0 | v8.4.0 |
| `CHANGELOG.md` | v7.1.0 (latest entry) | v8.4.0 (5 new entries: v8.0-v8.4) |
| `.github/copilot-instructions.md` | v8.3.0 | v8.4.0 |
| `docs/formats/CAPABILITY_AUDIT.md` | v7.1.0, "42+ formats" | v8.4.0, "200+ formats" |
| `docs/PERFORMANCE.md` | v7.0.0 / v6.2.0 | v8.4.0 |
| `docs/formats/DECODER_STATUS.md` | v7.1.0 | v8.4.0 |
| `docs/development/CODE_QUALITY_STANDARDS.md` | v7.1.0 | v8.4.0 |
| `SDK/docs/PLUGIN_SDK.md` | v7.1.0 | v8.4.0 |
| `tests/README.md` | v6.0.0 | v8.4.0 |

### README.md Specific Updates
- Version badge: 7.1.0 → 8.4.0
- Sprint count badge: 74 → 176
- Test count badge: 100% pass → 437 pass
- Platform badge: x64 → x64 | ARM64
- Decoder count: 24 → 25
- Status section: Updated version, test count, next milestone
- Footer: Updated date and DirectX version

### CHANGELOG.md New Entries
- **v8.4.0** — Sprints 175-177 (bug fixes, shell expansion, version normalization)
- **v8.3.0** — Sprints 150-174 (plugin ecosystem, ARM64, format expansion, memory, release)
- **v8.2.0** — Sprints 125-149 (advanced decoders)
- **v8.1.0** — Sprints 100-124 (SVG, RAW, PDF, archive expansion)
- **v8.0.0** — Sprints 75-99 (modern image formats, engine library, CMake)

### copilot-instructions.md Updates
- Version: 8.3.0 → 8.4.0
- Sprint count: 174 → 177
- Added v8.4.0 Sprint summary table
- Updated sprint execution guidance references

## Testing
- All version strings verified consistent across codebase
- No code changes — documentation only sprint

## Impact
- Future AI sessions will see correct version (v8.4.0) in copilot instructions
- CHANGELOG now has complete history from v7.1 through v8.4
- All documentation cross-references are consistent
