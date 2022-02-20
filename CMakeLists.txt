cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
project(rest-client)

find_package(Boost 1.74.0 COMPONENTS program_options system REQUIRED)

include_directories(./include ./include/beast)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wpedantic -Wextra")

add_executable(rest-client src/main.cpp src/handler.cpp src/session.cpp src/helper.cpp)
target_link_libraries(rest-client Boost::program_options Boost::system -lpthread -lssl -lcrypto)
