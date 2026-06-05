cmake_minimum_required(VERSION 3.25)

# 1. Provide a default fallback if GEODE_SDK is not set (CRITICAL for GitHub Actions)
if(NOT DEFINED GEODE_SDK)
    set(GEODE_SDK $ENV{GEODE_SDK})
endif()

project(pathfinder-route-assistant)

# 2. This line links the Geode build macros
include(${GEODE_SDK}/cmake/Geode.cmake)

# 3. NOW you can find the package safely
find_package(geode REQUIRED)

add_geode_mod(
    pathfinder-route-assistant
    GD_VERSION 2.204
    SOURCES main.cpp
)

target_compile_definitions(pathfinder-route-assistant PRIVATE GEODE_MOD_ID="pathfinder-route-assistant")