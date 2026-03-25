@echo off
cd /d "c:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\ExplorerLens.io"
echo --- git add -A ---
git add -A
echo --- git status ---
git status --short
echo --- git commit ---
git commit -m "feat: v15.3.0 Zenith-T - Resilience & Hardening (Sprints 9-16 of 100)"
echo --- git tag ---
git tag v15.3.0
echo --- git push ---
git push origin main --tags
echo --- DONE ---
