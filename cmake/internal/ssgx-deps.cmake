include_guard(GLOBAL)

# ============================================================================================
# Function: collect_dependencies
# Description:
#   Recursively collects all transitive dependencies for a given CMake target.
#   It walks through INTERFACE_LINK_LIBRARIES to find all linked libraries or flags.
#
# Arguments:
#   - target       : The name of the CMake target to analyze.
#   - output_list  : The variable name (not value) to store the resulting dependency list.
#
# Notes:
#   - Supports both CMake targets and "-lxxx" style link flags.
#   - Ignores compiler flags such as -O2 or -m64.
#
# Example usage:
#   collect_dependencies(my_target MY_DEPS)
#   # -> Sets MY_DEPS = [ list of all recursive dependencies of 'my_target' ]
# ============================================================================================
function(ssgx_collect_dependencies target output_list)
    set(collected_deps "")

    if (IS_ABSOLUTE "${target}" AND EXISTS "${target}")
        list(APPEND collected_deps ${target})
        set(${output_list} ${collected_deps} PARENT_SCOPE)
        return()
    endif()

    if(${target} MATCHES "^-l")
        list(APPEND collected_deps ${dep})
        return()
    endif()

    get_target_property(deps ${target} INTERFACE_LINK_LIBRARIES)
    if (deps)
        foreach(dep ${deps})
            if (TARGET ${dep})
                list(APPEND collected_deps ${dep})
                set(sub_deps "")
                ssgx_collect_dependencies(${dep} sub_deps)
                list(APPEND collected_deps ${sub_deps})
            elseif(dep MATCHES "^-l")
                list(APPEND collected_deps ${dep})
            elseif(NOT dep MATCHES "^-")
                list(APPEND collected_deps ${dep})
            endif()
        endforeach()
    endif()

    set(${output_list} ${collected_deps} PARENT_SCOPE)
endfunction()


# ============================================================================================
# Function: ssgx_get_all_dependencies
# Description:
#   Collects and deduplicates the transitive dependencies of one or more top-level libraries.
#
# Arguments:
#   - output_list  : The name of the variable (not value) to store the result.
#   - ARGN         : One or more top-level targets to process.
#
# Example usage:
#   ssgx_get_all_dependencies(ALL_DEPS crypto_base crypto_math thirdparty_foo)
#   # -> Sets ALL_DEPS = [ crypto_base + all its deps + crypto_math + ... (no duplicates) ]
# ============================================================================================
function(ssgx_get_all_dependencies output_list)
    set(all_dependencies "")
    # Iterate over all provided libraries
    foreach(lib ${ARGN})
        # Collect dependencies for each library
        ssgx_collect_dependencies(${lib} lib_deps)
        # Append the dependencies to the final list
        list(APPEND all_dependencies ${lib_deps})
    endforeach()

    # Remove duplicate entries to avoid redundant linking
    list(REMOVE_DUPLICATES all_dependencies)
    # Return the full list of dependencies to the parent scope
    set(${output_list} ${all_dependencies} PARENT_SCOPE)
endfunction()
