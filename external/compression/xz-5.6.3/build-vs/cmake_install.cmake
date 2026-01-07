# Install script for directory: C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/xz")
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

if(CMAKE_INSTALL_COMPONENT STREQUAL "liblzma_Development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/build-vs/lzma.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "liblzma_Development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/src/liblzma/api/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "liblzma_Development" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/liblzma/liblzma-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/liblzma/liblzma-targets.cmake"
         "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/build-vs/CMakeFiles/Export/6194817f435e7429e84a3ab7f926109c/liblzma-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/liblzma/liblzma-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/liblzma/liblzma-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/liblzma" TYPE FILE FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/build-vs/CMakeFiles/Export/6194817f435e7429e84a3ab7f926109c/liblzma-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/liblzma" TYPE FILE FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/build-vs/CMakeFiles/Export/6194817f435e7429e84a3ab7f926109c/liblzma-targets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "liblzma_Development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/liblzma" TYPE FILE FILES
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/build-vs/liblzma-config.cmake"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/build-vs/liblzma-config-version.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "liblzma_Development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/build-vs/liblzma.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "xzdec_Runtime" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/build-vs/xzdec.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lzmadec_Runtime" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/build-vs/lzmadec.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lzmainfo_Runtime" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/build-vs/lzmainfo.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "xz_Runtime" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/build-vs/xz.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "liblzma_Documentation" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/xz" TYPE DIRECTORY FILES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/doc/examples")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Documentation" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/xz" TYPE FILE FILES
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/AUTHORS"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/COPYING"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/COPYING.0BSD"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/COPYING.GPLv2"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/NEWS"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/README"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/THANKS"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/doc/faq.txt"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/doc/history.txt"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/doc/lzma-file-format.txt"
    "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/doc/xz-file-format.txt"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/build-vs/install_local_manifest.txt"
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
  file(WRITE "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/external/compression/xz-5.6.3/build-vs/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
