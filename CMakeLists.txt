cmake_minimum_required(VERSION 3.25)

# Força o CMake a aceitar a SDK do Geode injetada via CLI
if (NOT DEFINED GEODE_MOD_ID)
    set(GEODE_MOD_ID "seuusuario.gd_pathfinder_pro")
endif()

project(${GEODE_MOD_ID})

find_package(Geode REQUIRED)

add_geode_mod(
    ID "seuusuario.gd_pathfinder_pro"
    SOURCES src/main.cpp
)