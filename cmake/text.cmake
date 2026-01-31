
# fmt.dev
message(STATUS "Using fmt.dev via FetchContent")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(FT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(FT_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)

set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG 11.0.2 # 12.1.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(fmt)

if(MSVC)
    set_property(TARGET fmt PROPERTY MSVC_RUNTIME_LIBRARY ${MT_CONFIG})
endif()

target_link_libraries(${APP_NAME} PRIVATE fmt::fmt)


# vivid
message(STATUS "Using vivid via FetchContent")

set(VIVID_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(VIVID_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    vivid
    GIT_REPOSITORY https://github.com/gurki/vivid
    GIT_TAG v3.1.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(vivid)

if(MSVC)
    set_property(TARGET vivid PROPERTY MSVC_RUNTIME_LIBRARY ${MT_CONFIG})
endif()

target_link_libraries(${APP_NAME} PRIVATE vivid)
