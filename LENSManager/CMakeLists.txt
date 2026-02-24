cmake_minimum_required(VERSION 3.20)

# LENSManager - Manager Application
project(LENSManager)

# Source files
set(SOURCES
    LENSManager.cpp
    MainDlg.cpp
    stdafx.cpp
)

set(HEADERS
    About.h
    LENSManager.h
    MainDlg.h
    RegManager.h
    resource.h
    stdafx.h
    tools.h
)

set(RESOURCES
    LENSManager.rc
)

# Create the executable
add_executable(LENSManager WIN32
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# Include directories
target_include_directories(LENSManager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WTL_INCLUDE_DIR}
)

# Preprocessor definitions
target_compile_definitions(LENSManager PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    _WINDOWS
    STRICT
)

# Link libraries
target_link_libraries(LENSManager PRIVATE
    comctl32
    ole32
    oleaut32
    uuid
    gdi32
    user32
    advapi32
    shell32
    htmlhelp
)

# Set subsystem
set_target_properties(LENSManager PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Embed manifest
if(MSVC)
    set_target_properties(LENSManager PROPERTIES
        LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:\"${CMAKE_CURRENT_SOURCE_DIR}/LENSManager.manifest\""
    )
endif()

# Installation
install(TARGETS LENSManager
    RUNTIME DESTINATION bin
)

