# --------------------------------------------------
# FreeType
message(STATUS "Using FreeType via FetchContent")

set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_GZIP ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PCF ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PFR ON CACHE BOOL "" FORCE)
set(FT_DISABLE_WOFF2 ON CACHE BOOL "" FORCE)

set(FT_WITH_BROTLI OFF CACHE BOOL "" FORCE)
set(FT_WITH_BZIP2 OFF CACHE BOOL "" FORCE)
set(FT_WITH_HARFBUZZ OFF CACHE BOOL "" FORCE)
set(FT_WITH_PNG OFF CACHE BOOL "" FORCE)
set(FT_WITH_ZLIB OFF CACHE BOOL "" FORCE)

set(FT_ENABLE_ERROR_STRINGS OFF CACHE BOOL "" FORCE)
set(FT_ENABLE_WARNINGS OFF CACHE BOOL "" FORCE)

set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype
    GIT_TAG VER-2-14-1
    GIT_SHALLOW TRUE
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
    GIT_SHALLOW TRUE
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
