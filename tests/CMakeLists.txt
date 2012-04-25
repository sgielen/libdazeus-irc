set(TLIBS ssl crypto dazeus-irc ${LibIRCClient_LIBRARIES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
add_executable(compile ${CMAKE_CURRENT_SOURCE_DIR}/compile.cpp)
add_definitions("-Wall -Wextra -Weffc++ -Wno-long-long -pedantic -DNDEBUG")
target_link_libraries(compile ${TLIBS})
