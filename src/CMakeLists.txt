cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

project(libdazeus-irc)

set( LIBDAZEUS_IRC_VERSION_MAJOR "1" )
set( LIBDAZEUS_IRC_VERSION_MINOR "0" )
set( LIBDAZEUS_IRC_VERSION "${LIBDAZEUS_IRC_VERSION_MAJOR}.${LIBDAZEUS_IRC_VERSION_MINOR}" )

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
	set(CMAKE_INSTALL_LIBDIR lib CACHE PATH "Output directory for libraries")
endif(NOT DEFINED CMAKE_INSTALL_LIBDIR)

find_package(LibIRCClient REQUIRED)
include_directories(${LibIRCClient_INCLUDE_DIRS})

file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

add_library(dazeus-irc ${sources} ${headers})
target_link_libraries(dazeus-irc ${LibIRCClient_LIBRARIES})
set_target_properties(dazeus-irc PROPERTIES VERSION ${LIBDAZEUS_IRC_VERSION})
if(APPLE)
  set_target_properties(dazeus-irc PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif(APPLE)
add_definitions("-Wall -Wextra -Weffc++ -pedantic")

install (TARGETS dazeus-irc DESTINATION lib)
install (FILES network.h server.h user.h DESTINATION include)
