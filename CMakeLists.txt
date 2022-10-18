cmake_minimum_required(VERSION 3.10)

project(fclip)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions(UNICODE _UNICODE)

add_executable(fclip fclip.cpp pch.cpp)

target_link_libraries(fclip Ole32)
target_link_libraries(fclip Shlwapi)