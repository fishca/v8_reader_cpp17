find_package(GTest REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(v8reader_tests 
    core/V8File_test.cpp
    core/test_metadata_classes.cpp
)

target_link_libraries(v8reader_tests PRIVATE 
    v8reader::core 
    Qt6::Test
    GTest::gtest_main
)

target_compile_features(v8reader_tests PRIVATE cxx_std_17)
target_include_directories(v8reader_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src/core/include
)

add_test(NAME v8reader_tests COMMAND v8reader_tests)