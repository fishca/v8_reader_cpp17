find_package(Qt6 REQUIRED COMPONENTS Widgets)
add_executable(v8_reader WIN32)
target_sources(v8_reader PRIVATE main.cpp)
target_link_libraries(v8_reader PRIVATE v8reader::core v8reader::ui Qt6::Widgets)
target_compile_features(v8_reader PRIVATE cxx_std_17)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(v8_reader PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()