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
    src/synth.cpp
    src/mcmc_synth.cpp
    src/para.cpp
)

find_package (Threads REQUIRED)

add_executable(scratch src/scratch.cpp ${SOURCES})
add_executable(main src/main.cpp ${SOURCES})

target_link_libraries(scratch Threads::Threads)
target_link_libraries(main Threads::Threads)

set_target_properties(scratch main PROPERTIES CXX_STANDARD 17)

target_include_directories (scratch
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_include_directories (main
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
