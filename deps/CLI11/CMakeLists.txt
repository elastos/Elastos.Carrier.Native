project(CLI11)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalConfigureArgs)

ExternalProject_Add(
    CLI11

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/CLIUtils/CLI11/archive/refs/tags/v2.3.2.tar.gz"
    URL_HASH SHA256=aac0ab42108131ac5d3344a9db0fdf25c4db652296641955720a4fbe52334e22
    DOWNLOAD_NAME "CLI11-2.3.2.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
        -DCLI11_BUILD_DOCS=OFF
        -DCLI11_BUILD_EXAMPLES=OFF
        -DCLI11_BUILD_TESTS=OFF
)
