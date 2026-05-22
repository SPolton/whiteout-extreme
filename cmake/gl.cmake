# --------------------------------------------------
# OpenGL
message(STATUS "Using OpenGL")
find_package(OpenGL REQUIRED)
target_link_libraries(${APP_NAME} PRIVATE OpenGL::GL)

# --------------------------------------------------
# GLAD (OpenGL function loader)
# Note: GLAD does not provide a pre-built CMake target
# Generate GLAD files manually from https://glad.dav1d.de/
message(STATUS "Using GLAD in ${LIB_FOLDER}")

add_library(glad STATIC ${CMAKE_SOURCE_DIR}/${LIB_FOLDER}/glad/src/glad.c)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)

# Ensure GLAD uses the same runtime library as our project
if(MSVC)
    set_property(TARGET glad PROPERTY MSVC_RUNTIME_LIBRARY ${MT_CONFIG})
endif()

target_include_directories(glad SYSTEM PUBLIC ${CMAKE_SOURCE_DIR}/${LIB_FOLDER}/glad/include)

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

# Use static library to match PhysX
set(GLFW_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLFW_USE_STATIC_CRT ON CACHE BOOL "" FORCE)

# Disable Vulkan support (only using OpenGL)
set(GLFW_VULKAN_STATIC OFF CACHE BOOL "" FORCE)

# On Linux, allow GLFW to use native window backends.
if(UNIX AND NOT APPLE)
    set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
else()
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

# Ensure GLFW uses the same runtime library as our project
if(MSVC)
    set_property(TARGET glfw PROPERTY MSVC_RUNTIME_LIBRARY ${MT_CONFIG})
endif()

target_link_libraries(${APP_NAME} PRIVATE glfw)

# --------------------------------------------------
# GLM (header-only)
message(STATUS "Using GLM via FetchContent")

set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

target_include_directories(${APP_NAME} SYSTEM PRIVATE ${glm_SOURCE_DIR}/glm)
