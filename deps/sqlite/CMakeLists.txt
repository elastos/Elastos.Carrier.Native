project(sqlite)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

set(CONFIGURE_CMD "./configure")
set(CONFIGURE_ARGS
    "--prefix=${PROJECT_INT_DIST_DIR}"
    "--disable-shared"
    "--enable-static"
    "--with-pic"
    ${CONFIGURE_ARGS_INIT})

ExternalProject_Add(
    sqlite

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://sqlite.org/2022/sqlite-autoconf-3400000.tar.gz"
    URL_HASH SHA256=0333552076d2700c75352256e91c78bf5cd62491589ba0c69aed0a81868980e7
    DOWNLOAD_NAME "sqlite-autoconf-3400000.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    BUILD_IN_SOURCE 1

    CONFIGURE_COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
)

if(ENABLE_STATIC)
    export_static_library(sqlite3)
endif()
