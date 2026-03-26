# ExplorerLens v17.0.0 "Nova" — Store Certification Checklist

**Target:** Windows 11 Microsoft Store submission  
**Package:** `ExplorerLens-17.0.0-x64.msix`  
**WACK Version:** Windows App Certification Kit 10.0.26100

---

## Part 1 — WACK Automated Test Results

Run: `appcert.exe run -apptype desktop -setuppath ExplorerLens-17.0.0-x64.msix -setuptype store`

| Test | Status | Notes |
|------|--------|-------|
| Supported API test | ✅ Pass | No prohibited API calls detected |
| Supported API test (debug binaries) | ✅ Pass | Release-only binaries verified |
| App manifest resources test | ✅ Pass | All language resources validated |
| Package sanity test | ✅ Pass | MSIX structure correct |
| Windows security features test | ✅ Pass | DEP/ASLR/CFG/SafeSEH on all binaries |
| Supported API test (PCA) | ✅ Pass | ProgID compatibility entries verified |
| Branding | ✅ Pass | No generic placeholder branding |
| Capability declarations | ✅ Pass | Only `runFullTrust` + `allowElevation` declared |
| COM server validation | ✅ Pass | CLSID `9E6ECB90-5A61-42BD-B851-D3297D9C7F39` registered |
| Performance requirements | ✅ Pass | Cold launch < 2s; thumbnail < 500ms first request |
| Crash detection | ✅ Pass | No crashes in 1-hour stress test |

**Overall WACK Result: PASS**

---

## Part 2 — Store Submission Checklist

### Package Metadata

- [ ] Display name: `ExplorerLens – Thumbnail Enhancer`
- [ ] Publisher: must match Windows Dev Center account  
- [ ] Version: `17.0.0.0` (4-component Store format)
- [ ] Description: max 10,000 characters, no keyword stuffing
- [ ] Short description: max 270 characters
- [ ] Screenshots: 4 × 1280×800 or 1366×768 (required); 1920×1080 (optional)
- [ ] Icon: 300×300 PNG (Store tile), 150×150 (small), 44×44 (app list)
- [ ] Age rating: IARC questionnaire completed → `Everyone`
- [ ] Privacy policy URL: https://exploreriens.io/privacy
- [ ] Support URL: https://github.com/RajwanYair/ExplorerLens.io/issues

### Compliance

- [ ] GDPR: no EU personal data collected without consent
- [ ] COPPA: no features targeted at children under 13
- [ ] Export controls: no encryption beyond Windows built-ins (BCrypt)
- [ ] Content policy: thumbnails of user files — no server-side content generation

### Binary Signing

- [ ] `LENSShell.dll` — dual-signed (SHA-256 + SHA-1 timestamp)
- [ ] `LENSManager.WinUI.exe` — dual-signed
- [ ] `LENSManager.exe` — dual-signed
- [ ] MSIX package — Store-signed by Partner Center (auto on submission)

### Testing Matrix

- [ ] Clean Win11 23H2 VM — install, register, generate thumbnails (all 25 decoders)
- [ ] Clean Win10 22H2 VM — backwards compat verification
- [ ] FIPS-enabled Win2022 Server — FIPS compliance mode verified
- [ ] Domain-joined machine — GPO policy template applied and respected
- [ ] High-DPI (200%) display — all UI renders correctly
- [ ] Screen reader (Narrator) — NavigationView items + buttons accessible

---

## Part 3 — Store Asset Inventory

| Asset | Resolution | Format | Status |
|-------|-----------|--------|--------|
| Store listing icon | 300×300 | PNG | Required |
| Small tile | 150×150 | PNG | Required |
| App list icon | 44×44 | PNG | Required |
| Screenshot 1 — Dashboard | 1280×800 | PNG | Required |
| Screenshot 2 — Formats page | 1280×800 | PNG | Required |
| Screenshot 3 — GPU page | 1280×800 | PNG | Required |
| Screenshot 4 — Settings | 1280×800 | PNG | Required |
| Promotional banner | 1920×1080 | PNG | Optional |
| Xbox tile | 584×800 | PNG | For PC Game Pass area |

---

*Maintained by the ExplorerLens release team — Sprint 97 execution.*
