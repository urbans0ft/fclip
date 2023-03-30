cmake_minimum_required(VERSION 3.10)

set(TARGET fileclip CACHE STRING "The projects name." FORCE)

project(${TARGET} VERSION 2.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions(
	UNICODE
	_UNICODE
	$<$<CONFIG:DEBUG>:DEBUG>
)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic $<$<CONFIG:RELEASE>:-Werror>)
endif()

#add_executable(${TARGET} fclip.cpp pch.cpp)
add_executable(${TARGET} src/fclip.cpp src/pch.cpp)

configure_file(src/pch.h.in pch.h)
target_include_directories(${TARGET} PUBLIC "${PROJECT_BINARY_DIR}")

target_link_libraries(${TARGET} -static)
target_link_libraries(${TARGET} Ole32)
target_link_libraries(${TARGET} Shlwapi)