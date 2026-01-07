@echo off
REM build-tests.cmd - Build GPU test tools
REM Builds GPUThumbnailTest.exe and CBXBench.exe

setlocal enabledelayedexpansion

echo ========================================
echo Building GPU Test Tools
echo ========================================
echo.

REM Find MSBuild
set "MSBUILD="
for %%i in (msbuild.exe) do set "MSBUILD=%%~$PATH:i"

if not defined MSBUILD (
    set "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe"
)

if not exist "%MSBUILD%" (
    echo ERROR: MSBuild not found!
    echo Please install Visual Studio Build Tools 2022
    exit /b 1
)

echo Using MSBuild: %MSBUILD%
echo.

REM Find C++ compiler
set "CL="
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VSINSTALL=%%i"
)

if defined VSINSTALL (
    call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat"
) else (
    echo ERROR: Visual Studio C++ compiler not found!
    exit /b 1
)

echo.
echo ========================================
echo Building GPUThumbnailTest.exe
echo ========================================
echo.

cl.exe /EHsc /std:c++20 /O2 /Fe:GPUThumbnailTest.exe ^
    tests\GPUThumbnailTest.cpp ^
    /I. ^
    /link windowscodecs.lib shlwapi.lib

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to build GPUThumbnailTest.exe
    exit /b 1
)

echo.
echo ========================================
echo Building CBXBench.exe
echo ========================================
echo.

cl.exe /EHsc /std:c++20 /O2 /Fe:CBXBench.exe ^
    tests\CBXBench.cpp ^
    /I. ^
    /link windowscodecs.lib

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to build CBXBench.exe
    exit /b 1
)

echo.
echo ========================================
echo Build Complete!
echo ========================================
echo.
echo Executables:
if exist GPUThumbnailTest.exe (
    dir GPUThumbnailTest.exe | findstr "GPUThumbnailTest.exe"
)
if exist CBXBench.exe (
    dir CBXBench.exe | findstr "CBXBench.exe"
)

echo.
echo Usage:
echo   GPUThumbnailTest.exe -i C:\TestImages -o C:\Thumbnails -s 256 -v
echo   CBXBench.exe -i C:\TestImages -o results.csv -s 256 -n 10

endlocal
