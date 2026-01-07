# DarkThumbs Master Test Runner
# Runs all test suites and generates summary report

@echo off
setlocal enabledelayedexpansion

echo ==========================================
echo DarkThumbs Complete Test Suite
echo ==========================================
echo.

REM Initialize counters
set TOTAL_SUITES=0
set PASSED_SUITES=0
set FAILED_SUITES=0

REM Check for test data
echo [Phase 1/4] Checking test data...
if not exist "test_data\test_comic.cbz" (
    echo   Test data not found. Generating...
    python generate_test_data.py
    if %ERRORLEVEL% NEQ 0 (
        echo   [WARNING] Test data generation failed
        echo   Some integration tests may be skipped
    ) else (
        echo   Test data generated successfully
    )
) else (
    echo   Test data found
)
echo.

REM Check for compiler
echo [Phase 2/4] Checking build environment...
where cl.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo   [ERROR] MSVC compiler not found!
    echo.
    echo   Please run from Visual Studio Developer Command Prompt
    echo   or initialize build environment:
    echo.
    echo   "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
    echo.
    pause
    exit /b 1
)
echo   Compiler found: cl.exe
echo.

REM Build tests
echo [Phase 3/4] Building test suites...
if not exist "build" mkdir build
cd build

echo   [1/2] Compiling UnitTests.cpp...
cl.exe /nologo /std:c++17 /EHsc /W3 /Zi /D_UNICODE /DUNICODE ^
    /I"..\..\CBXShell" ^
    ..\UnitTests.cpp ^
    /link shlwapi.lib ^
    /OUT:UnitTests.exe >nul 2>&1

if %ERRORLEVEL% NEQ 0 (
    echo   [ERROR] Unit test compilation failed
    cd ..
    pause
    exit /b 1
)
echo   Compiled: UnitTests.exe

echo   [2/2] Compiling IntegrationTests.cpp...
cl.exe /nologo /std:c++17 /EHsc /W3 /Zi /D_UNICODE /DUNICODE ^
    ..\IntegrationTests.cpp ^
    /link shlwapi.lib ole32.lib ^
    /OUT:IntegrationTests.exe >nul 2>&1

if %ERRORLEVEL% NEQ 0 (
    echo   [ERROR] Integration test compilation failed
    cd ..
    pause
    exit /b 1
)
echo   Compiled: IntegrationTests.exe
echo.

REM Run tests
echo [Phase 4/4] Running test suites...
echo ==========================================
echo.

REM Run unit tests
set /a TOTAL_SUITES+=1
echo [Suite 1/2] Unit Tests
echo ------------------------------------------
UnitTests.exe
if %ERRORLEVEL% EQU 0 (
    set /a PASSED_SUITES+=1
    echo.
    echo Result: PASSED
) else (
    set /a FAILED_SUITES+=1
    echo.
    echo Result: FAILED
)
echo.

REM Run integration tests
set /a TOTAL_SUITES+=1
echo [Suite 2/2] Integration Tests
echo ------------------------------------------
IntegrationTests.exe
if %ERRORLEVEL% EQU 0 (
    set /a PASSED_SUITES+=1
    echo.
    echo Result: PASSED
) else (
    set /a FAILED_SUITES+=1
    echo.
    echo Result: FAILED
)
echo.

cd ..

REM Print summary
echo ==========================================
echo Overall Test Summary
echo ==========================================
echo Total Suites:  %TOTAL_SUITES%
echo Passed:        %PASSED_SUITES%
echo Failed:        %FAILED_SUITES%
echo.

if %FAILED_SUITES% EQU 0 (
    echo *** ALL TEST SUITES PASSED ***
    echo.
    echo The codebase is stable and ready for:
    echo   - Code migration
    echo   - Toolchain changes
    echo   - Platform updates
    echo   - Refactoring
    echo ==========================================
    exit /b 0
) else (
    echo !!! SOME TEST SUITES FAILED !!!
    echo.
    echo Please review failed tests before:
    echo   - Committing changes
    echo   - Migrating code
    echo   - Deploying builds
    echo ==========================================
    exit /b 1
)
