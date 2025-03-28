cmake_minimum_required(VERSION 3.10)
project(pgutils)

find_package(PostgreSQL REQUIRED)

include_directories(${PostgreSQL_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

add_library(pgutils SHARED pg_utils.c)
target_link_libraries(pgutils PRIVATE ${PostgreSQL_LIBRARIES})