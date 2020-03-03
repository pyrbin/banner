cmake_minimum_required (VERSION 3.15)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Projects Settings                                               │
# └──────────────────────────────────────────────────────────────────┘

project(game)

# engine
add_executable(game main.cpp)
target_link_libraries(game PUBLIC banner)

add_executable(test test.cpp)
target_link_libraries(test PUBLIC banner)

add_executable(clear clear.cpp)
target_link_libraries(clear PUBLIC banner)

add_executable(triangle triangle.cpp)
target_link_libraries(triangle PUBLIC banner)