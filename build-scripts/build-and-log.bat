@echo off
REM Build-and-Log.bat — Builds engine with vcvars64 and logs output
REM Usage: build-and-log.bat [log-filename]
REM Intermediate build files go to %TEMP%\ExplorerLens-build to avoid OneDrive sync locks.

set "LOG=%~1"
if "%LOG%"=="" set "LOG=%TEMP%\ExplorerLens-build-latest.log"

REM Ensure log directory exists
for %%F in ("%LOG%") do if not exist "%%~dpF" mkdir "%%~dpF"

echo [%date% %time%] Build starting... > "%LOG%"
echo ============================================ >> "%LOG%"

call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.50.35717 >nul 2>&1

echo [%date% %time%] vcvars64 loaded >> "%LOG%"

REM Use TEMP binary dir to avoid OneDrive .ninja_db lock
set "BUILD_DIR=%TEMP%\ExplorerLens-build"
cmake --build "%BUILD_DIR%" --config Release -j 8 >> "%LOG%" 2>&1
set BUILD_EXIT=%ERRORLEVEL%

echo ============================================ >> "%LOG%"
if %BUILD_EXIT%==0 (
    echo [%date% %time%] BUILD_SUCCESS >> "%LOG%"
    echo BUILD_SUCCESS
    echo Build log: %LOG%
) else (
    echo [%date% %time%] BUILD_FAILED (exit code: %BUILD_EXIT%) >> "%LOG%"
    echo BUILD_FAILED
    echo Build log: %LOG%
)
exit /b %BUILD_EXIT%
