project(deps_sqlite)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

if(WIN32)
    set(CONFIGURE_CMD "echo")
    set(CONFIGURE_ARGS "Done")

    set(BUILD_CMD nmake)
    set(BUILD_ARGS
        "/f" "Makefile.msc")

    set(INSTALL_CMD
        echo F | xcopy "sqlite3.h" "${PROJECT_INT_DIST_DIR}\\include\\sqlite3.h" /i /q /y &&
        echo F | xcopy "sqlite3.lib" "${PROJECT_INT_DIST_DIR}\\lib\\sqlite3.lib" /i /q /y &&
        echo F | xcopy "sqlite3.dll" "${PROJECT_INT_DIST_DIR}\\bin\\sqlite3.dll" /i /q /y &&
        echo F | xcopy "sqlite3.exe" "${PROJECT_INT_DIST_DIR}\\bin\\sqlite3.exe" /i /q /y)
else()
    set(CONFIGURE_CMD "./configure")
    set(CONFIGURE_ARGS
        "--prefix=${PROJECT_INT_DIST_DIR}"
        "--disable-shared"
        "--enable-static"
        "--with-pic"
        ${CONFIGURE_ARGS_INIT})

    set(INSTALL_CMD
        make install)
endif()

ExternalProject_Add(
    sqlite

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://sqlite.org/2022/sqlite-autoconf-3400000.tar.gz"
    URL_HASH SHA256=0333552076d2700c75352256e91c78bf5cd62491589ba0c69aed0a81868980e7
    DOWNLOAD_NAME "sqlite-autoconf-3400000.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    BUILD_IN_SOURCE 1

    #CONFIGURE_COMMAND  autoreconf -if && ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
    CONFIGURE_COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
    BUILD_COMMAND ${BUILD_CMD} ${BUILD_ARGS}
    INSTALL_COMMAND ${INSTALL_CMD}
)

if(ENABLE_STATIC)
    export_static_library(sqlite3)
endif()
