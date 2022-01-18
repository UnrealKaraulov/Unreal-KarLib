cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)

project(UnrealKarLibrary LANGUAGES CXX)
ENABLE_LANGUAGE(C)

# src
set(THIRD_PARTY "${CMAKE_SOURCE_DIR}/third_party")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
include_directories(${SRC_DIR})

# api
set(METAMOD "${THIRD_PARTY}/metamod-hl1/metamod")
set(HLSDK "${THIRD_PARTY}/hlsdk")
set(AMXXSDK "${THIRD_PARTY}/amxmodx/public")

include_directories(${AMXXSDK})
include_directories(${AMXXSDK}/sdk)

set(API_INCLUDE_PATH "${THIRD_PARTY} ${HLSDK}/common ${HLSDK}/dlls ${HLSDK}/engine ${HLSDK}/pm_shared ${HLSDK}/game_shared ${HLSDK}/public ${METAMOD}")
string(REPLACE " " ";" API_INCLUDE_PATH_LIST ${API_INCLUDE_PATH})
include_directories(${API_INCLUDE_PATH_LIST})

link_directories(${THIRD_PARTY}/linux32)

INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFileCXX)

CHECK_INCLUDE_FILE(stdint.h     HAVE_STDINT_H)

file(GLOB_RECURSE SOURCE_FILES ${SRC_DIR}/*.cpp ${AMXXSDK}/sdk/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "karlib")
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX "_amxx_i386.so")

set(FLAGS "-m32 -std=gnu++17 -std=c++17 -O3")

if ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
	set_target_properties(
			${PROJECT_NAME} PROPERTIES LINK_FLAGS
			"${FLAGS} \
			-static-libstdc++ \
			-static-libgcc"
	)
else()
	set_target_properties(
			${PROJECT_NAME} PROPERTIES LINK_FLAGS
			"${FLAGS} \
			-static-libstdc++ \
			-static-libgcc \
			-static-intel \
			-no-intel-extensions"
	)
endif()


set_property(GLOBAL PROPERTY CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)


set_target_properties(
        ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS
        "${FLAGS} \
        -fexceptions \
        -Wall \
        -Werror "
)

add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)