# src/core/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

# 1. Сначала находим зависимости
find_package(ZLIB REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core)

# 2. Создаём таргет
add_library(v8reader_core STATIC)
add_library(v8reader::core ALIAS v8reader_core)

# 3. Добавляем источники (проверьте пути!)
target_sources(v8reader_core PRIVATE
    src/V8Unpacker.cpp
    src/V8Container.cpp
    src/TreeReader.cpp
    src/V8Element.cpp
    src/V8Format.cpp
    src/Guid.cpp
    src/Types.cpp
    src/V8File.cpp
    src/V8Catalog.cpp
    src/metadata/TMDO.cpp
    src/metadata/TRequisite.cpp
    src/metadata/t_comand.cpp
    src/metadata/t_metadata_object_with_sections.cpp
    src/metadata/t_tabular.cpp
    src/metadata/t_catalog.cpp
    src/metadata/sections/section_requisites.cpp
    src/metadata/sections/section_commands.cpp
)

# 4. Настраиваем include-пути
target_include_directories(v8reader_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 5. Линкуем зависимости
target_link_libraries(v8reader_core
    PUBLIC
        Qt6::Core
    PRIVATE
        ZLIB::ZLIB
)

# 6. Стандарт C++17 и подавление предупреждений MSVC
target_compile_features(v8reader_core PUBLIC cxx_std_17)

if(MSVC)
    target_compile_definitions(v8reader_core PRIVATE
        _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
        _CRT_SECURE_NO_WARNINGS
    )
endif()
