cmake_minimum_required(VERSION 3.7)
project(VoxelEngine VERSION 1.0.0)

set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING INTERNAL FORCE)

# set default build type
if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

# set default output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE}/bin)

# engine
file(GLOB_RECURSE SOURCE "Sources/*.cpp" "Sources/*.h" "Sources/*.hpp")
include_directories(${CMAKE_SOURCE_DIR}/Sources)
include_directories(${CMAKE_SOURCE_DIR}/Vendor)
add_executable(VoxelEngine ${SOURCE})
target_compile_features(VoxelEngine PRIVATE cxx_std_20)

# glfw
add_subdirectory(${CMAKE_SOURCE_DIR}/Vendor/glfw)
include_directories(${CMAKE_SOURCE_DIR}/Vendor/glfw/include)
target_link_libraries(VoxelEngine PUBLIC glfw ${GLFW_LIBRARIES})

# evk
add_subdirectory(Vendor/evk)
target_link_libraries(VoxelEngine PUBLIC evk)

# json11
file(GLOB_RECURSE JSON11_SOURCE ${CMAKE_SOURCE_DIR}/Vendor/json11/*.cpp)
add_library(json11 ${JSON11_SOURCE})
target_compile_features(json11 PRIVATE cxx_std_20)
target_link_libraries(VoxelEngine PUBLIC json11)

# wren
file(GLOB_RECURSE WREN_SOURCE ${CMAKE_SOURCE_DIR}/Vendor/wren/*.c)
include_directories(${CMAKE_SOURCE_DIR}/Vendor/wren ${CMAKE_SOURCE_DIR}/Vendor/wren/vm)
add_library(wren ${WREN_SOURCE})
target_link_libraries(VoxelEngine PUBLIC wren)

# imgui
file(GLOB_RECURSE IMGUI_SOURCE ${CMAKE_SOURCE_DIR}/Vendor/imgui/*.cpp)
include_directories(${CMAKE_SOURCE_DIR}/Vendor/imgui)
add_library(imgui ${IMGUI_SOURCE})
target_compile_features(imgui PRIVATE cxx_std_20)
target_link_libraries(imgui PUBLIC evk)
target_link_libraries(VoxelEngine PUBLIC imgui)

# fastnoise
file(GLOB_RECURSE FASTNOISE_SOURCE ${CMAKE_SOURCE_DIR}/Vendor/FastNoise/*.cpp)
add_library(fastnoise ${FASTNOISE_SOURCE})
target_compile_features(fastnoise PRIVATE cxx_std_20)
target_link_libraries(VoxelEngine PUBLIC fastnoise)

# stb
file(GLOB_RECURSE STB_SOURCE ${CMAKE_SOURCE_DIR}/Vendor/stb/*.cpp)
add_library(stb ${STB_SOURCE})
target_link_libraries(VoxelEngine PUBLIC stb)

# fmt
add_subdirectory(${CMAKE_SOURCE_DIR}/Vendor/fmt)
target_link_libraries(VoxelEngine PUBLIC fmt::fmt)