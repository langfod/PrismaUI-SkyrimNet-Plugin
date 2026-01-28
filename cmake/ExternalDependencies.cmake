# ExternalDependencies.cmake
# Configures external library subdirectories
#
# This file includes modular configurations for:
#   - CommonLibSSE-NG
# Include individual library configurations

include(commonlibsse)


# Helper function to link all external dependencies to a target
function(link_external_dependencies target_name)


    # Add target-specific include directories (PRIVATE to avoid pollution)
    target_include_directories(${target_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/commonlibsse-ng/include
    )
endfunction()
