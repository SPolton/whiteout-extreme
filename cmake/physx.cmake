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
        "  2. Build libraries for your platform (Windows or Linux)\n"
        "See README.md for detailed instructions.")
endif()

message(STATUS "Using local PhysX installation")

# Check if PhysX include directory exists for headers
set(PHYSX_INCLUDE_DIRS ${PHYSX_ROOT_DIR}/include)
if(NOT EXISTS ${PHYSX_INCLUDE_DIRS})
    message(FATAL_ERROR "PhysX include directory not found at: ${PHYSX_INCLUDE_DIRS}\n"
        "See README.md for setup instructions.")
endif()

# Determine PhysX platform output folder and library extension.
if(WIN32)
    set(PHYSX_BIN_PLATFORM_DIR "win.x86_64.vc143.mt")
    set(PHYSX_IMPORTED_LIB_EXT ".lib")
elseif(UNIX AND NOT APPLE)
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        set(PHYSX_BIN_PLATFORM_DIR "linux.aarch64")
    else()
        set(PHYSX_BIN_PLATFORM_DIR "linux.x86_64")
    endif()
    set(PHYSX_IMPORTED_LIB_EXT "")
else()
    message(FATAL_ERROR "Unsupported platform for PhysX integration: ${CMAKE_SYSTEM_NAME}")
endif()

# Check if PhysX platform binary directory exists.
set(PHYSX_LIB_DIR ${PHYSX_ROOT_DIR}/bin/${PHYSX_BIN_PLATFORM_DIR})
if(NOT EXISTS ${PHYSX_LIB_DIR})
    message(FATAL_ERROR "PhysX library directory not found at: ${PHYSX_LIB_DIR}\n"
        "See README.md for setup instructions.")
endif()

# Map project configuration to PhysX build folders.
# Keep compatibility with existing setups that build only Debug and Checked.
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(PHYSX_LIB_SUBDIR "debug")
else()
    set(PHYSX_LIB_SUBDIR "checked")
endif()

set(PHYSX_CONFIG_DIR ${PHYSX_LIB_DIR}/${PHYSX_LIB_SUBDIR})
if(NOT EXISTS ${PHYSX_CONFIG_DIR})
    message(FATAL_ERROR "PhysX ${PHYSX_LIB_SUBDIR} configuration directory not found at: ${PHYSX_CONFIG_DIR}\n"
        "Please build PhysX libraries for the '${PHYSX_LIB_SUBDIR}' configuration.\n"
        "See README.md for build instructions.")
endif()

message(STATUS "Using PhysX libraries from: ${PHYSX_CONFIG_DIR}")

target_include_directories(${APP_NAME} SYSTEM PRIVATE ${PHYSX_INCLUDE_DIRS})

# Core PhysX libraries (order matters for linking)
set(PHYSX_COMPONENTS
    PhysXFoundation
    PhysXCommon
    PhysX
    PhysXCooking
    PhysXExtensions
    PhysXPvdSDK
    PhysXVehicle2
    PhysXCharacterKinematic
)

# Create imported targets for each PhysX component with platform-specific naming.
foreach(COMPONENT ${PHYSX_COMPONENTS})
    set(PHYSX_COMPONENT_NAMES
        ${COMPONENT}_static_64
        ${COMPONENT}_64
        ${COMPONENT}_static
        ${COMPONENT}
    )

    unset(PHYSX_COMPONENT_LIBRARY CACHE)
    unset(PHYSX_COMPONENT_LIBRARY)

    if(WIN32)
        set(PHYSX_COMPONENT_LIBRARY "")
        foreach(CANDIDATE ${PHYSX_COMPONENT_NAMES})
            set(CANDIDATE_PATH "${PHYSX_CONFIG_DIR}/${CANDIDATE}${PHYSX_IMPORTED_LIB_EXT}")
            if(EXISTS ${CANDIDATE_PATH})
                set(PHYSX_COMPONENT_LIBRARY ${CANDIDATE_PATH})
                break()
            endif()
        endforeach()
    else()
        find_library(PHYSX_COMPONENT_LIBRARY
            NAMES ${PHYSX_COMPONENT_NAMES}
            PATHS ${PHYSX_CONFIG_DIR}
            NO_DEFAULT_PATH
        )
    endif()

    if(NOT PHYSX_COMPONENT_LIBRARY)
        message(FATAL_ERROR "PhysX component not found for ${COMPONENT} in ${PHYSX_CONFIG_DIR}\n"
            "Tried names: ${PHYSX_COMPONENT_NAMES}\n"
            "Please ensure PhysX is built for ${PHYSX_LIB_SUBDIR}.")
    endif()

    add_library(PhysX::${COMPONENT} UNKNOWN IMPORTED)
    set_target_properties(PhysX::${COMPONENT} PROPERTIES
        IMPORTED_LOCATION ${PHYSX_COMPONENT_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES "${PHYSX_INCLUDE_DIRS}"
    )
endforeach()

# Link PhysX libraries to the application.
# On Linux these static archives have circular dependencies, so force the linker to rescan them.
set(PHYSX_TARGETS "")
foreach(COMPONENT ${PHYSX_COMPONENTS})
    list(APPEND PHYSX_TARGETS PhysX::${COMPONENT})
endforeach()

if(UNIX AND NOT APPLE)
    target_link_libraries(${APP_NAME} PRIVATE
        "-Wl,--start-group"
        ${PHYSX_TARGETS}
        "-Wl,--end-group"
    )
else()
    target_link_libraries(${APP_NAME} PRIVATE ${PHYSX_TARGETS})
endif()

# Copy PhysX runtime shared libraries to the output directory when present.
if(WIN32)
    file(GLOB PHYSX_RUNTIME_LIBS ${PHYSX_CONFIG_DIR}/*.dll)
else()
    file(GLOB PHYSX_RUNTIME_LIBS ${PHYSX_CONFIG_DIR}/*.so*)
endif()

if(PHYSX_RUNTIME_LIBS)
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PHYSX_RUNTIME_LIBS}
        $<TARGET_FILE_DIR:${APP_NAME}>
        COMMENT "Copying PhysX runtime libraries to output directory"
    )
endif()
