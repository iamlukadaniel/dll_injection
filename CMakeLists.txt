cmake_minimum_required(VERSION 3.15)
project(MonitorHookProject)

enable_language(C ASM_MASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CAPSTONE_ROOT "${CMAKE_SOURCE_DIR}/capstone")
include_directories("${CAPSTONE_ROOT}/include")
link_directories("${CAPSTONE_ROOT}/build/Release")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB HOOKDLL_SOURCES
    "hookdll/*.cpp"
    "hookdll/FileHider/*.cpp"
    "hookdll/FileHider/*.h"
    "hookdll/HookHandlers/*.cpp"
    "hookdll/HookHandlers/*.h"
    "hookdll/HookPatch/*.cpp"
    "hookdll/HookPatch/*.h"
    "hookdll/HookManager/*.cpp"
    "hookdll/HookManager/*.h"
    "hookdll/UnifiedHookHandler/*.asm"
    "hookdll/UnifiedHookHandler/*.h"
    "common/*.cpp"
    "common/*.h"
)

file(GLOB MONITOR_SOURCES
    "monitor/*.cpp"
    "monitor/Monitor/*.cpp"
    "monitor/Monitor/*.h"
    "monitor/ProcessManager/*.cpp"
    "monitor/ProcessManager/*.h"
    "common/*.cpp"
    "common/*.h"
)

add_library(hookdll SHARED ${HOOKDLL_SOURCES}  )

target_link_libraries(hookdll PRIVATE capstone)

add_executable(monitor ${MONITOR_SOURCES})

add_executable(test test.cpp)

set_source_files_properties(hookdll/UnifiedHookHandler.asm PROPERTIES LANGUAGE ASM_MASM)
