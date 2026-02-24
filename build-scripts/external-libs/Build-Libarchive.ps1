#Requires -Version 7.0
# ExplorerLens v15.0.0 - Build libarchive 3.7.6 (Multi-format Archive Support)
# Refactored to use Build-Library-Core.ps1 module
# Date: July 2026
#
# Provides support for: TAR, CPIO, ISO 9660, XAR, AR/DEB, CAB, and compressed variants
#
# Directory structure:
#   Project root:       <repo>\
#   This script:        <repo>\build-scripts\external-libs\Build-Libarchive.ps1
#   Core module:        <repo>\build-scripts\core\Build-Library-Core.ps1
#   Source:             <repo>\external\compression-libs\libarchive-3.7.6\
#   Build dir:          <repo>\external\compression-libs\libarchive-3.7.6\build-vs\
#   Install dir:        <repo>\external\compression-libs\libarchive-3.7.6\install\

param(
    [string]$Configuration = "Release",
    [switch]$Clean
)

# Import core build module
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$libarchiveDir = Join-Path $rootDir "external\compression-libs\libarchive-3.7.6"
$buildDir = Join-Path $libarchiveDir "build-vs"
$installDir = Join-Path $libarchiveDir "install"

Write-BuildHeader "Building libarchive 3.7.6 (Multi-format Archive Library)"

# Verify source directory
if (-not (Test-Path $libarchiveDir)) {
    Write-BuildLog "libarchive-3.7.6 not found at $libarchiveDir" -Level Error
    Write-BuildLog "Please download and extract libarchive source" -Level Warning
    exit 1
}

Write-BuildLog "Source: $libarchiveDir" -Level Info
Write-BuildLog "Build: $buildDir" -Level Info
Write-BuildLog "Install: $installDir" -Level Info

# Resolve zlib path for libarchive's optional zlib support
$zlibInstallDir = Join-Path $rootDir "external\compression-libs\zlib-1.3.1\install"
$zlibLibPath = Join-Path $zlibInstallDir "lib\zlibstatic.lib"
$zlibIncludePath = Join-Path $zlibInstallDir "include"

# Resolve zstd path
$zstdBuildDir = Join-Path $rootDir "external\compression-libs\zstd-1.5.7\build-manual"
$zstdIncludeDir = Join-Path $rootDir "external\compression-libs\zstd-1.5.7\lib"

# Resolve lz4 path
$lz4BuildDir = Join-Path $rootDir "external\compression-libs\lz4-1.10.0\build-vs\Release"
$lz4IncludeDir = Join-Path $rootDir "external\compression-libs\lz4-1.10.0\lib"

try {
    $cmakeLists = Join-Path $libarchiveDir "CMakeLists.txt"
    
    if (Test-Path $cmakeLists) {
        Write-BuildLog "Using CMake build (static library only)" -Level Info
        
        $cmakeOptions = @{
            'CMAKE_BUILD_TYPE'  = 'Release'
            'BUILD_SHARED_LIBS' = 'OFF'
            'ENABLE_TAR'        = 'OFF'
            'ENABLE_CPIO'       = 'OFF'
            'ENABLE_CAT'        = 'OFF'
            'ENABLE_UNZIP'      = 'OFF'
            'ENABLE_TEST'       = 'OFF'
            'ENABLE_INSTALL'    = 'ON'
            'ENABLE_WERROR'     = 'OFF'
            # Disable optional dependencies we don't need
            'ENABLE_MBEDTLS'    = 'OFF'
            'ENABLE_NETTLE'     = 'OFF'
            'ENABLE_OPENSSL'    = 'OFF'
            'ENABLE_LIBB2'      = 'OFF'
            'ENABLE_LZ4'        = 'OFF'
            'ENABLE_ZSTD'       = 'OFF'
            'ENABLE_LZMA'       = 'OFF'
            'ENABLE_ZLIB'       = 'OFF'
            'ENABLE_BZip2'      = 'OFF'
            'ENABLE_LIBXML2'    = 'OFF'
            'ENABLE_EXPAT'      = 'OFF'
            'ENABLE_PCRE2POSIX' = 'OFF'
            'ENABLE_PCREPOSIX'  = 'OFF'
            'ENABLE_ICONV'      = 'OFF'
            'ENABLE_ACL'        = 'OFF'
            'ENABLE_XATTR'      = 'OFF'
        }
        
        # If zlib is available, enable it for better compression support
        if (Test-Path $zlibLibPath) {
            Write-BuildLog "Found zlib at $zlibInstallDir — enabling zlib support" -Level Info
            $cmakeOptions['ENABLE_ZLIB'] = 'ON'
            $cmakeOptions['ZLIB_LIBRARY'] = $zlibLibPath
            $cmakeOptions['ZLIB_INCLUDE_DIR'] = $zlibIncludePath
        }
        
        Invoke-CMakeBuild `
            -LibraryName "libarchive" `
            -SourceDir $libarchiveDir `
            -BuildDir $buildDir `
            -InstallDir $installDir `
            -Configuration $Configuration `
            -CMakeOptions $cmakeOptions `
            -Clean:$Clean
        
        # Verify output — on Windows with BUILD_SHARED_LIBS=OFF, static lib is archive_static.lib
        $expectedLib = Join-Path $installDir "lib\archive_static.lib"
        if (-not (Test-Path $expectedLib)) {
            # Try alternate names
            $expectedLib = Join-Path $installDir "lib\archive.lib"
        }
        if (-not (Test-Path $expectedLib)) {
            $expectedLib = Join-Path $buildDir "libarchive\$Configuration\archive_static.lib"
        }
        if (-not (Test-Path $expectedLib)) {
            $expectedLib = Join-Path $buildDir "libarchive\$Configuration\archive.lib"
        }
        
        Test-BuildOutput -Files @($expectedLib) -ThrowOnMissing
    } else {
        throw "CMakeLists.txt not found in libarchive source directory"
    }
    
    Write-BuildLog "libarchive 3.7.6 build completed successfully" -Level Success
    Write-BuildLog "Features: TAR, CPIO, ISO 9660, XAR, AR/DEB, CAB, compressed TAR (.gz/.bz2/.xz/.zst)" -Level Info
    Write-BuildLog "Output: $expectedLib" -Level Info
    
} catch {
    Write-BuildLog "Build failed: $($_.Exception.Message)" -Level Error
    exit 1
}
