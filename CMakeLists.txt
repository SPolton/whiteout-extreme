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

# --------------------------------------------------
# Third-party libraries folder
include(FetchContent)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/libraries)
target_include_directories(${APP_NAME} PRIVATE ${LIB_DIR})

# --------------------------------------------------
# OpenGL
message(STATUS "Using OpenGL")
find_package(OpenGL REQUIRED)
target_link_libraries(${APP_NAME} PRIVATE OpenGL::GL)

# --------------------------------------------------
# GLAD (OpenGL function loader)
# Note: GLAD v2.x does not provide a pre-built CMake target
# Generate GLAD files manually from https://glad.dav1d.de/
message(STATUS "Using GLAD via libraries")

# Create a separate library for GLAD to ensure proper compilation
add_library(glad STATIC
    ${CMAKE_SOURCE_DIR}/libraries/glad/src/glad.c
)

# Set the language to C for the GLAD library
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)

target_include_directories(glad PUBLIC
    ${CMAKE_SOURCE_DIR}/libraries/glad/include
)

target_link_libraries(${APP_NAME} PRIVATE glad)

# --------------------------------------------------
# GLFW
# Check for precompiled GLFW Windows binaries, otherwise use FetchContent
# https://www.glfw.org/download.html
# Put glfw-3.4.bin.WIN64 in the libraries folder

set(GLFW_PREBUILT_DIR ${CMAKE_SOURCE_DIR}/libraries/glfw-3.4.bin.WIN64)
if (NOT EXISTS "${GLFW_PREBUILT_DIR}")
    set(GLFW_PREBUILT_DIR ${CMAKE_SOURCE_DIR}/libraries/glfw-3.4.bin.WIN32)
endif()

if (WIN32 AND EXISTS "${GLFW_PREBUILT_DIR}")
    message(STATUS "Using precompiled GLFW binaries")
    add_library(glfw STATIC IMPORTED)
    set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION
            ${GLFW_PREBUILT_DIR}/lib-vc2022/glfw3.lib
        INTERFACE_INCLUDE_DIRECTORIES
            ${GLFW_PREBUILT_DIR}/include
    )
else()
    message(STATUS "Using GLFW via FetchContent")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
    )
    FetchContent_MakeAvailable(glfw)
endif()

target_link_libraries(${APP_NAME} PRIVATE glfw)

# --------------------------------------------------
# GLM (header-only library)
message(STATUS "Using GLM via FetchContent")
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(glm)

target_link_libraries(${APP_NAME} PRIVATE glm::glm)

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

# --------------------------------------------------
# PhysX - Using local installation from physx folder
message(STATUS "Using local PhysX installation")

set(PHYSX_ROOT_DIR ${CMAKE_SOURCE_DIR}/physx)
set(PHYSX_INCLUDE_DIRS ${PHYSX_ROOT_DIR}/include)
set(PHYSX_LIB_DIR ${PHYSX_ROOT_DIR}/bin/win.x86_64.vc143.mt)

# Use debug libraries for Debug builds, checked libraries for Release builds
# PhysX Debug configuration uses /MTd (matches our Debug)
# PhysX Checked configuration uses /MT (matches our Release)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(PHYSX_LIB_SUBDIR "debug")
    message(STATUS "Using PhysX Debug libraries (built with /MTd)")
elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(PHYSX_LIB_SUBDIR "checked")
    message(STATUS "Using PhysX Checked libraries (built with /MT)")
else()
    set(PHYSX_LIB_SUBDIR "checked")
    message(STATUS "Using PhysX Checked libraries (built with /MT)")
endif()

# Include directories
target_include_directories(${APP_NAME} PRIVATE 
    ${PHYSX_INCLUDE_DIRS}
)

# Core PhysX libraries (order matters for linking)
set(PHYSX_LIBS
    PhysXFoundation_64
    PhysXCommon_64
    PhysX_64
    PhysXCooking_64
    PhysXExtensions_static_64
    PhysXPvdSDK_static_64
    PhysXVehicle2_static_64
    PhysXCharacterKinematic_static_64
)

# Create imported targets for each PhysX library
foreach(LIB ${PHYSX_LIBS})
    add_library(PhysX::${LIB} STATIC IMPORTED)
    set_target_properties(PhysX::${LIB} PROPERTIES
        IMPORTED_LOCATION ${PHYSX_LIB_DIR}/${PHYSX_LIB_SUBDIR}/${LIB}.lib
        INTERFACE_INCLUDE_DIRECTORIES "${PHYSX_INCLUDE_DIRS}"
    )
endforeach()

# Link PhysX libraries to the application
target_link_libraries(${APP_NAME} PRIVATE 
    PhysX::PhysXFoundation_64
    PhysX::PhysXCommon_64
    PhysX::PhysX_64
    PhysX::PhysXCooking_64
    PhysX::PhysXExtensions_static_64
    PhysX::PhysXPvdSDK_static_64
    PhysX::PhysXVehicle2_static_64
    PhysX::PhysXCharacterKinematic_static_64
)

# Copy PhysX DLLs to output directory (if any exist)
# Note: Static libraries don't need DLLs, but shared ones do
file(GLOB PHYSX_DLLS ${PHYSX_LIB_DIR}/${PHYSX_LIB_SUBDIR}/*.dll)
if(PHYSX_DLLS)
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PHYSX_DLLS}
        $<TARGET_FILE_DIR:${APP_NAME}>
        COMMENT "Copying PhysX DLLs to output directory"
    )
endif()

# --------------------------------------------------
# FMOD
# message(STATUS "Finding FMOD")
# find_package(FMOD REQUIRED)
# target_link_libraries(${APP_NAME} PRIVATE FMOD::FMOD)

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
