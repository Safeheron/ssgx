# ============================================================================================
# ssgx-init.cmake
# This script checks for minimum required CMake version and initializes global link group definitions and other
# global build settings for Safeheron SGX projects
# ============================================================================================

# Check for minimum required CMake version.
# The $<LINK_GROUP:...> generator expression, which is essential for robustly handling
# circular dependencies, was introduced in CMake 3.24. This check fails fast with
# a clear error message if the user's environment is not supported.
if (CMAKE_VERSION VERSION_LESS 3.24)
    message(FATAL_ERROR "Safeheron SGX build scripts require CMake 3.24 or newer. Current version is ${CMAKE_VERSION}.")
endif ()
message(STATUS "CMake version: ${CMAKE_VERSION}")

# Define custom link group
set(CMAKE_LINK_GROUP_USING_ssgx_wrap_SUPPORTED TRUE CACHE INTERNAL "")
set(CMAKE_LINK_GROUP_USING_ssgx_wrap_C   TRUE CACHE INTERNAL "")
set(CMAKE_LINK_GROUP_USING_ssgx_wrap_CXX TRUE CACHE INTERNAL "")
set(CMAKE_LINK_GROUP_USING_ssgx_wrap
        "LINKER:--start-group"
        "LINKER:--end-group"
        CACHE INTERNAL ""
)

