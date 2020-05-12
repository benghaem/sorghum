cmake_minimum_required (VERSION 3.11)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#Project Name
project (SORGHUM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

project (SORGHUM_TEST)

add_subdirectory(test)
enable_testing()
add_test(SORGHUM_TEST SORGHUM_TEST)

