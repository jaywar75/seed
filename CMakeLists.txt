cmake_minimum_required(VERSION 3.10)
project(seed LANGUAGES CXX)

# Set C++ standard.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the Vulkan package (ensure the Vulkan SDK is installed and environment variables set).
find_package(Vulkan REQUIRED)

# Include directories.
include_directories(${CMAKE_SOURCE_DIR}/include)

# Build the encryption shared library.
add_library(encrypt SHARED src/encrypt.cpp)
set_target_properties(encrypt PROPERTIES OUTPUT_NAME "encrypt")

# Build the main executable (includes the Vulkan-based GPU module).
add_executable(seed_app src/main.cpp src/gpu_app.cpp)
target_link_libraries(seed_app encrypt Vulkan::Vulkan)