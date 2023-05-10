cmake_minimum_required(VERSION 3.21)
project(GLAD)

set(GLAD_SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(GLAD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

include_directories(include ${GLAD_INCLUDE_DIR})

add_library(glad)
target_sources(glad PRIVATE ${GLAD_SRC_DIR}/glad.c)
target_include_directories(glad PUBLIC include)