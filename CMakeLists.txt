cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Default variables
if (NOT DEFINED BUILD_TESTS)
    set(BUILD_TESTS OFF)
endif()
if (NOT DEFINED BUILD_EXAMPLES)
    set(BUILD_EXAMPLES OFF)
endif()
if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project(ProjectName)
add_library(${PROJECT_NAME} STATIC)

# compile options
function(CONFIGURE_TARGET TARGET_NAME)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${TARGET_NAME} PRIVATE -g -fsanitize=address -Wall -Wextra -Wpedantic -Werror)
        target_link_options(${TARGET_NAME} PRIVATE -fsanitize=address)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${TARGET_NAME} PRIVATE -O3)
    else()
    endif()
endfunction()

CONFIGURE_TARGET(${PROJECT_NAME})

# source files and headers
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_sources(${PROJECT_NAME}
    PRIVATE
    src/test.cpp
)


# handle tests/examples

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
