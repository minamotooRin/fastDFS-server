cmake_minimum_required(VERSION 3.4.1)
project(fileCacheProxy)

# SET(DEBUG_MODE TRUE)

if(DEBUG_MODE)
    message(STATUS "===Compiling in DEBUG mode...===" )
    add_definitions(-DDEBUG)
    add_definitions("-Wall -std=c++17 -O0 -g") # 编译选项
else()
    message(STATUS "===Compiling in Release mode...===" )
    add_definitions("-Wall -std=c++17 -O3") # 编译选项
endif()

# 打印信息
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/event2
    ${CMAKE_CURRENT_SOURCE_DIR}/include/spdlog
)
link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/lib SRC_LIST)
add_library(SPH SHARED
    ${SRC_LIST}
)

add_executable(main
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(main fdfsclient fastcommon event-2.0 pthread)