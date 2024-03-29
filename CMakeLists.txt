cmake_minimum_required (VERSION 3.9)
project (n3)

set (libn3_VERSION_MAJOR 1)
set (libn3_VERSION_MINOR 0)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message(FATAL_ERROR "libn3 only supports Linux")
endif()

# TODO: Static vs shared libraries
# TODO: Autodetection/messaging around PIE, LTO, and other features
# TODO: Feature flags
# TODO: Configuration options (tweaking internal static cache sizes, epoll vs io_uring, etc)

# DO NOT ADD ADDRESS SANITIZER TO THE COMPILE FLAGS
# SDL and OpenGL causes tons of errors
# We suppress them with a static valgrind log file, which can't be done with ASan
set(CMAKE_C_FLAGS "-Wall -Wextra -Wshadow -Wpedantic")
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

include(ExternalProject)
include(FetchContent)

set(LIBURING_BASE_ARGS
    GIT_REPOSITORY https://github.com/axboe/liburing
    GIT_TAG        liburing-2.5
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
    BUILD_IN_SOURCE TRUE
    CONFIGURE_COMMAND "./configure"
    )

if(CMAKE_VERSION VERSION_LESS "3.28.0")
    include(ProcessorCount)
    ProcessorCount(CORE_COUNT)
    if(CORE_COUNT EQUAL 0)
        set(CORE_COUNT 1)
    endif()
    set(LIBURING_BUILD_COMMAND
        BUILD_COMMAND make -j${CORE_COUNT}
    )
else()
    set(LIBURING_BUILD_COMMAND
        BUILD_COMMAND make
        BUILD_JOB_SERVER_AWARE TRUE
    )
endif()

ExternalProject_Add(liburing
    ${LIBURING_BASE_ARGS}
    ${LIBURING_BUILD_COMMAND}
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(liburing SOURCE_DIR)

add_library(liburing_static_lib STATIC IMPORTED GLOBAL)
set_target_properties(liburing_static_lib
    PROPERTIES
    IMPORTED_LOCATION ${SOURCE_DIR}/src/liburing.a
)






SET(COMMON
    "src/libn3.cpp"
    "src/epoll.cpp"
    "src/epoll_executor.cpp"
    "src/error.cpp"
    "src/tcp.cpp"
    "src/socket.cpp"
    "src/buffer.cpp"
    "src/syscalls.cpp"
    "src/dns.cpp"
    "src/runtime.cpp"
    "src/address.cpp"
    "src/callbacks.cpp"
    "src/handle.cpp"
    "src/timer_list.cpp"
    "src/io_uring.cpp"
    "src/ownership.cpp"
    "src/page_size.cpp"
    )

SET(COMMON_INCLUDE_DIRS
    "src/"
    "${SOURCE_DIR}/include/"
    )

find_package(Threads REQUIRED)

add_library(n3 SHARED ${COMMON} $<TARGET_OBJECTS:liburing_static_lib>)
add_dependencies(n3 liburing)
target_include_directories(n3 PUBLIC ${COMMON_INCLUDE_DIRS})
target_link_libraries(n3 PRIVATE ${liburing_static_lib})

set_property(TARGET n3 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
set_property(TARGET n3 PROPERTY POSITION_INDEPENDENT_CODE TRUE)


# TEST CONFIGURATION AND BUILDING

enable_testing()

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

set(TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/test/ownership.cpp"
)

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests PRIVATE n3 Catch2::Catch2WithMain)

#target_compile_options(tests
#    PRIVATE ${WARNINGS}
#    PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>" "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>")
target_compile_definitions(tests PRIVATE "$<$<CONFIG:RELEASE>:NDEBUG>")
target_include_directories(tests PRIVATE ${INCLUDE_DIRS})

include(CTest)
include(Catch)
catch_discover_tests(tests)
