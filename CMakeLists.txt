cmake_minimum_required(VERSION 3.8)
project(chat)

set(CMAKE_CXX_STANDARD 11)

include_directories(../libsocket/headers)


set(SOURCE_FILES main.cpp libsocket.h libsocket.cpp ISocket.h MySocket.h MySocket.cpp helper.h)
add_executable(chat ${SOURCE_FILES})
target_link_libraries(chat libsocket++.dylib)
