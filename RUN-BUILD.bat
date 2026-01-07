@echo off
REM DarkThumbs - Run Fixed Production Build
REM This batch file runs the fixed PowerShell build script

echo.
echo ========================================
echo   DarkThumbs Production Build (Fixed)
echo ========================================
echo.
echo This will clean and rebuild all components.
echo Estimated time: 10-15 minutes
echo.
pause

cd /d "%~dp0"

echo.
echo Starting PowerShell build script...
echo.

powershell.exe -ExecutionPolicy Bypass -File "Build-Production-Fixed.ps1" -Clean -Verbose

echo.
echo ========================================
echo   Build Process Complete
echo ========================================
echo.
echo Check the latest log file in build-logs\ directory
echo.
pause
