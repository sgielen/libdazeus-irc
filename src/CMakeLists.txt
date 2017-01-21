cmake_minimum_required(VERSION 2.8)

file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

add_library(dazeus-irc ${sources} ${headers})
target_link_libraries(dazeus-irc ${LibIRCClient_LIBRARIES} ${OPENSSL_LIBRARIES})
target_include_directories(dazeus-irc SYSTEM PUBLIC ${LibIRCClient_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
set_target_properties(dazeus-irc PROPERTIES VERSION ${LIBDAZEUS_IRC_VERSION})
if(APPLE)
  set_target_properties(dazeus-irc PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif(APPLE)
add_definitions("-Wall -Wextra -pedantic")

install (TARGETS dazeus-irc DESTINATION lib)
install (FILES network.h server.h DESTINATION include)
