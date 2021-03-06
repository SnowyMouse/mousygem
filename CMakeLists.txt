# CMake stuff - Get this stuff out of the way
cmake_minimum_required(VERSION 3.12)
project(mousygem
    VERSION 0.1
    DESCRIPTION "Simple C++ Gemini server library"
)

# Here's our library
add_library(mousygem
    src/client.cpp
    src/socket.cpp
    src/server.cpp
    src/ssl_context.cpp
    src/uri.cpp
)

# Add include directories
target_include_directories(mousygem
    PUBLIC include/
)

target_link_libraries(mousygem ssl crypto pthread)

# Use C++17 (std::filesystem among other things)
set_property(TARGET mousygem PROPERTY CXX_STANDARD 17)

# Let's do some unit testing!
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
    
    if(BUILD_TESTING)
        add_subdirectory(test)
    endif()
endif()
