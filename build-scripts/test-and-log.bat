@echo off
REM Test-and-Log.bat — Runs EngineTests.exe and logs output
REM Usage: test-and-log.bat [log-filename]
REM Intermediate build files are in %TEMP%\ExplorerLens-build (temp-release preset)

set "LOG=%~1"
if "%LOG%"=="" set "LOG=%TEMP%\ExplorerLens-test-latest.log"

REM Ensure log directory exists
for %%F in ("%LOG%") do if not exist "%%~dpF" mkdir "%%~dpF"

set "ROOT=%CD%"
set "BUILD_DIR=%TEMP%\ExplorerLens-build"

echo [%date% %time%] Test run starting... > "%LOG%"
echo ============================================ >> "%LOG%"

REM Source vcvars64 to ensure MSVC runtime DLLs are on PATH
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.50.35717 >nul 2>&1

REM Check TEMP build dir first, then fall back to local build/
set "TESTS_EXE=%BUILD_DIR%\bin\EngineTests.exe"
if not exist "%TESTS_EXE%" set "TESTS_EXE=%ROOT%\build\bin\EngineTests.exe"
if not exist "%TESTS_EXE%" (
    echo ERROR: EngineTests.exe not found in %BUILD_DIR%\bin or build\bin >> "%LOG%"
    echo TEST_FAILED - exe not found
    exit /b 1
)

for %%F in ("%TESTS_EXE%") do set "TESTS_DIR=%%~dpF"
pushd "%TESTS_DIR%"
EngineTests.exe >> "%LOG%" 2>&1
set TEST_EXIT=%ERRORLEVEL%
popd

echo ============================================ >> "%LOG%"
echo [%date% %time%] TEST_EXIT: %TEST_EXIT% >> "%LOG%"
if %TEST_EXIT%==0 (
    echo TEST_SUCCESS
    echo Test log: %LOG%
) else (
    echo TEST_FAILED
)
exit /b %TEST_EXIT%
