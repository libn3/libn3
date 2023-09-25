cmake_minimum_required (VERSION 3.9)
project (libn3)

set (libn3_VERSION_MAJOR 1)
set (libn3_VERSION_MINOR 0)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message(FATAL_ERROR "libn3 only supports Linux")
endif()

# DO NOT ADD ADDRESS SANITIZER TO THE COMPILE FLAGS
# SDL and OpenGL causes tons of errors
# We suppress them with a static valgrind log file, which can't be done with ASan
set(CMAKE_C_FLAGS "-Wall -Wextra")
# TSan doesn't play well with Valgrind, need to pick and choose
#set(CMAKE_C_FLAGS_DEBUG "-g -O0 -fsanitize=thread -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_RELEASE "-Ofast -s -fno-ident -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})

#set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 23)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}")

set(THREADS_PREFER_PTHREAD_FLAG ON)

add_definitions(-D_POSIX_C_SOURCE=200809L)
#Needed for madvise
add_definitions(-D_DEFAULT_SOURCE)

SET(COMMON
    "src/libn3.cpp"
    )

SET(COMMON_INCLUDE_DIRS
    "src/"
    )

find_package(Threads REQUIRED)

add_library(brrr-common OBJECT ${COMMON})
target_include_directories(brrr-common PUBLIC ${COMMON_INCLUDE_DIRS})

set_property(TARGET brrr-common PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
