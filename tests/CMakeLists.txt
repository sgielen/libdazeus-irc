include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
add_definitions("-Wall -Wextra -Wno-long-long -pedantic -DNDEBUG")

add_executable(compile ${CMAKE_CURRENT_SOURCE_DIR}/compile.cpp)
target_link_libraries(compile dazeus-irc)

add_executable(connect ${CMAKE_CURRENT_SOURCE_DIR}/connect.cpp)
target_link_libraries(connect dazeus-irc)

add_executable(reconnect ${CMAKE_CURRENT_SOURCE_DIR}/reconnect.cpp)
target_link_libraries(reconnect dazeus-irc)

add_executable(connectevents ${CMAKE_CURRENT_SOURCE_DIR}/connectevents.cpp)
target_link_libraries(connectevents dazeus-irc)

