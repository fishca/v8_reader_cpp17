add_library(v8reader_core STATIC)
add_library(v8reader::core ALIAS v8reader_core)

target_sources(v8reader_core PRIVATE
    src/V8Unpacker.cpp
    src/V8File.cpp
    src/V8Catalog.cpp
    src/Types.cpp
)

target_include_directories(v8reader_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# find_package(ZLIB REQUIRED)  # <-- Раскомментируете, когда начнёте работу с zlib
target_link_libraries(v8reader_core 
    # PRIVATE ZLIB::ZLIB  # <-- Временно закомментировано
)

target_compile_features(v8reader_core PUBLIC cxx_std_17)