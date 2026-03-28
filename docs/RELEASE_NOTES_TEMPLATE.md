# Release Notes Template — ExplorerLens
# This file documents the release notes format for ExplorerLens releases.
# Used by the automated release workflow and manual release notes.
# Copy this template when drafting release notes for a new version.
# Current version: v23.7.0 "Vega-X" | Last updated: 2026-03-28

## ExplorerLens v{VERSION} "{CODENAME}" — {DATE}

### What's New

> *(Summarise the 2–3 most impactful changes in 2–3 sentences for non-technical users.)*

ExplorerLens v{VERSION} delivers [KEY_FEATURE_1], [KEY_FEATURE_2], and [KEY_FEATURE_3].
GPU-accelerated thumbnail generation continues to support **200+ file formats** across
25 specialized decoders with sub-17ms per-thumbnail performance and 3,125+ unit tests.

---

### Highlights

| Area | Change |
|------|--------|
| ✨ Feature | [FEATURE_DESCRIPTION] |
| 🐛 Fixed | [BUG_DESCRIPTION] |
| ⚡ Performance | [PERF_IMPROVEMENT] |
| 🔒 Security | [SECURITY_NOTE] |

---

### Added
- [ITEM_1]
- [ITEM_2]

### Fixed
- [BUG_1]
- [BUG_2]

### Changed
- [CHANGE_1]

### Performance
- [PERF_1]

---

### Installation

**Via MSI (recommended):**
1. Download `ExplorerLens-{VERSION}-x64.msi`
2. Run as Administrator
3. Thumbnails appear automatically in Windows Explorer

**Via MSIX (Store-style, Sprint 38+):**
1. Download `ExplorerLens-{VERSION}-x64.msix`
2. Double-click to install (or `Add-AppxPackage` via PowerShell)

**Manual (portable):**
1. Download `ExplorerLens-{VERSION}-x64.zip`
2. Extract and run `Install-ExplorerLens.ps1` as Administrator

---

### Supported Formats

| Category | Formats |
|----------|---------|
| Images | JPEG, PNG, WebP, AVIF, JPEG XL, HEIC/HEIF, TIFF, BMP, GIF |
| RAW Camera | CR2, NEF, ARW, DNG, ORF, RW2, PEF (via LibRaw 0.21.3) |
| Archives | ZIP, 7-Zip, RAR, TAR, GZ, BZ2, XZ, ZST |
| Documents | PDF (via MuPDF 1.24.11) |
| 3D/CAD | GLTF, GLB, OBJ, FBX, STEP, STL |
| Scientific | HDF5, NetCDF, FITS, VTK |

---

### System Requirements

| Component | Minimum |
|-----------|---------|
| OS | Windows 10 build 17763 (RS5, October 2018 Update) |
| Architecture | x64 |
| GPU | DirectX 11 compatible (CPU fallback always available) |
| Runtime | Visual C++ Redistributable 2022 (bundled in MSI) |

---

### Checksums

Verify download integrity using `SHA256SUMS.txt` (included in the release):

```powershell
Get-Content SHA256SUMS.txt | ForEach-Object {
    $hash, $file = $_ -split '\s+', 2
    $actual = (Get-FileHash $file -Algorithm SHA256).Hash.ToLower()
    if ($actual -eq $hash) { "OK: $file" } else { "MISMATCH: $file" }
}
```

---

### Full Changelog

See [CHANGELOG.md](https://github.com/RajwanYair/ExplorerLens.io/blob/main/CHANGELOG.md#[ANCHOR])
for complete commit-level details.

---

*ExplorerLens is an open-source project. Contributions welcome!*
*[GitHub](https://github.com/RajwanYair/ExplorerLens.io) • [Report Issues](https://github.com/RajwanYair/ExplorerLens.io/issues)*
