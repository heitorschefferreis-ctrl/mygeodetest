cmake_minimum_required(VERSION 3.25)

project(gd_pathfinder_pro)

find_package(Geode REQUIRED)

add_geode_mod(
    ID "seuusuario.gd_pathfinder_pro"
    SOURCES src/main.cpp
)