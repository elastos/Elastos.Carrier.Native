project(spdlog-setup)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    spdlog-setup

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/guangie88/spdlog_setup/archive/refs/tags/v0.3.2.tar.gz"
    URL_HASH SHA256=08229cd17178f939d37de1f6d93d8983b6d1696e57c1359a967dddc65bc16a1e
    DOWNLOAD_NAME "spdlog_setup-0.3.2.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
        ${CMAKE_ARGS_INIT}
)
