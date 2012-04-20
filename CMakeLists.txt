cmake_minimum_required(VERSION 2.8)

project(libdazeus-irc)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set( LIBDAZEUS_IRC_VERSION_MAJOR "1" )
set( LIBDAZEUS_IRC_VERSION_MINOR "0" )
set( LIBDAZEUS_IRC_VERSION "${LIBDAZEUS_IRC_VERSION_MAJOR}.${LIBDAZEUS_IRC_VERSION_MINOR}" )

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
	set(CMAKE_INSTALL_LIBDIR lib CACHE PATH "Output directory for libraries")
endif(NOT DEFINED CMAKE_INSTALL_LIBDIR)

find_package(LibIRCClient REQUIRED)
include_directories(${LibIRCClient_INCLUDE_DIRS})

add_subdirectory(src)
