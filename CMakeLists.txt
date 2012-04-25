cmake_minimum_required(VERSION 2.8)

project(libdazeus-irc)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set( LIBDAZEUS_IRC_VERSION_MAJOR "1" )
set( LIBDAZEUS_IRC_VERSION_MINOR "0" )
set( LIBDAZEUS_IRC_VERSION "${LIBDAZEUS_IRC_VERSION_MAJOR}.${LIBDAZEUS_IRC_VERSION_MINOR}" )

SET(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib CACHE PATH "The place where the library and pkgconfig files will be stored")
SET(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include CACHE PATH "The place where the header files will be stored")

find_package(LibIRCClient REQUIRED)
include_directories(${LibIRCClient_INCLUDE_DIRS})

add_subdirectory(src)
add_subdirectory(tests)

# pkg-config file
configure_file(libdazeus-irc.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libdazeus-irc.pc)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/libdazeus-irc.pc
         DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

enable_testing()
add_test(compile tests/compile)
add_test(connect ${CMAKE_SOURCE_DIR}/tests/connect.pl tests/connect)
