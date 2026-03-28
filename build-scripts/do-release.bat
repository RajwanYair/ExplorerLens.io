@echo off
cd /d "c:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\ExplorerLens.io"
echo --- git add -A ---
git add -A
echo --- git status ---
git status --short
echo --- git commit ---
git commit -m "chore: bump version to v23.6.0 (Vega-W)" -m "Release v23.6.0 Vega-W - Security Hardening v2 (Sprints 461-470)" -m "- 8 new security headers: ZeroTrust, DecoderSandbox, RuntimeIntegrity, ExploitMitigation, PrivilegeSeparation, SecureIPC, AuditTrail, AntiTamper" -m "- Test count: 3117 unit tests (+179)" -m "- Added SPRINT_PLAN_700.md (Sprints 661-760, v26.2.0 Canopus-S through v27.3.0 Sirius-T)" -m "- Updated all version references, README badge, SVG graphics, CHANGELOG"
echo --- git tag ---
git tag -a v23.6.0 -m "Release v23.6.0 (Vega-W) - Security Hardening v2"
echo --- git push ---
git push origin main --tags
echo --- DONE ---
echo Visit: https://github.com/RajwanYair/ExplorerLens.io/releases/tag/v23.6.0
