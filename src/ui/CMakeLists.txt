cmake_minimum_required(VERSION 3.20)

# 🔑 КРИТИЧЕСКИ ВАЖНО: Включаем автоматическую обработку MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)  # Для ресурсов .qrc
set(CMAKE_AUTOUIC ON)  # Для .ui файлов (если будут)

find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)

add_library(v8reader_ui STATIC)
add_library(v8reader::ui ALIAS v8reader_ui)

# 🔑 Все заголовки с Q_OBJECT должны быть в target_sources!
target_sources(v8reader_ui PRIVATE
    # Реализации (.cpp)
    src/MainWindow.cpp
    src/MetadataTree.cpp
    src/ContentPane.cpp
    
    # 🔥 Заголовки с макросом Q_OBJECT (для AUTOMOC)
    include/v8reader/ui/MainWindow.h
    include/v8reader/ui/MetadataTree.h
    include/v8reader/ui/ContentPane.h
    
    # Ресурсы
    resources.qrc
)

target_include_directories(v8reader_ui
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(v8reader_ui 
    PUBLIC Qt6::Widgets Qt6::Concurrent
    PRIVATE v8reader::core
)

target_compile_features(v8reader_ui PUBLIC cxx_std_17)

# Подавление предупреждений MSVC для Qt-заголовков
if(MSVC)
    target_compile_definitions(v8reader_ui PRIVATE
        _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
        _CRT_SECURE_NO_WARNINGS
    )
endif()
