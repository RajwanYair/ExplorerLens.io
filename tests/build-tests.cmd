# ExplorerLens Unit Test Suite
# Build and run tests with MSVC

@echo off
setlocal enabledelayedexpansion

echo ==========================================
echo ExplorerLens Unit Test Build Script
echo ==========================================
echo.

REM Check for MSVC compiler
where cl.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] MSVC compiler not found!
    echo.
    echo Please run this script from:
    echo   - Visual Studio Developer Command Prompt, or
    echo   - After running vcvarsall.bat
    echo.
    echo Example:
    echo   "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
    echo.
    pause
    exit /b 1
)

echo [1/4] Checking MSVC version...
cl.exe 2>&1 | findstr /C:"Version" 
echo.

echo [2/4] Creating build directory...
if not exist "tests\build" mkdir "tests\build"
cd tests\build
echo   Created: tests\build\
echo.

echo [3/4] Compiling unit tests...
echo   Compiler: cl.exe
echo   Standard: C++17
echo   Config:   Debug with symbols

cl.exe /nologo /std:c++17 /EHsc /W3 /Zi /D_UNICODE /DUNICODE ^
    /I"..\..\LENSShell" ^
    /I"..\..\external\wtl\Include" ^
    ..\UnitTests.cpp ^
    /link shlwapi.lib ^
    /OUT:UnitTests.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Compilation failed!
    cd ..\..
    pause
    exit /b 1
)

echo.
echo   Compiled: UnitTests.exe
echo.

echo [4/4] Running unit tests...
echo ==========================================
echo.

UnitTests.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo ==========================================
echo Build and Test Complete
echo ==========================================

cd ..\..

if %TEST_RESULT% EQU 0 (
    echo Result: SUCCESS - All tests passed
    exit /b 0
) else (
    echo Result: FAILURE - Some tests failed
    exit /b 1
)

