project(deps_sqlite)

include(ProjectDefaults)
include(ExternalProject)

if(NOT WIN32)
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

    ExternalProject_Add(
        sqlite

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "https://sqlite.org/2022/sqlite-autoconf-3400000.tar.gz"
        URL_HASH SHA256=0333552076d2700c75352256e91c78bf5cd62491589ba0c69aed0a81868980e7
        DOWNLOAD_NAME "sqlite-autoconf-3400000.tar.gz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND ${AUTOCONF_CMD} ${AUTOCONF_ARGS}
                COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
        BUILD_COMMAND ${BUILD_CMD} ${BUILD_ARGS}
        INSTALL_COMMAND ${INSTALL_CMD}
    )

    if(ENABLE_STATIC)
        export_static_library(sqlite3)
    endif()
else()
    include(ExternalCMakeArgs)

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

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
            -DENABLE_SHARED=OFF
            -DENABLE_STATIC=ON
            ${CMAKE_ARGS_INIT}
    )

    ExternalProject_Add_Step(
        sqlite post-install
        DEPENDEES install
        WORKING_DIRECTORY "${PROJECT_INT_DIST_DIR}/lib"
        COMMAND copy sqlite3_s.lib sqlite3.lib
    )
endif()

if(ENABLE_STATIC)
    export_static_library(sqlites)
endif()