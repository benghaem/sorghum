# Build output setup
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

set(SOURCES
    esynth.cpp
    vm.cpp
    util.cpp
    synth.cpp
    mcmc_synth.cpp
    para.cpp
    input.cpp
)

find_package (Threads REQUIRED)

#add_executable(scratch scratch.cpp)
add_executable(sorghum main.cpp)
add_library(sorghum_lib ${SOURCES})

#target_link_libraries(scratch Threads::Threads)
target_link_libraries(sorghum Threads::Threads)
#target_link_libraries(scratch sorghum_lib)
target_link_libraries(sorghum sorghum_lib)

#set_target_properties(scratch sorghum PROPERTIES CXX_STANDARD 17)
set_target_properties(sorghum PROPERTIES CXX_STANDARD 17)

#target_include_directories (scratch
#    PRIVATE
#    ${PROJECT_SOURCE_DIR}/include
#)

target_include_directories (sorghum
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_include_directories (sorghum_lib
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)


