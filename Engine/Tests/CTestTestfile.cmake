# CMake generated Testfile for 
# Source directory: C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Tests
# Build directory: C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[EngineUnitTests]=] "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Tests/Debug/EngineTests.exe")
  set_tests_properties([=[EngineUnitTests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Tests/CMakeLists.txt;47;add_test;C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[EngineUnitTests]=] "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Tests/Release/EngineTests.exe")
  set_tests_properties([=[EngineUnitTests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Tests/CMakeLists.txt;47;add_test;C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[EngineUnitTests]=] "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Tests/MinSizeRel/EngineTests.exe")
  set_tests_properties([=[EngineUnitTests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Tests/CMakeLists.txt;47;add_test;C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[EngineUnitTests]=] "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Tests/RelWithDebInfo/EngineTests.exe")
  set_tests_properties([=[EngineUnitTests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Tests/CMakeLists.txt;47;add_test;C:/Users/ryair/OneDrive - Intel Corporation/Documents/MyScripts/DarkThumbs/Engine/Tests/CMakeLists.txt;0;")
else()
  add_test([=[EngineUnitTests]=] NOT_AVAILABLE)
endif()
