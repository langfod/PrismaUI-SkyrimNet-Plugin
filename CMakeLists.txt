cmake_minimum_required(VERSION 4.1)
add_definitions(-D_WIN32_WINNT=0x0A00)
option(PRISMAUI_ENABLE_INSPECTOR "Enable PrismaUI Inspector" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(
    PrismaUI-SkyrimNet-UI
    VERSION 1.1.1
    LANGUAGES CXX
)

if(PRISMAUI_ENABLE_INSPECTOR)
 add_compile_definitions(PRISMAUI_ENABLE_INSPECTOR)
endif()

# Set build root for external builds
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(BUILD_ROOT "${CMAKE_SOURCE_DIR}/build")
# Include compiler flags configuration
include(CompilerFlags)

# find vcpkg packages
find_package(httplib CONFIG REQUIRED)

# Include External Dependencies configuration
include(ExternalDependencies)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include)

# Add the plugin using CommonLibSSE function
add_commonlibsse_plugin(${PROJECT_NAME}
    NAME "PrismaUI-SkyrimNet-UI"
    AUTHOR "SkyrimNet Community"
    USE_ADDRESS_LIBRARY
    MINIMUM_SKSE_VERSION "2.0.12"
    VERSION ${PROJECT_VERSION}
    SOURCES
    src/main.cpp
    src/ui/UIBridge.cpp
    src/skyrimnet/GameMasterController.cpp
    src/keyhandler/keyhandler.cpp
    src/http/HttpClient.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Required for delay loading
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    delayimp
    httplib::httplib
)
# Precompiled header
target_precompile_headers(${PROJECT_NAME}
    PRIVATE
    "include/PCH.h"
)
link_external_dependencies(${PROJECT_NAME})

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)


# Post-build: Create distribution folder structure
set(DIST_ROOT "${CMAKE_SOURCE_DIR}/dist")
set(DIST_VERSION_DIR "${DIST_ROOT}/${PROJECT_NAME}_${PROJECT_VERSION}")
set(DIST_PRISMAUI_DIR "${DIST_VERSION_DIR}/PrismaUI")
set(DIST_SKSE_PLUGINS_DIR "${DIST_VERSION_DIR}/SKSE/plugins")
set(DIST_VIEWS_DIR "${DIST_PRISMAUI_DIR}/views/${PROJECT_NAME}")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    # Create distribution directories
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_VIEWS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_SKSE_PLUGINS_DIR}"
    
    # Copy views folder contents to PrismaUI folder
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/view" "${DIST_VIEWS_DIR}"
       
    # Copy PrismaUI DLL to SKSE/plugins
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${PROJECT_NAME}>" "${DIST_SKSE_PLUGINS_DIR}/"
    
    # Copy PrismaUI PDB to SKSE/plugins (if exists)
    COMMAND ${CMAKE_COMMAND} -E $<IF:$<BOOL:$<TARGET_PDB_FILE:${PROJECT_NAME}>>,copy,true>
        "$<$<BOOL:$<TARGET_PDB_FILE:${PROJECT_NAME}>>:$<TARGET_PDB_FILE:${PROJECT_NAME}>>"
        "${DIST_SKSE_PLUGINS_DIR}/"
    
    COMMENT "Creating distribution folder: dist/PrismaUI_${PROJECT_VERSION}"
)

