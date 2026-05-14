find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)

add_library(v8reader_ui STATIC)
add_library(v8reader::ui ALIAS v8reader_ui)

target_sources(v8reader_ui PRIVATE
    # Реализации
    src/MainWindow.cpp
    src/MetadataTree.cpp
    src/ContentPane.cpp
    
    # 🔑 КРИТИЧЕСКИ ВАЖНО: заголовки с Q_OBJECT должны быть в target_sources для AUTOMOC
    include/v8reader/ui/MainWindow.h
    include/v8reader/ui/MetadataTree.h
    include/v8reader/ui/ContentPane.h
    
    resources.qrc
)

target_include_directories(v8reader_ui
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(v8reader_ui PUBLIC Qt6::Widgets Qt6::Concurrent v8reader::core)
target_compile_features(v8reader_ui PUBLIC cxx_std_17)