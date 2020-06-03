cmake_minimum_required(VERSION 3.15)
project(Projet C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
add_executable(Projet main.c initialisation.c functions.c fonctions.h structures.h constantes.h)
target_link_libraries(Projet m)