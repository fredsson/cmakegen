cmake_minimum_required(VERSION 3.10.0)

project(cmakegen VERSION 0.1.0 LANGUAGES CXX)

option(BUILD_WITH_TIDY "BUILD_WITH_TIDY" OFF)

if(NOT CMAKE_BUILD_TYPE)
  message("-- No Build type set: defaulting to Debug")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build Configuration")
endif(NOT CMAKE_BUILD_TYPE)

if(BUILD_WITH_TIDY)
  set(CMAKE_CXX_CLANG_TIDY
    clang-tidy
    -checks=-*,readability-*,clang-analyzer-*,cppcoreguidelines-*,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-readability-implicit-bool-conversion
  )
endif(BUILD_WITH_TIDY)

set(INCLUDE_FILES
  "src/cmake/cmakefile.h"
  "src/cmake/cmakefunction.h"
  "src/cmake/cmakefunctioncriteria.h"
  "src/cmake/impl/cmakeformatter.h"
  "src/cmake/impl/cmakescanner.h"
  "src/cmake/impl/constants.h"
  "src/file_utils/directory.h"
  "src/file_utils/fileutils.h"
  "src/file_utils/ignorefile.h"
  "src/cmakegenerator.h"
  "src/cmdoptionparser.h"
  "src/projectbuilder.h"
)

set(SRC_FILES
  "src/cmake/impl/cmakescanner.cpp"
  "src/cmake/impl/cmakefunctioncriteria.cpp"
  "src/cmake/impl/cmakeformatter.cpp"
  "src/cmake/impl/cmakefunction.cpp"
  "src/cmake/impl/cmakefile.cpp"
  "src/impl/cmdoptionparser.cpp"
  "src/file_utils/impl/directory.cpp"
  "src/file_utils/impl/ignorefile.cpp"
  "src/file_utils/impl/fileutils.cpp"
  "src/impl/cmakegenerator.cpp"
  "src/impl/projectbuilder.cpp"
  "src/main.cpp"
)

add_executable(cmakegen ${INCLUDE_FILES} ${SRC_FILES})

target_compile_features(cmakegen PRIVATE cxx_std_17)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  message(STATUS "GCC|Clang detected, adding compile flags")
  target_compile_options(cmakegen
    PRIVATE
    -Wall
    -Wextra
    -Wshadow
    -Wnon-virtual-dtor
    -pedantic
    -Werror
  )
endif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

install(
  TARGETS cmakegen
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static
)
