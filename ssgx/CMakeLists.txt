project(ssgx VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 11)
set(SSGX_EDL_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../common/include/ /opt/safeheron/ssgx/include/mbedtls)
include(../cmake/ssgx-build.cmake)

add_subdirectory(log/t)
add_subdirectory(log/u)
add_subdirectory(utils/t)
add_subdirectory(utils/u)
add_subdirectory(json/t)
add_subdirectory(decimal/t)
add_subdirectory(attestation/t)
add_subdirectory(attestation/u)
add_subdirectory(config/t)
add_subdirectory(config/u)
add_subdirectory(filesystem/t)
add_subdirectory(filesystem/u)
add_subdirectory(http/t)
add_subdirectory(http/u)

include(CMakePackageConfigHelpers)
# XXXConfigVersion.cmake
write_basic_package_version_file(
        "ssgxConfigVersion.cmake"
        VERSION ${ssgx_VERSION}
        COMPATIBILITY AnyNewerVersion)
# Install ssgxConfig.cmake
install(FILES ssgxConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ssgxConfigVersion.cmake
        DESTINATION lib/cmake/ssgx)

install(DIRECTORY ../common/include/
        DESTINATION include)

install(FILES ../cmake/ssgx-build.cmake DESTINATION lib/cmake/ssgx)
install(DIRECTORY ../cmake/internal DESTINATION lib/cmake/ssgx)
install(DIRECTORY ../cmake/entrypoints DESTINATION lib/cmake/ssgx)
