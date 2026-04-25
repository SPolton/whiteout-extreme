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

# Keep global compile flags minimal so external dependencies don't inherit unwanted flags.

# --------------------------------------------------
# Use static runtime library to match PhysX on Windows
# Our Windows PhysX builds use /MT (Release) and /MTd (Debug)
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
add_executable(${APP_NAME} ${GAME_SRC} "src/ui/GameMenus.cpp")
target_include_directories(${APP_NAME} PRIVATE src)

# Force static runtime library for the target (match PhysX)
if(MSVC)
    set_property(TARGET ${APP_NAME} PROPERTY MSVC_RUNTIME_LIBRARY ${MT_CONFIG})
endif()

# --------------------------------------------------
# Copy assets to build directory with dependency tracking
message(STATUS "Copying assets to build directory")

# Gather all asset files to track as dependencies
file(GLOB_RECURSE ASSET_FILES "${CMAKE_SOURCE_DIR}/assets/*")

# Create a custom target that depends on all asset files
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:${APP_NAME}>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${APP_NAME}>/assets
    COMMENT "Copying assets to build directory"
    DEPENDS ${ASSET_FILES}
)

# Make the executable depend on the asset copying
add_dependencies(${APP_NAME} copy_assets)

# --------------------------------------------------
# Third-party libraries folder
set(LIB_FOLDER libraries)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/${LIB_FOLDER})
target_include_directories(${APP_NAME} PRIVATE ${LIB_DIR})

include(FetchContent)
set(FETCHCONTENT_QUIET ON)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# Force static builds for FetchContent libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# --------------------------------------------------
# stb https://github.com/nothings/stb
message(STATUS "Using stb headers in ${LIB_FOLDER}")
target_include_directories(${APP_NAME} PRIVATE ${LIB_DIR}/stb)

# pl_mpeg https://github.com/phoboslab/pl_mpeg
message(STATUS "Using pl_mpeg headers in ${LIB_FOLDER}")
target_include_directories(${APP_NAME} PRIVATE ${LIB_DIR}/pl_mpeg)

# fmt.dev, vivid
include(cmake/text.cmake)

# AssImp, RapidJSON
include(cmake/in.cmake)

# OpenGL, GLAD, GLFW, GLM
include(cmake/gl.cmake)

# FreeType, ImGui
include(cmake/ui.cmake)

# PhysX
include(cmake/physx.cmake)

# FMOD
include(cmake/fmod.cmake)

# XInput (Windows only)
if (WIN32)
    target_link_libraries(${APP_NAME} PRIVATE Xinput9_1_0)
endif()

if(UNIX AND NOT APPLE)
    # Linux PhysX/FMOD static and shared dependencies require these system libs.
    target_link_libraries(${APP_NAME} PRIVATE pthread dl)
endif()

# --------------------------------------------------
# Enable Hot Reload for MSVC compilers if supported.
# if (POLICY CMP0141)
#   cmake_policy(SET CMP0141 NEW)
# endif()

if (WIN32)
    message(STATUS "Using Windows compile definitions")
    target_compile_definitions(${APP_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
    )
endif()

# --------------------------------------------------
# Compiler warnings and options
if(MSVC)
    message(STATUS "Using MSVC compile options")
    target_compile_options(${APP_NAME} PRIVATE /W4 /EHsc /MP)
else()
    message(STATUS "Using default compile options for ${APP_NAME}")
    target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra -pedantic)

    if(UNIX AND NOT APPLE)
        target_compile_options(${APP_NAME} PRIVATE
            $<$<CONFIG:Debug>:-g3>
            $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
        )
    endif()
endif()
