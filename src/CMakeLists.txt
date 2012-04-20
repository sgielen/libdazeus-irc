cmake_minimum_required(VERSION 2.8)

#For our own CMake modules!
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

project(libdazeus-irc)

find_package(LibIRCClient REQUIRED)
include_directories(${LibIRCClient_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LibIRCClient_LIBRARIES})

file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

add_library(dazeus-irc ${sources} ${headers})
add_definitions("-Wall -Wextra -Weffc++ -pedantic")
