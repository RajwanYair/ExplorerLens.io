cmake_minimum_required(VERSION 3.20)
project(lzma LANGUAGES C)

add_library(lzma STATIC
    Alloc.c
    CpuArch.c
    LzFind.c
    LzFindMt.c
    LzmaDec.c
    LzmaEnc.c
    LzmaLib.c
    Threads.c
)

target_include_directories(lzma PUBLIC 
    $<BUILD_INTERFACE:>
    $<INSTALL_INTERFACE:include>
)

set_target_properties(lzma PROPERTIES OUTPUT_NAME "lzma")

install(TARGETS lzma
    ARCHIVE DESTINATION lib
)

install(FILES
    LzmaLib.h
    LzmaDec.h
    LzmaEnc.h
    DESTINATION include
)
