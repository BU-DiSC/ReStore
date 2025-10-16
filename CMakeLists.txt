cmake_minimum_required(VERSION 3.16)
project(ReStore VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pthread -g")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/cpp)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/..)

# Source files for the main CLI
set(CLI_SOURCES
    src/main.cpp
    src/config_manager.cpp
    src/simulator.cpp
)

# Create the main restore executable
add_executable(restore ${CLI_SOURCES})

# Link libraries
target_link_libraries(restore Threads::Threads)

# Set install target
install(TARGETS restore DESTINATION bin)

# Copy executables to build directory for runtime
set(EXECUTABLES
    Run_RL Run_LFU Run_LRU Run_LRFU Run_EXD Run_TEMP 
    Run_static Run_ideal Run_LogiReg Run_XGBfast
)

# Build Sibyl library with configurable device paths
set(SIBYL_SOURCE ${CMAKE_SOURCE_DIR}/cpp/Sibyl_driver/Sibyl_lib_configurable.c)

# Get device paths from cache or use defaults
if(NOT DEFINED FAST_DEVICE_PATH)
    set(FAST_DEVICE_PATH "/dev/nvme0n1" CACHE STRING "Path to fast storage device")
endif()
if(NOT DEFINED MIDDLE_DEVICE_PATH)
    set(MIDDLE_DEVICE_PATH "/dev/nvme1n1" CACHE STRING "Path to middle storage device")
endif()
if(NOT DEFINED SLOW_DEVICE_PATH)
    set(SLOW_DEVICE_PATH "/dev/nvme2n1" CACHE STRING "Path to slow storage device")
endif()

# Create Sibyl library with device path configuration
add_library(Sibyl_lib SHARED ${SIBYL_SOURCE})
target_compile_definitions(Sibyl_lib PRIVATE 
    FAST_DEVICE_PATH="${FAST_DEVICE_PATH}"
    MIDDLE_DEVICE_PATH="${MIDDLE_DEVICE_PATH}"
    SLOW_DEVICE_PATH="${SLOW_DEVICE_PATH}"
)
# Set the language for the Sibyl library to C
set_target_properties(Sibyl_lib PROPERTIES LINKER_LANGUAGE C)

# Build individual policy executables
foreach(EXEC ${EXECUTABLES})
    # Source files are in the cpp subdirectory
    set(SOURCE_FILE ${CMAKE_SOURCE_DIR}/cpp/${EXEC}.cpp)
    set(DEPS 
        ${CMAKE_SOURCE_DIR}/../BS_thread_pool.hpp
        ${CMAKE_SOURCE_DIR}/cpp/RL_Agent_2states.h
        ${CMAKE_SOURCE_DIR}/cpp/ReStore_driver_IO.hpp
    )
    
    add_executable(${EXEC} ${SOURCE_FILE})
    target_include_directories(${EXEC} PRIVATE ${CMAKE_SOURCE_DIR}/cpp)
    target_include_directories(${EXEC} PRIVATE ${CMAKE_SOURCE_DIR}/..)
    target_link_libraries(${EXEC} Threads::Threads Sibyl_lib)
    # Set rpath to find the library in the installation directory
    set_target_properties(${EXEC} PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    set_target_properties(${EXEC} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
    
    # Special handling for XGBfast
    if(${EXEC} STREQUAL "Run_XGBfast")
        find_library(FASTFOREST_LIB fastforest PATHS $ENV{HOME}/.local/lib64 $ENV{HOME}/.local/lib /usr/local/lib)
        if(FASTFOREST_LIB)
            target_link_libraries(${EXEC} ${FASTFOREST_LIB})
            target_include_directories(${EXEC} PRIVATE $ENV{HOME}/.local/include /usr/local/include)
        endif()
    endif()
endforeach()

# Install policy executables
install(TARGETS ${EXECUTABLES} DESTINATION bin)

# Install Sibyl library to lib directory
install(TARGETS Sibyl_lib DESTINATION lib)

# Install configuration files and data
# Note: workload and results directories are not installed by default
# Users should provide their own workload data when running experiments

# Create a package configuration
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/ReStoreConfig.cmake.in
    ${CMAKE_BINARY_DIR}/ReStoreConfig.cmake
    @ONLY
)

# Install the package configuration
install(FILES ${CMAKE_BINARY_DIR}/ReStoreConfig.cmake
        DESTINATION lib/cmake/ReStore)

# Create pkg-config file
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/restore.pc.in
    ${CMAKE_BINARY_DIR}/restore.pc
    @ONLY
)

install(FILES ${CMAKE_BINARY_DIR}/restore.pc
        DESTINATION lib/pkgconfig)

# Create shell script for easy execution
configure_file(
    ${CMAKE_SOURCE_DIR}/scripts/restore.in
    ${CMAKE_BINARY_DIR}/restore
    @ONLY
)

install(PROGRAMS ${CMAKE_BINARY_DIR}/restore
        DESTINATION bin)

# Print configuration summary
message(STATUS "ReStore Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")

