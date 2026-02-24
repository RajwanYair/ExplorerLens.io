@echo off
REM Quick LENSManager GUI Validation Script
REM Launches LENSManager and provides manual checklist

echo ========================================
echo LENSManager GUI Manual Validation
echo ========================================
echo.
echo Starting LENSManager.exe...
start "" "..\..\x64\Release\LENSManager.exe"
timeout /t 2 /nobreak >nul
echo.
echo Please verify the following items:
echo.
echo LAYOUT CHECKS:
echo [ ] 1. Dialog window appears (approx 480x500 pixels)
echo [ ] 2. All group boxes are visible and not overlapping
echo [ ] 3. Comic Book Formats (top left) has 4 checkboxes
echo [ ] 4. E-Book Formats (top right) has 4 checkboxes
echo [ ] 5. Archive Formats (middle left) has 4 checkboxes
echo [ ] 6. Photo ^& Other Formats (middle right) has 2 checkboxes
echo [ ] 7. Modern Image Formats (lower left) has 4 checkboxes
echo [ ] 8. Media ^& Documents (lower right) has 5 checkboxes INCLUDING RAW
echo [ ] 9. Collage Mode (bottom left) has 4 radio buttons
echo [ ] 10. Advanced Options (bottom right) has 2 checkboxes
echo [ ] 11. OK, Cancel, Apply buttons at bottom
echo [ ] 12. Status bar at very bottom showing "Ready • X of 31 formats enabled"
echo.
echo RAW CHECKBOX CHECKS:
echo [ ] 13. RAW checkbox is visible in Media ^& Documents section
echo [ ] 14. RAW checkbox label reads "RAW Photos (DNG, CR2, NEF)"
echo [ ] 15. RAW checkbox is BELOW SVG checkbox
echo [ ] 16. RAW checkbox is INSIDE the Media ^& Documents group box
echo [ ] 17. RAW checkbox has a colored status dot (green/gray) next to it
echo.
echo FUNCTIONALITY CHECKS:
echo [ ] 18. Click RAW checkbox - it toggles ON/OFF
echo [ ] 19. Hover over RAW checkbox - tooltip appears
echo [ ] 20. Tooltip says "RAW Photos (requires Camera Codec Pack)"
echo [ ] 21. Click Apply button - status bar shows "Applying..."
echo [ ] 22. Status bar returns to "Ready" after applying
echo [ ] 23. Close and reopen - RAW checkbox state is remembered
echo [ ] 24. Status bar format count updates when toggling checkboxes
echo.
echo DARK MODE CHECK (if Windows is in dark mode):
echo [ ] 25. Dialog has dark background
echo [ ] 26. Text is light colored (readable on dark)
echo [ ] 27. Group boxes have dark theme styling
echo.
echo ========================================
echo.
echo If ALL checks pass, the GUI is working correctly!
echo If ANY checks fail, report the issue number.
echo.
pause

