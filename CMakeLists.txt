cmake_minimum_required (VERSION 3.15)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Projects Settings                                               │
# └──────────────────────────────────────────────────────────────────┘

project(banner VERSION 0.0.1 LANGUAGES CXX)

file(GLOB BANNER_PROJECT_FILES

    include/banner/util/*.hpp
    include/banner/util/*.cpp

    include/banner/core/*.hpp
    include/banner/core/*.cpp

    include/banner/gfx/vma.hpp
    include/banner/gfx/*.hpp
    include/banner/gfx/*.cpp
)

add_library(${PROJECT_NAME} STATIC ${BANNER_PROJECT_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Externals                                                       │
# └──────────────────────────────────────────────────────────────────┘

# vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})

# vma 
target_include_directories(${PROJECT_NAME} PUBLIC extern/vma/src)

# glfw
add_subdirectory(extern/glfw)
target_link_libraries(${PROJECT_NAME} glfw)

# glm
add_subdirectory(extern/glm)
target_link_libraries(${PROJECT_NAME} glm)

# realm (ecs)
target_include_directories(${PROJECT_NAME} PUBLIC extern/realm/include)

# nano-signal-slot
target_include_directories(${PROJECT_NAME} PUBLIC extern/nano-signal-slot)

# dl_libs
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})


# ┌──────────────────────────────────────────────────────────────────┐
# │  Game                                                            │
# └──────────────────────────────────────────────────────────────────┘

# Test game
add_subdirectory(game)
