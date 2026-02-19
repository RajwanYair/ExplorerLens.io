# Sprint 163: Scientific Format Plugin (FITS/NIfTI)

**Block:** v8.3.0 — Phase P3: Format Expansion  
**Status:** ✅ Done  
**Sprint Count:** 163 / 174

---

## Overview

Adds a scientific data format plugin supporting FITS (Flexible Image Transport System) for
astronomy images and NIfTI for medical/neuroimaging data. Includes false-color LUT application
(Viridis, Plasma, Grayscale) for meaningful thumbnail generation.

---

## Deliverables

| Artifact | Path | Notes |
|---|---|---|
| Header | `Engine/Decoders/ScientificFormatPlugin.h` | `FITSHeader`, `NIfTIHeader`, `ScientificLUT`, `GetViridisLUT5()` |
| GTest | `Engine/Tests/Sprint163_Scientific.cpp` | 15 test cases |
| Sprint doc | `docs/development/sprints-v8/SPRINT_163.md` | This document |

---

## Tests (15)

- `ScientificPlugin_FITSHeaderFields` — BITPIX/NAXIS/OBJECT/TELESCOP
- `ScientificPlugin_FITSHeaderDefaultValid`
- `ScientificPlugin_NIfTIHeaderFields` — dim/datatype/vox_offset
- `ScientificPlugin_NIfTIHeaderDefaultValid`
- `ScientificPlugin_LUTTypes` — Grayscale/Viridis/Plasma/Inferno/Jet
- `ScientificPlugin_ViridisLUT5Entries` — 5 entries, first dark blue
- `ScientificPlugin_ViridisLUT5LastYellow` — last entry yellowish
- `ScientificPlugin_LUTInterpolation` — interpolate between entries
- `ScientificPlugin_DecodeStatusValues`
- `ScientificPlugin_DecodeResultFields`
- `ScientificPlugin_PluginInterface`
- `ScientificPlugin_FITSExtension` — .fit/.fits/.fts mapped
- `ScientificPlugin_NIfTIExtension` — .nii/.nii.gz mapped
- `ScientificPlugin_FalseColorApplication` — LUT applied to grayscale
- `ScientificPlugin_Factory`

---

## Acceptance Criteria

- [x] Header compiles with `/W4` zero warnings
- [x] FITS and NIfTI header structs defined
- [x] 5 LUT types with `GetViridisLUT5()` sample implementation
- [x] All 15 GTest cases pass
- [x] Sprint doc created
