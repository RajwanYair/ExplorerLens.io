@echo off
echo === Building liblzma (XZ 5.6.3) ===
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to setup Visual Studio environment
    exit /b 1
)

cd /d "C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\DarkThumbs\external\compression\xz-5.6.3\build-vs"

echo.
echo Configuring with CMake...
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS_RELEASE="/MT /O2 /DNDEBUG" -DBUILD_SHARED_LIBS=OFF -DENABLE_NLS=OFF
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed
    exit /b %ERRORLEVEL%
)

echo.
echo Building with nmake...
nmake
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    exit /b %ERRORLEVEL%
)

echo.
echo Build completed successfully!
dir /s *.lib
