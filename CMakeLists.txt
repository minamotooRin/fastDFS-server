cmake_minimum_required(VERSION 3.4.1)
project(fileCacheProxy)

SET(DEBUG_MODE TRUE)

if(DEBUG_MODE)
    message(STATUS "===Compiling in DEBUG mode...===" )
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

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LIST)
list(REMOVE_ITEM SRC_LIST "${CMAKE_CURRENT_SOURCE_DIR}/src/expireCleaner.cpp")
add_executable(main
    ${SRC_LIST}
)

IF (WIN32)
	MESSAGE(STATUS "Now is windows")
ELSEIF (APPLE)
	MESSAGE(STATUS "Now is Apple systens.")
ELSEIF (UNIX)
	MESSAGE(STATUS "Now is UNIX-like OS's.")
    target_link_libraries(main pthread)
ENDIF ()

target_link_libraries(main fdfsclient fastcommon event)