project(spdlog)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    spdlog

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/gabime/spdlog/archive/refs/tags/v1.10.0.tar.gz"
    URL_HASH SHA256=697f91700237dbae2326b90469be32b876b2b44888302afbc7aceb68bcfe8224
    DOWNLOAD_NAME "spdlog-1.10.0.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
        ${CMAKE_ARGS_INIT}
)

if(ENABLE_STATIC)
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        export_static_library(spdlogd)
    else()
        export_static_library(spdlog)
    endif()
endif()