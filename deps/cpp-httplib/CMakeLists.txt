project(cpp-httplib)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    cpp-httplib

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.12.2.tar.gz"
    URL_HASH SHA256=a7897d052de8fae75817e7a261ae37b89dc4cb8ac74b74458a1f2d2e707cfd03
    DOWNLOAD_NAME "cpp-httplib-0.12.2.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
        -DHTTPLIB_REQUIRE_OPENSSL=OFF
        -DHTTPLIB_TEST=OFF
)
