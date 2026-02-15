cmake_minimum_required(VERSION 3.20)

# CBXManager - Manager Application
project(CBXManager)

# Source files
set(SOURCES
    CBXManager.cpp
    MainDlg.cpp
    stdafx.cpp
)

set(HEADERS
    About.h
    CBXManager.h
    MainDlg.h
    RegManager.h
    resource.h
    stdafx.h
    tools.h
)

set(RESOURCES
    CBXManager.rc
)

# Create the executable
add_executable(CBXManager WIN32
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# Include directories
target_include_directories(CBXManager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WTL_INCLUDE_DIR}
)

# Preprocessor definitions
target_compile_definitions(CBXManager PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    _WINDOWS
    STRICT
)

# Link libraries
target_link_libraries(CBXManager PRIVATE
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
set_target_properties(CBXManager PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Embed manifest
if(MSVC)
    set_target_properties(CBXManager PROPERTIES
        LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:\"${CMAKE_CURRENT_SOURCE_DIR}/CBXManager.manifest\""
    )
endif()

# Installation
install(TARGETS CBXManager
    RUNTIME DESTINATION bin
)
