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

target_include_directories(${APP_NAME} SYSTEM PRIVATE ${PHYSX_INCLUDE_DIRS})

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
