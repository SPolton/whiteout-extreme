# CMakeList.txt : CMake project for RacingGame, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.20)

# --------------------------------------------------
# Use static runtime library to match PhysX
# Our PhysX is built with /MT (Release) and /MTd (Debug)
# This MUST be set BEFORE the project() command to affect all dependencies
set(MT_CONFIG "MultiThreaded$<$<CONFIG:Debug>:Debug>")
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY ${MT_CONFIG})
    message(STATUS "Using static runtime library (/MT for Release, /MTd for Debug)")
endif()

# --------------------------------------------------
# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OpenGL_GL_PREFERENCE GLVND)

set(APP_NAME "WhiteoutExtreme")
project(${APP_NAME} LANGUAGES C CXX)

# --------------------------------------------------
# Add source to this project's executable
message(STATUS "Adding source files to executable")
file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
    src/*.cpp
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
include(FetchContent)
set(LIB_FOLDER libraries)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/${LIB_FOLDER})
target_include_directories(${APP_NAME} PRIVATE ${LIB_DIR})

# --------------------------------------------------
# OpenGL, GLAD, GLFW, GLM
include(cmake/gl.cmake)

# --------------------------------------------------
# FreeType
message(STATUS "Using FreeType via FetchContent")
FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype
    GIT_TAG VER-2-14-1
)
FetchContent_MakeAvailable(freetype)

target_link_libraries(${APP_NAME} PRIVATE freetype)
target_include_directories(${APP_NAME} PRIVATE ${freetype_SOURCE_DIR}/include)
add_definitions(-DIMGUI_DISABLE_FREETYPE)

# --------------------------------------------------
# ImGui
message(STATUS "Using ImGui via FetchContent")
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.5
)
FetchContent_MakeAvailable(imgui)

target_sources(${APP_NAME} PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(${APP_NAME} PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# ImGui build optimizations
target_compile_definitions(${APP_NAME} PRIVATE
    # Disable demo window and metrics/debugger to reduce binary size
    IMGUI_DISABLE_DEMO_WINDOWS
    IMGUI_DISABLE_DEBUG_TOOLS
    
    # Disable obsolete functions for cleaner API
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    
    # Optional: Disable file browser (if not using ImGui file dialogs)
    IMGUI_DISABLE_FILE_FUNCTIONS
)

# --------------------------------------------------
# PhysX
include(cmake/physx.cmake)

# --------------------------------------------------
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
