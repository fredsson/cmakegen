cmake_minimum_required(VERSION 3.10.0)

project(cmakegen VERSION 0.1.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  message("-- No Build type set: defaulting to Debug")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build Configuration")
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*,clang-analyzer-*,cppcoreguidelines-*,-cppcoreguidelines-pro-bounds-pointer-arithmetic)

add_library(stdc++fs UNKNOWN IMPORTED)
set_property(TARGET stdc++fs PROPERTY IMPORTED_LOCATION "/usr/lib/libstdc++fs.a")

set(SRC_FILES
  "src/impl/cmdoptionparser.cpp"
  "src/main.cpp"
)

add_executable(cmakegen ${SRC_FILES})

target_compile_features(cmakegen PRIVATE cxx_std_17)
target_compile_options(cmakegen PRIVATE -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Werror)

install(
  TARGETS cmakegen
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static
)