# CMakeList.txt : CMake project for RacingGame, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.20)

# --------------------------------------------------
# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OpenGL_GL_PREFERENCE GLVND)

set(APP_NAME "WhiteoutExtreme")
project(${APP_NAME} LANGUAGES C CXX)

# --------------------------------------------------
# Use static runtime library to match PhysX
# Our PhysX is built with /MT (Release) and /MTd (Debug)
set(MT_CONFIG "MultiThreaded$<$<CONFIG:Debug>:Debug>")
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY ${MT_CONFIG})
    message(STATUS "Using static runtime library (/MT for Release, /MTd for Debug)")
endif()

# --------------------------------------------------
# Add source to this project's executable
message(STATUS "Adding source files to executable")
file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
    src/*.cpp
    src/*.hpp
    src/*.h
)
add_executable(${APP_NAME} ${GAME_SRC})
target_include_directories(${APP_NAME} PRIVATE src)

# Force static runtime library for the target (match PhysX)
if(MSVC)
    set_property(TARGET ${APP_NAME} PROPERTY MSVC_RUNTIME_LIBRARY ${MT_CONFIG})
endif()

# --------------------------------------------------
# Copy assets to build directory
message(STATUS "Copying assets to build directory")
add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${APP_NAME}>/assets
    COMMENT "Copying assets to build directory"
)

# --------------------------------------------------
# Third-party libraries folder
set(LIB_FOLDER libraries)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/${LIB_FOLDER})
target_include_directories(${APP_NAME} PRIVATE ${LIB_DIR})

include(FetchContent)
set(FETCHCONTENT_QUIET ON)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# --------------------------------------------------
# stb https://github.com/nothings/stb
message(STATUS "Using stb headers in ${LIB_FOLDER}")
target_include_directories(${APP_NAME} PRIVATE ${LIB_DIR}/stb)

# fmt.dev, vivid
include(cmake/text.cmake)

# rapidjson, Assimp
include(cmake/in.cmake)

# OpenGL, GLAD, GLFW, GLM
include(cmake/gl.cmake)

# FreeType, ImGui
include(cmake/ui.cmake)

# PhysX
include(cmake/physx.cmake)

# FMOD
# include(cmake/fmod.cmake)

# --------------------------------------------------
# Enable Hot Reload for MSVC compilers if supported.
# if (POLICY CMP0141)
#   cmake_policy(SET CMP0141 NEW)
#   set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
# endif()

if (WIN32)
    message(STATUS "Using Windows compile definitions")
    target_compile_definitions(${APP_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
    )
endif()

# --------------------------------------------------
# Compiler warnings
if(MSVC)
    message(STATUS "Using MSVC compile options")
    target_compile_options(${APP_NAME} PRIVATE /W4)
else()
    message(STATUS "Using default compile options")
    target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
