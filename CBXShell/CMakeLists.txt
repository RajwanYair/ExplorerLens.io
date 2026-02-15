cmake_minimum_required(VERSION 3.20)

# CBXShell - Shell Extension DLL
project(CBXShell)

# Source files
set(SOURCES
    CBXShell.cpp
    CBXShellClass.cpp
    StdAfx.cpp
    unzip_new.cpp
)

set(HEADERS
    cbxArchive.h
    CBXShellClass.h
    CBXShell.h
    Resource.h
    StdAfx.h
    targetver.h
    unrar.h
    unzip.h
)

set(RESOURCES
    CBXShell.rc
)

set(DEF_FILE
    CBXShell.def
)

# IDL file for COM type library
set(IDL_FILE
    CBXShell.idl
)

# Create the DLL
add_library(CBXShell SHARED
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    ${DEF_FILE}
)

# Include directories
target_include_directories(CBXShell PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WTL_INCLUDE_DIR}
)

# Preprocessor definitions
target_compile_definitions(CBXShell PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    _WINDOWS
    _USRDLL
)

# Link libraries
target_link_libraries(CBXShell PRIVATE
    shlwapi
    ole32
    oleaut32
    uuid
    comctl32
    gdi32
    gdiplus
    ${CMAKE_CURRENT_SOURCE_DIR}/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,unrar64.lib,unrar.lib>
)

# Module definition file
set_target_properties(CBXShell PROPERTIES
    LINK_FLAGS "/DEF:\"${CMAKE_CURRENT_SOURCE_DIR}/CBXShell.def\""
)

# Copy UnRAR DLL to output directory
add_custom_command(TARGET CBXShell POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,UnRAR64.dll,UnRAR.dll>"
        "$<TARGET_FILE_DIR:CBXShell>"
    COMMENT "Copying UnRAR DLL to output directory"
)

# Embed manifest
if(MSVC)
    set_target_properties(CBXShell PROPERTIES
        LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:\"${CMAKE_CURRENT_SOURCE_DIR}/CBXShell.manifest\""
    )
endif()

# Generate type library from IDL
if(MSVC)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CBXShell.tlb
        COMMAND midl /nologo /char signed /env win32 /Oicf
                /tlb "${CMAKE_CURRENT_BINARY_DIR}/CBXShell.tlb"
                /h "${CMAKE_CURRENT_BINARY_DIR}/CBXShell_generated.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/${IDL_FILE}"
        DEPENDS ${IDL_FILE}
        COMMENT "Generating type library from ${IDL_FILE}"
    )
    add_custom_target(CBXShell_TLB DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CBXShell.tlb)
    add_dependencies(CBXShell CBXShell_TLB)
endif()

# Installation
install(TARGETS CBXShell
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
)

install(FILES 
    $<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,${CMAKE_CURRENT_SOURCE_DIR}/UnRAR64.dll,${CMAKE_CURRENT_SOURCE_DIR}/UnRAR.dll>
    DESTINATION bin
)
