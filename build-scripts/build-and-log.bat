@echo off
REM Build-and-Log.bat — Builds engine with vcvars64 and logs output
REM Usage: build-and-log.bat [log-filename]

set "LOG=%~1"
if "%LOG%"=="" set "LOG=build-logs\build-latest.log"

echo [%date% %time%] Build starting... > "%LOG%"
echo ============================================ >> "%LOG%"

call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.50.35717 >nul 2>&1

echo [%date% %time%] vcvars64 loaded >> "%LOG%"
cmake --build build --config Release -j 8 >> "%LOG%" 2>&1
set BUILD_EXIT=%ERRORLEVEL%

echo ============================================ >> "%LOG%"
if %BUILD_EXIT%==0 (
    echo [%date% %time%] BUILD_SUCCESS >> "%LOG%"
    echo BUILD_SUCCESS
) else (
    echo [%date% %time%] BUILD_FAILED (exit code: %BUILD_EXIT%) >> "%LOG%"
    echo BUILD_FAILED
)
exit /b %BUILD_EXIT%
