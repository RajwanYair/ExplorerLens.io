# CMakeLists.txt for LZMA SDK 24.08
# Builds static library for Windows x64

cmake_minimum_required(VERSION 3.20)
project(lzma C)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Use static runtime library
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Core LZMA source files
set(LZMA_SOURCES
    Alloc.c
    LzFind.c
    LzmaDec.c
    LzmaEnc.c
    7zFile.c
    7zStream.c
    Bcj2.c
    Bra.c
    Bra86.c
    BraIA64.c
    Delta.c
    CpuArch.c
    Lzma2Dec.c
    Lzma2Enc.c
    Ppmd7.c
    Ppmd7Dec.c
    Ppmd7Enc.c
    LzmaLib.c
    7zCrc.c
    7zCrcOpt.c
    XzCrc64.c
    XzCrc64Opt.c
    Sha256.c
    Threads.c
    MtCoder.c
    MtDec.c
    LzFindMt.c
    Lzma2DecMt.c
    7zBuf.c
    7zBuf2.c
    7zAlloc.c
)

# Add library
add_library(lzma STATIC ${LZMA_SOURCES})

# Compiler options for Release build
target_compile_options(lzma PRIVATE
    /W3
    /O2
    /GL
    /DNDEBUG
    /D_7ZIP_ST  # Single-threaded for simplicity
)

# Include directories
target_include_directories(lzma PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Installation
install(TARGETS lzma
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(FILES
    7z.h
    7zTypes.h
    LzmaDec.h
    LzmaEnc.h
    LzmaLib.h
    Lzma2Dec.h
    Lzma2Enc.h
    DESTINATION include/lzma
)
