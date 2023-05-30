project(CppUnit)

include(ProjectDefaults)
include(ExternalProject)

if(NOT WIN32)
    include(ExternalConfigureArgs)

    set(AUTOCONF_CMD "autoreconf")
    set(AUTOCONF_ARGS "-if")

    set(CONFIGURE_CMD "./configure")
    set(CONFIGURE_ARGS
        "--prefix=${PROJECT_INT_DIST_DIR}"
        "--disable-shared"
        "--enable-static"
        ${CONFIGURE_ARGS_INIT})

    set(BUILD_CMD "make")

    set(INSTALL_CMD "make")
    set(INSTALL_ARGS "install")

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

        CONFIGURE_COMMAND ${AUTOCONF_CMD} ${AUTOCONF_ARGS}
            COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
        BUILD_COMMAND ${BUILD_CMD} ${BUILD_ARGS}
        INSTALL_COMMAND ${INSTALL_CMD} ${INSTALL_ARGS}
    )
else()
    include(ExternalCMakeArgs)

    ExternalProject_Add(
        CppUnit

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "http://dev-www.libreoffice.org/src/cppunit-1.15.1.tar.gz"
        URL_HASH SHA256=89c5c6665337f56fd2db36bc3805a5619709d51fb136e51937072f63fcc717a7
        DOWNLOAD_NAME "cppunit-1.15.1.tar.gz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        PATCH_COMMAND ${PATCH_EXE} -s -p1 < ${CMAKE_CURRENT_LIST_DIR}/cppunit.patch

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
            -DENABLE_SHARED=OFF
            -DENABLE_STATIC=ON
            ${CMAKE_ARGS_INIT}
    )

    ExternalProject_Add_Step(
        CppUnit post-install
        DEPENDEES install
        WORKING_DIRECTORY "${PROJECT_INT_DIST_DIR}/lib"
        COMMAND copy cppunit_s.lib cppunit.lib
    )
endif()

if(ENABLE_STATIC)
    export_static_library(cppunit)
endif()