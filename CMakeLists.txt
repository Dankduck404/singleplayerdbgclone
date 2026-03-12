cmake_minimum_required(VERSION 3.16)
project(PlanetVegeta)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch Raylib automatically
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

add_executable(PlanetVegeta
    main.cpp
    menu.cpp
    player.cpp
    world.cpp
)

target_link_libraries(PlanetVegeta PRIVATE raylib)
