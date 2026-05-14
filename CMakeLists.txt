cmake_minimum_required(VERSION 3.21)
project(v8_reader_cpp17 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt автоматизация
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Интеграция vcpkg
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

option(V8_READER_BUILD_TESTS "Build unit tests" ON)
option(V8_READER_BUILD_CLI "Build CLI tool" OFF)

add_subdirectory(src/core)
add_subdirectory(src/ui)
add_subdirectory(src/app)

if(V8_READER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()