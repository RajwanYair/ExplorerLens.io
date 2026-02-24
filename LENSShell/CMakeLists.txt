cmake_minimum_required(VERSION 3.20)

# LENSShell - Shell Extension DLL
project(LENSShell)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "LENSShell is x64-only. Reconfigure with a 64-bit generator/platform.")
endif()

# Source files
set(SOURCES
    LENSShell.cpp
    LENSShellClass.cpp
    StdAfx.cpp
    unzip_new.cpp
)

set(HEADERS
    LENSArchive.h
    LENSShellClass.h
    LENSShell.h
    Resource.h
    StdAfx.h
    targetver.h
    unrar.h
    unzip.h
)

set(RESOURCES
    LENSShell.rc
)

set(DEF_FILE
    LENSShell.def
)

# IDL file for COM type library
set(IDL_FILE
    LENSShell.idl
)

# Create the DLL
add_library(LENSShell SHARED
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    ${DEF_FILE}
)

# Include directories
target_include_directories(LENSShell PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WTL_INCLUDE_DIR}
)

# Preprocessor definitions
target_compile_definitions(LENSShell PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    _WINDOWS
    _USRDLL
)

# Link libraries
target_link_libraries(LENSShell PRIVATE
    shlwapi
    ole32
    oleaut32
    uuid
    comctl32
    gdi32
    gdiplus
    ${CMAKE_CURRENT_SOURCE_DIR}/unrar64.lib
)

# Module definition file
set_target_properties(LENSShell PROPERTIES
    LINK_FLAGS "/DEF:\"${CMAKE_CURRENT_SOURCE_DIR}/LENSShell.def\""
)

# Copy UnRAR DLL to output directory
add_custom_command(TARGET LENSShell POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/UnRAR64.dll"
        "$<TARGET_FILE_DIR:LENSShell>"
    COMMENT "Copying UnRAR DLL to output directory"
)

# Embed manifest
if(MSVC)
    set_target_properties(LENSShell PROPERTIES
        LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:\"${CMAKE_CURRENT_SOURCE_DIR}/LENSShell.manifest\""
    )
endif()

# Generate type library from IDL
if(MSVC)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/LENSShell.tlb
        COMMAND midl /nologo /char signed /env x64 /Oicf
                /tlb "${CMAKE_CURRENT_BINARY_DIR}/LENSShell.tlb"
                /h "${CMAKE_CURRENT_BINARY_DIR}/LENSShell_generated.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/${IDL_FILE}"
        DEPENDS ${IDL_FILE}
        COMMENT "Generating type library from ${IDL_FILE}"
    )
    add_custom_target(LENSShell_TLB DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/LENSShell.tlb)
    add_dependencies(LENSShell LENSShell_TLB)
endif()

# Installation
install(TARGETS LENSShell
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
)

install(FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/UnRAR64.dll
    DESTINATION bin
)

