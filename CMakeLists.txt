cmake_minimum_required (VERSION 3.7)
project ("VoxelEngine")

set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING INTERNAL FORCE)

# set default build type
if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

# set default output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

# engine
file(GLOB_RECURSE SOURCE "Sources/*.cpp" "Sources/*.h" "Sources/*.hpp")
include_directories(${CMAKE_SOURCE_DIR}/Sources)
include_directories(${CMAKE_SOURCE_DIR}/Vendor)
add_executable(${PROJECT_NAME} ${SOURCE})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# glfw
add_subdirectory(${CMAKE_SOURCE_DIR}/Vendor/glfw)
include_directories(${CMAKE_SOURCE_DIR}/Vendor/glfw/include)
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES})

# vulkan
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})

# vma
file(GLOB_RECURSE VMA_SOURCE ${CMAKE_SOURCE_DIR}/Vendor/vma/*.cpp)
add_library(vma ${VMA_SOURCE})
target_link_libraries(${PROJECT_NAME} vma)

# evk
add_library(evk ${CMAKE_SOURCE_DIR}/Vendor/evk/evk.cpp)
target_link_libraries(${PROJECT_NAME} evk)

# json11
file(GLOB_RECURSE JSON11_SOURCE ${CMAKE_SOURCE_DIR}/Vendor/json11/*.cpp)
add_library(json11 ${JSON11_SOURCE})
target_link_libraries(${PROJECT_NAME} json11)

# wren
file(GLOB_RECURSE WREN_SOURCE ${CMAKE_SOURCE_DIR}/Vendor/wren/*.c)
include_directories(${CMAKE_SOURCE_DIR}/Vendor/wren ${CMAKE_SOURCE_DIR}/Vendor/wren/vm)
add_library(wren ${WREN_SOURCE})
target_link_libraries(${PROJECT_NAME} wren)

# imgui
file(GLOB_RECURSE IMGUI_SOURCE ${CMAKE_SOURCE_DIR}/Vendor/imgui/*.cpp)
include_directories(${CMAKE_SOURCE_DIR}/Vendor/imgui)
add_library(imgui ${IMGUI_SOURCE})
target_link_libraries(${PROJECT_NAME} imgui)

# fastnoise
file(GLOB_RECURSE FASTNOISE_SOURCE ${CMAKE_SOURCE_DIR}/Vendor/FastNoise/*.cpp)
add_library(fastnoise ${FASTNOISE_SOURCE})
target_link_libraries(${PROJECT_NAME} fastnoise)

# stb
file(GLOB_RECURSE STB_SOURCE ${CMAKE_SOURCE_DIR}/Vendor/stb/*.cpp)
add_library(stb ${STB_SOURCE})
target_link_libraries(${PROJECT_NAME} stb)

# spdlog
include_directories(${CMAKE_SOURCE_DIR}/Vendor/spdlog/include)

# shaders
#file(GLOB GLSL_SOURCE "Sources/Shaders/*.vert" "Sources/Shaders/*.frag" "Sources/Shaders/*.comp")
#
#foreach(GLSL ${GLSL_SOURCE})
#  get_filename_component(FILE_NAME ${GLSL} NAME)
#  set(SPIRV "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets/Mods/default/Shaders/${FILE_NAME}.spv")
#
#  if(NOT SPIRV MATCHES "Lib.*")
#    add_custom_command(
#      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/Sources/Shaders
#      OUTPUT ${SPIRV}
#      COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -I${CMAKE_SOURCE_DIR}/Sources/Shaders -std=450 -o ${SPIRV}
#      DEPENDS ${GLSL}
#      VERBATIM )
#    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
#  endif()
#endforeach(GLSL)
#add_custom_target(shaders DEPENDS ${SPIRV_BINARY_FILES})
#add_dependencies(${PROJECT_NAME} shaders)
#
#set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")