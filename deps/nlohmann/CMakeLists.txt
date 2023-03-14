project(nlohmann)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    nlohmann

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/nlohmann/json/archive/refs/tags/v3.10.5.tar.gz"
    URL_HASH SHA256=5daca6ca216495edf89d167f808d1d03c4a4d929cef7da5e10f135ae1540c7e4
    DOWNLOAD_NAME "json.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR} -DJSON_BuildTests=OFF
)
