# Install script for directory: C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/DarkThumbsEngine")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Tests/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Debug/DarkThumbsEngine.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Release/DarkThumbsEngine.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/MinSizeRel/DarkThumbsEngine.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/RelWithDebInfo/DarkThumbsEngine.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/DarkThumbs/Engine" TYPE FILE FILES
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Engine.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Core/Types.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Core/IThumbnailDecoder.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Core/IFormatDetector.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Core/IGPURenderer.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Core/ICacheProvider.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Pipeline/ThumbnailPipeline.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Pipeline/DecoderRegistry.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Pipeline/FormatDetector.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Decoders/ImageDecoder.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Decoders/WebPDecoder.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Decoders/AVIFDecoder.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Decoders/ArchiveDecoder.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/GPU/D3D11Renderer.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/GPU/GDIRenderer.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Cache/ThumbnailCache.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Utils/PerformanceProfiler.h"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
