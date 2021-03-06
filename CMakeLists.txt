# CMake stuff - Get this stuff out of the way
cmake_minimum_required(VERSION 3.12)
project(mousygem
    VERSION 0.1
    DESCRIPTION "Simple C++ Gemini server library"
)

# Here's our library
add_library(mousygem
    src/uri.cpp
)

# Use C++17 (std::filesystem among other things)
set_property(TARGET mousygem PROPERTY CXX_STANDARD 17)
