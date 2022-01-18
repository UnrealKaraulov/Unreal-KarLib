cmake_minimum_required(VERSION 3.9)
project(UnrealKarLibrary)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

set(FLAGS "-m32 -O3 -pipe -fvisibility-inlines-hidden -fvisibility=hidden -flto ")

set_target_properties(
        ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS
        "${FLAGS} \
        -fno-strict-aliasing \
        -fexceptions \
        -g \
        -Wall \
        -Werror \
        -fdata-sections \
        -ffunction-sections \
        -Wunknown-pragmas"
)


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"")
	set_target_properties(
			${PROJECT_NAME} PROPERTIES LINK_FLAGS
			"${FLAGS} \
			-Wl,--gc-sections \
			-static-libstdc++ \
			-static-libgcc"
	)
else()
	set_target_properties(
			${PROJECT_NAME} PROPERTIES LINK_FLAGS
			"${FLAGS} \
			-Wl,--gc-sections \
			-static-libstdc++ \
			-static-libgcc \
			-static-intel \
			-no-intel-extensions"
	)
endif()