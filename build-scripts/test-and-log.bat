@echo off
REM Test-and-Log.bat — Runs EngineTests.exe and logs output
REM Usage: test-and-log.bat [log-filename]
REM Must be run from the project root directory (where build\ lives)

set "LOG=%~1"
if "%LOG%"=="" set "LOG=build-logs\test-latest.log"

set "ROOT=%CD%"

echo [%date% %time%] Test run starting... > "%LOG%"
echo ============================================ >> "%LOG%"

if not exist "build\bin\EngineTests.exe" (
    echo ERROR: build\bin\EngineTests.exe not found >> "%LOG%"
    echo TEST_FAILED - exe not found
    exit /b 1
)

pushd "build\bin"
EngineTests.exe >> "%ROOT%\%LOG%" 2>&1
set TEST_EXIT=%ERRORLEVEL%
popd

echo ============================================ >> "%LOG%"
echo [%date% %time%] TEST_EXIT: %TEST_EXIT% >> "%LOG%"
if %TEST_EXIT%==0 (
    echo TEST_SUCCESS
) else (
    echo TEST_FAILED
)
exit /b %TEST_EXIT%
