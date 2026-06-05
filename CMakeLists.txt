cmake_minimum_required(VERSION 3.12)

project(pathfinder-route-assistant VERSION 1.0.0 LANGUAGES CXX)

find_package(geode REQUIRED)

add_geode_mod(
    pathfinder-route-assistant
    GD_VERSION 2.204
    SOURCES main.cpp
)

target_compile_definitions(pathfinder-route-assistant PRIVATE GEODE_MOD_ID="pathfinder-route-assistant")