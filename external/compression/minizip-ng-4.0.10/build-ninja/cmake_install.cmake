# Install script for directory: C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/minizip-ng")
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

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/build-ninja/minizip.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip/minizip.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip/minizip.cmake"
         "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/build-ninja/CMakeFiles/Export/4fe77da0f4d2c94dd906efce3aa1c0aa/minizip.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip/minizip-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip/minizip.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip" TYPE FILE FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/build-ninja/CMakeFiles/Export/4fe77da0f4d2c94dd906efce3aa1c0aa/minizip.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip" TYPE FILE FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/build-ninja/CMakeFiles/Export/4fe77da0f4d2c94dd906efce3aa1c0aa/minizip-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip" TYPE FILE FILES
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/build-ninja/minizip-config-version.cmake"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/build-ninja/minizip-config.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/minizip" TYPE FILE FILES
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/mz.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/mz_os.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/mz_crypt.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/mz_strm.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/mz_strm_buf.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/mz_strm_mem.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/mz_strm_split.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/mz_strm_os.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/mz_zip.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/mz_zip_rw.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/mz_strm_pkcrypt.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/mz_strm_wzaes.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/compat/ioapi.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/compat/unzip.h"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/compat/zip.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/build-ninja/minizip.pc")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/build-ninja/install_local_manifest.txt"
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
  file(WRITE "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/minizip-ng-4.0.10/build-ninja/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
