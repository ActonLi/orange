cmake_minimum_required(VERSION 3.2)

project(libelf)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(LibelfMacros)

# always include project's folder to includes
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

include(configure.cmake)

include_directories(${CMAKE_BINARY_DIR})

add_subdirectory(lib)
if(UNIX)
	add_subdirectory(po)
endif()