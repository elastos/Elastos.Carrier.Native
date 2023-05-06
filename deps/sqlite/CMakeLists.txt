project(deps_sqlite)

include(ProjectDefaults)
include(ExternalProject)

if(WIN32)
    set(AUTOCONF_CMD "echo")
    set(AUTOCONF_ARGS "Done")

    set(CONFIGURE_CMD "echo")
    set(CONFIGURE_ARGS "Done")

    set(BUILD_CMD nmake)
    set(BUILD_ARGS
        "CYGWIN_PATH=${PROJECT_HOST_TOOLS_DIR}\\usr\\bin" "/f" "Makefile.msc")

    set(INSTALL_CMD
        echo F | xcopy "sqlite3.h" "${PROJECT_INT_DIST_DIR}\\include\\sqlite3.h" /i /q /y &&
        echo F | xcopy "sqlite3.lib" "${PROJECT_INT_DIST_DIR}\\lib\\sqlite3.lib" /i /q /y &&
        echo F | xcopy "sqlite3.dll" "${PROJECT_INT_DIST_DIR}\\bin\\sqlite3.dll" /i /q /y &&
        echo F | xcopy "sqlite3.exe" "${PROJECT_INT_DIST_DIR}\\bin\\sqlite3.exe" /i /q /y)

else()
    include(ExternalConfigureArgs)

if(${CMAKE_CROSSCOMPILING} AND IOS)
    set(SHELL_CFLAGS_FOR_IOS "SHELL_CFLAGS=-DSQLITE_NOHAVE_SYSTEM")
endif()

    set(AUTOCONF_CMD "autoreconf")
    set(AUTOCONF_ARGS "-if")

    set(CONFIGURE_CMD "./configure")
    set(CONFIGURE_ARGS
        "--prefix=${PROJECT_INT_DIST_DIR}"
        "--disable-shared"
        "--enable-static"
        "--with-pic"
        "--disable-editline"
        "--disable-readline"
        "--disable-debug"
        ${SHELL_CFLAGS_FOR_IOS}
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

    PATCH_COMMAND ${PATCH_EXE} -s -p1 < ${CMAKE_CURRENT_LIST_DIR}/sqlite.patch

    CONFIGURE_COMMAND ${AUTOCONF_CMD} ${AUTOCONF_ARGS}
            COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
    BUILD_COMMAND ${BUILD_CMD} ${BUILD_ARGS}
    INSTALL_COMMAND ${INSTALL_CMD}
)

if(ENABLE_STATIC)
    export_static_library(sqlite3)
endif()
