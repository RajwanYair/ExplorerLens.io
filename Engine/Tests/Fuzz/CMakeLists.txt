cmake_minimum_required(VERSION 3.25)
project(ExplorerLensFuzz)

# Fuzz targets are only built with Clang + libFuzzer
# Enable with: cmake -DENABLE_FUZZING=ON ..
# (Automatically disabled when using MSVC)

option(ENABLE_FUZZING "Build libFuzzer fuzz targets (requires Clang + -fsanitize=fuzzer)" OFF)

if(ENABLE_FUZZING AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Building fuzz targets with libFuzzer")

    set(FUZZ_FLAGS "-fsanitize=fuzzer,address" CACHE STRING "Fuzz sanitizer flags")

    function(add_fuzz_target name source)
        add_executable(${name} ${source})
        target_compile_options(${name} PRIVATE ${FUZZ_FLAGS} -O1)
        target_link_options(${name} PRIVATE ${FUZZ_FLAGS})
        target_link_libraries(${name} PRIVATE ExplorerLensEngine)
        set_target_properties(${name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/fuzz")
    endfunction()

    add_fuzz_target(FuzzFormatDetection        FuzzFormatDetection.cpp)
    add_fuzz_target(FuzzArchiveCoverExtractor  FuzzArchiveCoverExtractor.cpp)
    add_fuzz_target(FuzzExifOrientationNormlzr FuzzExifOrientationNormalizer.cpp)

    # Corpus directories (create if absent)
    foreach(target FuzzFormatDetection FuzzArchiveCoverExtractor FuzzExifOrientationNormlzr)
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/fuzz-corpus/${target}")
    endforeach()

    message(STATUS "Fuzz targets: FuzzFormatDetection, FuzzArchiveCoverExtractor, FuzzExifOrientationNormlzr")
    message(STATUS "Run: ./fuzz/FuzzFormatDetection fuzz-corpus/FuzzFormatDetection -max_len=65536")
else()
    if(ENABLE_FUZZING AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(WARNING "ENABLE_FUZZING=ON requires Clang; fuzz targets not built with ${CMAKE_CXX_COMPILER_ID}")
    endif()
    # Create no-op stub targets so the VS solution loads without errors
    add_library(FuzzStubs OBJECT
        FuzzFormatDetection.cpp
        FuzzArchiveCoverExtractor.cpp
        FuzzExifOrientationNormalizer.cpp)
    target_link_libraries(FuzzStubs PRIVATE ExplorerLensEngine)
endif()
