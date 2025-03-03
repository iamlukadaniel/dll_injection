cmake_minimum_required(VERSION 3.15)
project(MonitorHookProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB HOOKDLL_SOURCES
    "hookdll/*.cpp"
    "hookdll/*.h"
)

file(GLOB MONITOR_SOURCES
    "monitor/*.cpp"
    "monitor/*.h"
)

add_library(hookdll SHARED ${HOOKDLL_SOURCES})

add_executable(monitor ${MONITOR_SOURCES})

add_executable(test test.cpp)


