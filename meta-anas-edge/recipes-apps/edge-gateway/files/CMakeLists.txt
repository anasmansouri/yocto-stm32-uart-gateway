cmake_minimum_required(VERSION 3.10)

project(edge-gateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler warnings 
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -ftemplate-backtrace-limit=0 -fdiagnostics-color=always)
endif()


set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

# Utils module 
add_library(edge_utils 
    utils/Result.hpp utils/Result.cpp
)
target_include_directories(
    edge_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

# Manager module 
add_library(edge_manager 
    manager/uartManager.cpp manager/uartManager.hpp 
    manager/protocolParser.cpp manager/protocolParser.hpp 
    manager/ipcServer.cpp manager/ipcServer.hpp
)
target_include_directories(
    edge_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(edge_manager PUBLIC edge_utils)

# Main excutable
add_executable(edge-gateway main.cpp)

target_link_libraries(edge-gateway PUBLIC edge_manager edge_utils)

install(TARGETS edge-gateway
        RUNTIME DESTINATION bin)
