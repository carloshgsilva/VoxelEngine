cmake_minimum_required (VERSION 3.8)
project(evk VERSION 1.0.0 DESCRIPTION "Easy Vulkan")

set(EVK_DEBUG ON CACHE BOOL "Enables evk asserts for debugging")
if(EVK_DEBUG)
  add_compile_definitions(EVK_DEBUG)
endif()

add_library(evk "evk.cpp")

# vulkan
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
list(APPEND Vulkan_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

set(evk_INCLUDE_DIRS ${Vulkan_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(evk_LIBRARIES evk PARENT_SCOPE)