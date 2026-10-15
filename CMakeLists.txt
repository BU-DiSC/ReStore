cmake_minimum_required(VERSION 3.16)
project(ReStore VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to an optimized build
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pthread -g")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/cpp)
include_directories(${CMAKE_SOURCE_DIR}/src)

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
# Lets `restore build` find this source tree
target_compile_definitions(restore PRIVATE RESTORE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# Set install target
install(TARGETS restore DESTINATION bin)

# Policy executables
set(EXECUTABLES
    Run_RL Run_LFU Run_LRU Run_LRFU Run_EXD Run_TEMP 
    Run_static Run_ideal Run_LogiReg
)

# The XGBfast policy needs XGBoost-FastForest (https://github.com/guitargeek/XGBoost-FastForest);
# it is skipped if the library is not installed.
find_library(FASTFOREST_LIB fastforest PATHS $ENV{HOME}/.local/lib64 $ENV{HOME}/.local/lib /usr/local/lib /usr/local/lib64)
find_path(FASTFOREST_INCLUDE_DIR fastforest.h PATHS $ENV{HOME}/.local/include /usr/local/include)
if(FASTFOREST_LIB AND FASTFOREST_INCLUDE_DIR)
    list(APPEND EXECUTABLES Run_XGBfast)
    message(STATUS "FastForest found: building Run_XGBfast")
else()
    message(WARNING "XGBoost-FastForest not found: skipping the XGBfast policy (Run_XGBfast). "
                    "Install it from https://github.com/guitargeek/XGBoost-FastForest to enable it.")
endif()

# Build the tier device driver library with configurable device paths
set(DEVICE_DRIVER_SOURCE ${CMAKE_SOURCE_DIR}/cpp/device_driver/ReStore_device.c)

# Get device paths from cache or use defaults
if(NOT DEFINED FAST_DEVICE_PATH)
    set(FAST_DEVICE_PATH "" CACHE STRING "Path to fast storage device (empty: simulated tier)")
endif()
if(NOT DEFINED MIDDLE_DEVICE_PATH)
    set(MIDDLE_DEVICE_PATH "" CACHE STRING "Path to middle storage device (empty: simulated tier)")
endif()
if(NOT DEFINED SLOW_DEVICE_PATH)
    set(SLOW_DEVICE_PATH "" CACHE STRING "Path to slow storage device (empty: simulated tier)")
endif()

# Create the device driver library with device path configuration
add_library(ReStore_device SHARED ${DEVICE_DRIVER_SOURCE})
target_compile_definitions(ReStore_device PRIVATE 
    FAST_DEVICE_PATH="${FAST_DEVICE_PATH}"
    MIDDLE_DEVICE_PATH="${MIDDLE_DEVICE_PATH}"
    SLOW_DEVICE_PATH="${SLOW_DEVICE_PATH}"
)
# Set the language for the device driver library to C
set_target_properties(ReStore_device PROPERTIES LINKER_LANGUAGE C)

# Build individual policy executables
foreach(EXEC ${EXECUTABLES})
    # Source files are in the cpp subdirectory
    set(SOURCE_FILE ${CMAKE_SOURCE_DIR}/cpp/${EXEC}.cpp)
    
    add_executable(${EXEC} ${SOURCE_FILE})
    target_include_directories(${EXEC} PRIVATE ${CMAKE_SOURCE_DIR}/cpp)
    target_link_libraries(${EXEC} Threads::Threads ReStore_device)
    # Find libReStore_device in the build tree before install, and in <prefix>/lib after install
    set_target_properties(${EXEC} PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
    
    # Special handling for XGBfast
    if(${EXEC} STREQUAL "Run_XGBfast")
        target_link_libraries(${EXEC} ${FASTFOREST_LIB})
        target_include_directories(${EXEC} PRIVATE ${FASTFOREST_INCLUDE_DIR})
    endif()
endforeach()

# Install policy executables
install(TARGETS ${EXECUTABLES} DESTINATION bin)

# Install the device driver library to lib directory
install(TARGETS ReStore_device DESTINATION lib)

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

# Print configuration summary
message(STATUS "ReStore Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Default tier devices (empty: simulated): '${FAST_DEVICE_PATH}' | '${MIDDLE_DEVICE_PATH}' | '${SLOW_DEVICE_PATH}'")

