function(install_targets_config)
    set(oneValueArgs NAMESPACE LIB_NAME)
    cmake_parse_arguments("ITC" "" "${oneValueArgs}" "" ${ARGN})

    include(CMakePackageConfigHelpers)
    # XXXConfigVersion.cmake
    write_basic_package_version_file(
            "${ITC_LIB_NAME}ConfigVersion.cmake"
            VERSION ${${ITC_LIB_NAME}_VERSION}
            COMPATIBILITY AnyNewerVersion)

    # XXXConfig.cmake
    configure_package_config_file(
            "${ITC_LIB_NAME}Config.cmake.in" "${ITC_LIB_NAME}Config.cmake"
            INSTALL_DESTINATION lib/cmake/${ITC_LIB_NAME}
    )

    # export ${ITC_LIB_NAME}
    install(TARGETS ${ITC_LIB_NAME}
            EXPORT ${ITC_LIB_NAME}Targets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            INCLUDES DESTINATION include)

    # install XXXTargets.cmake
    install(EXPORT ${ITC_LIB_NAME}Targets
            FILE ${ITC_LIB_NAME}Targets.cmake
            NAMESPACE ${ITC_NAMESPACE}::
            DESTINATION lib/cmake/${ITC_LIB_NAME})

    # install XXXConfig.cmake
    install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/${ITC_LIB_NAME}Config.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/${ITC_LIB_NAME}ConfigVersion.cmake
            DESTINATION lib/cmake/${ITC_LIB_NAME})
endfunction()