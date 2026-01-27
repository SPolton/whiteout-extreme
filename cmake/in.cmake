# --------------------------------------------------
# Assimp
message(STATUS "Using Assimp via FetchContent")

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)

# Disable all importers by default, then enable only what we need
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
# set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
# set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
# set(ASSIMP_BUILD_BLEND_IMPORTER ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v6.0.4
)
FetchContent_MakeAvailable(assimp)

if(MSVC)
    set_property(TARGET assimp PROPERTY MSVC_RUNTIME_LIBRARY ${MT_CONFIG})
endif()

target_link_libraries(${APP_NAME} PRIVATE assimp)
