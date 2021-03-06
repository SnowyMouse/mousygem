# More CMake stuff dammit!
cmake_minimum_required(VERSION 3.12)

add_executable(uri-test
    uri/main.cpp
)

target_include_directories(uri-test
    PRIVATE ../include
)
set_property(TARGET uri-test PROPERTY CXX_STANDARD 17)
add_test(NAME uri-test COMMAND uri-test)

target_link_libraries(uri-test mousygem)
