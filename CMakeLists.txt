cmake_minimum_required(VERSION 3.15) #指定 CMake 的最低版本要求：

# 定义项目的名称和使用的编程语言:
project(webserver)

if(CMAKE_TOOLCHAIN_FILE)
    include(${CMAKE_TOOLCHAIN_FILE})
    message(STATUS "Current toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build Type" FORCE)
    # "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
endif()
message(STATUS "Current build type: ${CMAKE_BUILD_TYPE}")

# output directory
if (NOT DEFINED BIN_DIR)
    string(TOLOWER "${CMAKE_BUILD_TYPE}" TARGET_DIR)
    set(BIN_DIR "${PROJECT_SOURCE_DIR}/bin/${TARGET_DIR}")
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

# set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD 99)
# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(EXECUTABLE_NAME webserver)


option(BUILD_ASAN "Build with address sanitizer (gcc/clang)" OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.0")
        message(FATAL_ERROR "GCC < 4.8 doesn't support the address sanitizer")
    else()
        if (BUILD_ASAN)
            message(STATUS "BUILD_ASAN: define")
            set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address")
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
            # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libasan")
        else()
            message(STATUS "BUILD_ASAN: undefine")
        endif()
    endif()
endif()

add_subdirectory(src)
