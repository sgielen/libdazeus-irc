cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

project(libdazeus-irc)

find_package(LibIRCClient REQUIRED)
include_directories(${LibIRCClient_INCLUDE_DIRS})

file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

add_library(dazeus-irc ${sources} ${headers})
target_link_libraries(dazeus-irc ${LibIRCClient_LIBRARIES})
add_definitions("-Wall -Wextra -Weffc++ -pedantic")

install (TARGETS dazeus-irc DESTINATION lib)
install (FILES network.h server.h user.h DESTINATION include)
