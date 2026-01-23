# CMakeList.txt : CMake project for RacingGame, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.20)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OpenGL_GL_PREFERENCE GLVND)

set(APP_NAME "WhiteoutExtreme")
project(${APP_NAME} LANGUAGES CXX)

# --------------------------------------------------
# Add source to this project's executable
file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
    src/*.cpp
    src/*.h
)
add_executable(${APP_NAME} ${GAME_SRC})
target_include_directories(${APP_NAME} PRIVATE src)

# --------------------------------------------------
# Third-party libraries folder
include(FetchContent)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/libraries)
target_include_directories(${APP_NAME} PRIVATE ${LIB_DIR})

# --------------------------------------------------
# Enable Hot Reload for MSVC compilers if supported.
# if (POLICY CMP0141)
#   cmake_policy(SET CMP0141 NEW)
#   set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
# endif()

if (WIN32)
    message(STATUS "Windows compile definitions")
    target_compile_definitions(${APP_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
    )
endif()

# --------------------------------------------------
# Compiler warnings
if(MSVC)
    message(STATUS "MSVC compile options")
    target_compile_options(${APP_NAME} PRIVATE /W4)
else()
    message(STATUS "Default compile options")
    target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
