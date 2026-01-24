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
# OpenGL
message(STATUS "Using OpenGL")
find_package(OpenGL REQUIRED)
target_link_libraries(${APP_NAME} PRIVATE OpenGL::GL)

# --------------------------------------------------
# GLAD (OpenGL function loader)
# Note: GLAD v2.x does not provide a pre-built CMake target
# Generate GLAD files manually from https://glad.dav1d.de/
message(STATUS "Using GLAD in ${LIB_FOLDER}")

add_library(glad STATIC ${CMAKE_SOURCE_DIR}/${LIB_FOLDER}/glad/src/glad.c)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)

# Ensure GLAD uses the same runtime library as our project
if(MSVC)
    set_property(TARGET glad PROPERTY MSVC_RUNTIME_LIBRARY ${MT_CONFIG})
endif()

target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/${LIB_FOLDER}/glad/include)

target_link_libraries(${APP_NAME} PRIVATE glad)

# --------------------------------------------------
# GLFW
# Build from source to match our static runtime library (/MT, /MTd)
# Precompiled binaries are built with dynamic runtime and cause linker errors
message(STATUS "Building GLFW from source via FetchContent")

# Configure GLFW to not build examples, tests, or docs
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Use static CRT to match PhysX
set(GLFW_USE_STATIC_CRT ON CACHE BOOL "" FORCE)

# Disable Vulkan support (only using OpenGL)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

# Ensure GLFW uses the same runtime library as our project
if(MSVC)
    set_property(TARGET glfw PROPERTY MSVC_RUNTIME_LIBRARY ${MT_CONFIG})
endif()

target_link_libraries(${APP_NAME} PRIVATE glfw)

# --------------------------------------------------
# GLM (header-only library)
message(STATUS "Using GLM via FetchContent")

set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(glm)

# Ensure GLM uses the same runtime library as our project
if(MSVC AND TARGET glm)
    set_property(TARGET glm PROPERTY MSVC_RUNTIME_LIBRARY ${MT_CONFIG})
endif()

target_link_libraries(${APP_NAME} PRIVATE glm)
target_include_directories(${APP_NAME} PRIVATE ${glm_SOURCE_DIR}/glm)

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

# Check if local PhysX directory exists
set(PHYSX_ROOT_DIR ${CMAKE_SOURCE_DIR}/physx)
if(NOT EXISTS ${PHYSX_ROOT_DIR})
    set(PHYSX_ROOT_DIR ${CMAKE_SOURCE_DIR}/${LIB_FOLDER}/physx)
endif()
if(NOT EXISTS ${PHYSX_ROOT_DIR})
    message(FATAL_ERROR "PhysX directory not found at ${PHYSX_ROOT_DIR}\n"
        "Please install PhysX SDK:\n"
        "  1. Download PhysX 5.x from https://github.com/NVIDIA-Omniverse/PhysX\n"
        "  2. Build the libraries with /MT (Release/Checked) and /MTd (Debug)\n"
        "See README.md for detailed instructions.")
endif()

message(STATUS "Using local PhysX installation")

# Check if PhysX include directory exists for headers
set(PHYSX_INCLUDE_DIRS ${PHYSX_ROOT_DIR}/include)
if(NOT EXISTS ${PHYSX_INCLUDE_DIRS})
    message(FATAL_ERROR "PhysX include directory not found at: ${PHYSX_INCLUDE_DIRS}\n"
        "See README.md for setup instructions.")
endif()

# Check if PhysX library directory exists for binaries
set(PHYSX_LIB_DIR ${PHYSX_ROOT_DIR}/bin/win.x86_64.vc143.mt)
if(NOT EXISTS ${PHYSX_LIB_DIR})
    message(FATAL_ERROR "PhysX library directory not found at: ${PHYSX_LIB_DIR}\n"
        "See README.md for setup instructions.")
endif()

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

# Check if the specific configuration directory exists
set(PHYSX_CONFIG_DIR ${PHYSX_LIB_DIR}/${PHYSX_LIB_SUBDIR})
if(NOT EXISTS ${PHYSX_CONFIG_DIR})
    message(FATAL_ERROR "PhysX ${PHYSX_LIB_SUBDIR} configuration directory not found at: ${PHYSX_CONFIG_DIR}\n"
        "Please build PhysX libraries for the '${PHYSX_LIB_SUBDIR}' configuration.\n"
        "See README.md for build instructions.")
endif()

target_include_directories(${APP_NAME} PRIVATE ${PHYSX_INCLUDE_DIRS})

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
    set(LIB_PATH ${PHYSX_LIB_DIR}/${PHYSX_LIB_SUBDIR}/${LIB}.lib)
    if(NOT EXISTS ${LIB_PATH})
        message(FATAL_ERROR "PhysX library not found: ${LIB_PATH}\n"
            "Missing library: ${LIB}.lib\n"
            "Please ensure all required PhysX libraries are built.\n"
            "See README.md for setup instructions.")
    endif()
    
    add_library(PhysX::${LIB} STATIC IMPORTED)
    set_target_properties(PhysX::${LIB} PROPERTIES
        IMPORTED_LOCATION ${LIB_PATH}
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
