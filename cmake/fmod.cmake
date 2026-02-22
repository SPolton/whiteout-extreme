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

# Check if FMOD library directory exists
set(FMOD_LIB_DIR ${FMOD_ROOT_DIR}/lib/x64)
set(FMOD_LIB_NAME fmod_vc)
if(NOT EXISTS ${FMOD_LIB_DIR})
    message(FATAL_ERROR "FMOD library directory not found at: ${FMOD_LIB_DIR}\n")
endif()

set(FMOD_STD_LIB_DIR ${FMOD_STUDIO_DIR}/lib/x64)
set(FMOD_STD_LIB_NAME fmodstudio_vc)
if(NOT EXISTS ${FMOD_STD_LIB_DIR})
    message(FATAL_ERROR "FMOD studio library directory not found at: ${FMOD_STD_LIB_DIR}\n")
endif()

# Use debug libraries for Debug builds, checked libraries for Release builds
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(FMOD_LIB_FILE "${FMOD_LIB_DIR}/${FMOD_LIB_NAME}.lib")
    set(FMOD_STD_LIB_FILE "${FMOD_STD_LIB_DIR}/${FMOD_STD_LIB_NAME}.lib")
    message(STATUS "Using FMOD Debug library: ${FMOD_LIB_FILE}\n")
else()
    set(FMOD_LIB_FILE "${FMOD_LIB_DIR}/${FMOD_LIB_NAME}.lib")
    set(FMOD_STD_LIB_FILE "${FMOD_STD_LIB_DIR}/${FMOD_STD_LIB_NAME}.lib")
    message(STATUS "Using FMOD Checked library: ${FMOD_LIB_FILE}\n")
endif()

target_include_directories(${APP_NAME} SYSTEM PRIVATE ${FMOD_INCLUDE_DIR})
target_include_directories(${APP_NAME} SYSTEM PRIVATE ${FMOD_STUDIO_INCLUDE_DIR})


# Create imported targets for FMOD
add_library(FMOD::FMOD STATIC IMPORTED)
set_target_properties(FMOD::FMOD PROPERTIES
    IMPORTED_LOCATION ${FMOD_LIB_FILE}
    INTERFACE_INCLUDE_DIRECTORIES "${FMOD_INCLUDE_DIR}"
)

# ---- FMOD Studio ----
add_library(FMOD::FMODSTUDIO STATIC IMPORTED)
set_target_properties(FMOD::FMODSTUDIO PROPERTIES
    IMPORTED_LOCATION ${FMOD_STD_LIB_FILE}
    INTERFACE_INCLUDE_DIRECTORIES "${FMOD_STUDIO_INCLUDE_DIR}"
)

# Link FMOD library to the application
target_link_libraries(${APP_NAME} PRIVATE FMOD::FMOD)

# Link studio as well
target_link_libraries(${APP_NAME} PRIVATE FMOD::FMODSTUDIO)

# Copy FMOD DLLs to output directory (if any exist)
file(GLOB FMOD_DLLS "${FMOD_LIB_DIR}/fmod.dll")
file(GLOB FMOD_STD_DLLS "${FMOD_STD_LIB_DIR}/fmodstudio.dll")
if(FMOD_DLLS)
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${FMOD_DLLS}
        $<TARGET_FILE_DIR:${APP_NAME}>
        COMMENT "Copying FMOD DLLs to output directory"
    )
endif()
if(FMOD_STD_DLLS)
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${FMOD_STD_DLLS}
        $<TARGET_FILE_DIR:${APP_NAME}>
        COMMENT "Copying FMOD Studio DLLs to output directory"
    )
endif()
