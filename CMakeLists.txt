cmake_minimum_required (VERSION 3.15)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Projects Settings                                               │
# └──────────────────────────────────────────────────────────────────┘

project(banner VERSION 0.0.1 LANGUAGES CXX)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Engine                                                          │
# └──────────────────────────────────────────────────────────────────┘

add_library(${PROJECT_NAME}
	include/engine/util/debug.hpp
	include/engine/util/debug.cpp
)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Externals                                                       │
# └──────────────────────────────────────────────────────────────────┘

# realm (ecs)
target_include_directories(${PROJECT_NAME} PUBLIC extern/realm/include)

# vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})

# VulkanMemoryAllocator
target_include_directories(${PROJECT_NAME} PUBLIC
	extern/VulkanMemoryAllocator/src
)

# glfw
add_subdirectory(extern/glfw)
target_link_libraries(${PROJECT_NAME} glfw)

# glm
add_subdirectory(extern/glm)
target_link_libraries(${PROJECT_NAME} glm)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Game                                                            │
# └──────────────────────────────────────────────────────────────────┘

# Test game
add_subdirectory(game)
