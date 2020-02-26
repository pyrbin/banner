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
    src/*.hpp
    src/*.cpp

    src/common/*.hpp
    src/common/*.cpp

    src/core/*.hpp
    src/core/*.cpp

    src/render/*.hpp
    src/render/*.cpp
)

# Executable
add_executable(tdengine ${SOURCES})

# Settings
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Copy shaders                                                    │
# └──────────────────────────────────────────────────────────────────┘
file(GLOB SHADERS ${CMAKE_SOURCE_DIR}/shaders/*.spv)

foreach(shader ${SHADERS})
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy 
                     ${shader} ${CMAKE_BINARY_DIR})
endforeach()


# ┌──────────────────────────────────────────────────────────────────┐
# │  Link externals                                                  │
# └──────────────────────────────────────────────────────────────────┘

# realm (ecs)
add_subdirectory(extern/realm)
target_link_libraries(${PROJECT_NAME} realm)

# vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})

# glfw
add_subdirectory(extern/glfw)
target_link_libraries(${PROJECT_NAME} glfw)

# glm
add_subdirectory(extern/glm)
target_link_libraries(${PROJECT_NAME} glm)