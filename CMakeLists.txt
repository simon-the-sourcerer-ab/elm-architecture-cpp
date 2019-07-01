cmake_minimum_required(VERSION 3.0)
project(elm-architecture VERSION 1.0.0 LANGUAGES CXX)

option(EA_BUILD_EXAMPLE "Build the example application" OFF)

find_package(Threads REQUIRED)

add_library(elm-architecture INTERFACE)
target_include_directories(elm-architecture INTERFACE $<INSTALL_INTERFACE:include> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_features(elm-architecture INTERFACE cxx_std_17)
target_link_libraries(elm-architecture INTERFACE Threads::Threads)

if(EA_BUILD_EXAMPLE)
  add_executable(elm-architecture-example example.cpp)
  target_link_libraries(elm-architecture-example elm-architecture)
endif()
