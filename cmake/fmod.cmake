# --------------------------------------------------
# FMOD

# Check if local FMOD directory exists
set(FMOD_ROOT_DIR ${CMAKE_SOURCE_DIR}/libraries/fmod/api/core)
set(FMOD_STUDIO_DIR ${CMAKE_SOURCE_DIR}/libraries/fmod/api/studio)
if(NOT EXISTS ${FMOD_ROOT_DIR})
    set(FMOD_ROOT_DIR ${CMAKE_SOURCE_DIR}/${LIB_FOLDER}/fmod/api/core)
    set(FMOD_STUDIO_DIR ${CMAKE_SOURCE_DIR}/libraries/fmod/api/studio)
endif()
if(NOT EXISTS ${FMOD_ROOT_DIR})
    message(FATAL_ERROR "FMOD directory not found at ${FMOD_ROOT_DIR}\n"
        "Please download FMOD Engine API:\n"
        " 1. Download FMOD Engine 2.x from https://www.fmod.com/download#fmodengine\n"
        " 1.1. Need to create an account to download\n"
        " 2. Find the FMOD program folder, navigate until you see a folder named api, and place it under libraries/fmod\n")
endif()

message(STATUS "Using local FMOD installation")

# Check if FMOD include directory exists for headers
set(FMOD_INCLUDE_DIR ${FMOD_ROOT_DIR}/inc)
set(FMOD_STUDIO_INCLUDE_DIR ${FMOD_STUDIO_DIR}/inc)
if(NOT EXISTS ${FMOD_INCLUDE_DIR})
    message(FATAL_ERROR "FMOD include directory not found at: ${FMOD_INCLUDE_DIR}\n")
endif()
if(NOT EXISTS ${FMOD_STUDIO_INCLUDE_DIR})
    message(FATAL_ERROR "FMOD studio include directory not found at: ${FMOD_STUDIO_INCLUDE_DIR}\n")
endif()

# Configure platform-specific FMOD library names.
if(WIN32)
    set(FMOD_LIB_DIR ${FMOD_ROOT_DIR}/lib/x64)
    set(FMOD_STD_LIB_DIR ${FMOD_STUDIO_DIR}/lib/x64)

    set(FMOD_CORE_LIB_NAME fmod_vc)
    set(FMOD_STUDIO_LIB_NAME fmodstudio_vc)
    set(FMOD_LIB_EXT .lib)

    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(FMOD_RUNTIME_GLOB "fmodL*.dll")
        set(FMOD_STUDIO_RUNTIME_GLOB "fmodstudioL*.dll")
    else()
        set(FMOD_RUNTIME_GLOB "fmod*.dll")
        set(FMOD_STUDIO_RUNTIME_GLOB "fmodstudio*.dll")
    endif()
elseif(UNIX AND NOT APPLE)
    set(FMOD_LIB_DIR ${FMOD_ROOT_DIR}/lib/x86_64)
    set(FMOD_STD_LIB_DIR ${FMOD_STUDIO_DIR}/lib/x86_64)

    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(FMOD_CORE_LIB_NAME fmodL)
        set(FMOD_STUDIO_LIB_NAME fmodstudioL)
    else()
        set(FMOD_CORE_LIB_NAME fmod)
        set(FMOD_STUDIO_LIB_NAME fmodstudio)
    endif()

    set(FMOD_LIB_EXT "")
    set(FMOD_RUNTIME_GLOB "libfmod*.so*")
    set(FMOD_STUDIO_RUNTIME_GLOB "libfmodstudio*.so*")
else()
    message(FATAL_ERROR "Unsupported platform for FMOD integration: ${CMAKE_SYSTEM_NAME}")
endif()

if(NOT EXISTS ${FMOD_LIB_DIR})
    message(FATAL_ERROR "FMOD library directory not found at: ${FMOD_LIB_DIR}\n")
endif()

if(NOT EXISTS ${FMOD_STD_LIB_DIR})
    message(FATAL_ERROR "FMOD studio library directory not found at: ${FMOD_STD_LIB_DIR}\n")
endif()

if(WIN32)
    set(FMOD_LIB_FILE "${FMOD_LIB_DIR}/${FMOD_CORE_LIB_NAME}${FMOD_LIB_EXT}")
    set(FMOD_STD_LIB_FILE "${FMOD_STD_LIB_DIR}/${FMOD_STUDIO_LIB_NAME}${FMOD_LIB_EXT}")
else()
    find_library(FMOD_LIB_FILE
        NAMES ${FMOD_CORE_LIB_NAME}
        PATHS ${FMOD_LIB_DIR}
        NO_DEFAULT_PATH
    )

    find_library(FMOD_STD_LIB_FILE
        NAMES ${FMOD_STUDIO_LIB_NAME}
        PATHS ${FMOD_STD_LIB_DIR}
        NO_DEFAULT_PATH
    )
endif()

if(NOT FMOD_LIB_FILE)
    message(FATAL_ERROR "FMOD core library not found in ${FMOD_LIB_DIR}\n")
endif()

if(NOT FMOD_STD_LIB_FILE)
    message(FATAL_ERROR "FMOD studio library not found in ${FMOD_STD_LIB_DIR}\n")
endif()

message(STATUS "Using FMOD core library: ${FMOD_LIB_FILE}")
message(STATUS "Using FMOD studio library: ${FMOD_STD_LIB_FILE}")

target_include_directories(${APP_NAME} SYSTEM PRIVATE ${FMOD_INCLUDE_DIR})
target_include_directories(${APP_NAME} SYSTEM PRIVATE ${FMOD_STUDIO_INCLUDE_DIR})


# Create imported targets for FMOD
add_library(FMOD::FMOD UNKNOWN IMPORTED)
set_target_properties(FMOD::FMOD PROPERTIES
    IMPORTED_LOCATION ${FMOD_LIB_FILE}
    INTERFACE_INCLUDE_DIRECTORIES "${FMOD_INCLUDE_DIR}"
)

# ---- FMOD Studio ----
add_library(FMOD::FMODSTUDIO UNKNOWN IMPORTED)
set_target_properties(FMOD::FMODSTUDIO PROPERTIES
    IMPORTED_LOCATION ${FMOD_STD_LIB_FILE}
    INTERFACE_INCLUDE_DIRECTORIES "${FMOD_STUDIO_INCLUDE_DIR}"
)

# Link FMOD library to the application
target_link_libraries(${APP_NAME} PRIVATE FMOD::FMOD)

# Link studio as well
target_link_libraries(${APP_NAME} PRIVATE FMOD::FMODSTUDIO)

# Copy FMOD runtime libraries to output directory.
file(GLOB FMOD_RUNTIME_LIBS "${FMOD_LIB_DIR}/${FMOD_RUNTIME_GLOB}")
file(GLOB FMOD_STUDIO_RUNTIME_LIBS "${FMOD_STD_LIB_DIR}/${FMOD_STUDIO_RUNTIME_GLOB}")

if(FMOD_RUNTIME_LIBS)
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${FMOD_RUNTIME_LIBS}
        $<TARGET_FILE_DIR:${APP_NAME}>
        COMMENT "Copying FMOD runtime libraries to output directory"
    )
endif()

if(FMOD_STUDIO_RUNTIME_LIBS)
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${FMOD_STUDIO_RUNTIME_LIBS}
        $<TARGET_FILE_DIR:${APP_NAME}>
        COMMENT "Copying FMOD Studio runtime libraries to output directory"
    )
endif()
