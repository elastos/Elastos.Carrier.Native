project(CppUnit)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalConfigureArgs)

set(CONFIGURE_CMD "./configure")
set(CONFIGURE_ARGS
    "--prefix=${PROJECT_INT_DIST_DIR}"
    "--disable-shared"
    "--enable-static"
    ${CONFIGURE_ARGS_INIT})

ExternalProject_Add(
    CppUnit

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "http://dev-www.libreoffice.org/src/cppunit-1.15.1.tar.gz"
    URL_HASH SHA256=89c5c6665337f56fd2db36bc3805a5619709d51fb136e51937072f63fcc717a7
    DOWNLOAD_NAME "cppunit-1.15.1.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1
    LOG_DOWNLOAD FALSE

    BUILD_IN_SOURCE 1

    CONFIGURE_COMMAND  autoreconf -if && ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
)
