set(TLIBS ssl crypto dazeus-irc ${LibIRCClient_LIBRARIES})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
add_definitions("-Wall -Wextra -Wno-long-long -pedantic -DNDEBUG")

add_executable(compile ${CMAKE_CURRENT_SOURCE_DIR}/compile.cpp)
target_link_libraries(compile ${TLIBS})

add_executable(connect ${CMAKE_CURRENT_SOURCE_DIR}/connect.cpp)
target_link_libraries(connect ${TLIBS})

add_executable(reconnect ${CMAKE_CURRENT_SOURCE_DIR}/reconnect.cpp)
target_link_libraries(reconnect ${TLIBS})

add_executable(connectevents ${CMAKE_CURRENT_SOURCE_DIR}/connectevents.cpp)
target_link_libraries(connectevents ${TLIBS})

