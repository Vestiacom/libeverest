cmake_minimum_required(VERSION 3.0.2)
project(everest) # Same as library's name

## Build configuration ########################################################
if(NOT DEFINED VERSION)
    set(VERSION "1.0.1")
endif(NOT DEFINED VERSION)

if(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "RELEASE")
endif(NOT CMAKE_BUILD_TYPE)

option(ENABLE_STATIC  "Build static library" ON)
option(ENABLE_TESTING "Build tests"          ON)
option(ENABLE_HEADERS "Install headers"      ON)

## Build info #################################################################
message(STATUS "-------------------------------------------------")
message(STATUS "Compiler:          " ${CMAKE_CXX_COMPILER_ID})
message(STATUS "Compiler version:  " ${CMAKE_CXX_COMPILER_VERSION})
message(STATUS "Build type:        " ${CMAKE_BUILD_TYPE})
message(STATUS "Enable static:     " ${ENABLE_STATIC})
message(STATUS "Enable testing:    " ${ENABLE_TESTING})
message(STATUS "Enable headers:    " ${ENABLE_HEADERS})
message(STATUS "Version:           " ${VERSION})
message(STATUS "-------------------------------------------------")

## Enable pkgconfig ###########################################################
include(FindPkgConfig)

## Build options ##############################################################
set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_C_FLAGS_RELEASE} -std=c++14 -Os -DNDEBUG -fvisibility=hidden -fvisibility-inlines-hidden")
set(CMAKE_CXX_FLAGS_DEBUG    "${CMAKE_C_FLAGS_DEBUG}   -std=c++14 -O0 -ggdb")
set(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_C_FLAGS_DEBUG}   -std=c++14 -O0 -ggdb -fsanitize=address -static-libasan -fno-omit-frame-pointer" )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")

## Definitions ################################################################
add_definitions("-fPIC")

# Make warnings into errors
add_definitions("-Werror")
add_definitions("-Wall")
add_definitions("-Wextra")

# Discarded warnings:
add_definitions("-Wno-variadic-macros")
add_definitions("-Wno-uninitialized")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	# Clang has a better warnings control so it's a special case
	add_definitions("-pedantic")
	add_definitions("-pedantic-errors")
    add_definitions("-Wdocumentation")
	add_definitions("-Wno-gnu-zero-variadic-macro-arguments")
endif()

## External projects ##########################################################
include(http-parser.cmake)

## Various install paths ######################################################
## default CMAKE_INSTALL_* variables
include(GNUInstallDirs)

if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif(NOT DEFINED LIB_INSTALL_DIR)

if(NOT DEFINED INCLUDE_INSTALL_DIR)
    set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")
endif(NOT DEFINED INCLUDE_INSTALL_DIR)

## Dirs #######################################################################
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(TESTS_DIR ${PROJECT_SOURCE_DIR}/tests)

## Subdirs ####################################################################
add_subdirectory(${SRC_DIR})

if(ENABLE_TESTING)
	add_subdirectory(${TESTS_DIR})
endif()
