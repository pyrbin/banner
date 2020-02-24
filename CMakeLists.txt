cmake_minimum_required (VERSION 3.15)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Projects Settings                                               │
# └──────────────────────────────────────────────────────────────────┘

project(
  tdengine
  VERSION "0.0.1"
  LANGUAGES CXX
  DESCRIPTION
  "c++ 3d game engine using glfw & vulkan"
)

file(GLOB SOURCES
    src/*.h
    src/*.cpp
)

# Executable
add_executable(tdengine ${SOURCES})

# Settings
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Link externals                                                  │
# └──────────────────────────────────────────────────────────────────┘

# realm (ecs)
target_include_directories(${PROJECT_NAME} PUBLIC extern/realm/include)

# vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})

# glfw
add_subdirectory(extern/glfw)
target_link_libraries(${PROJECT_NAME} glfw)