cmake_minimum_required (VERSION 3.5)

project (SORGHUM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(SOURCES
    src/esynth.cpp
    src/vm.cpp
    src/util.cpp
)

add_executable(scratch src/scratch.cpp ${SOURCES})
add_executable(main src/main.cpp ${SOURCES})

target_include_directories (scratch
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_include_directories (main
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
