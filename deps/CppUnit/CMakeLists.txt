project(CppUnit)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalConfigureArgs)

if(WIN32)
    set_win_build_options(WIN_BUILD_OPTIONS "")

    if("${CMAKE_BUILD_TYPE}" MATCHES "(Debug|RelWithDebInfo)")
        set(BUILD_SUFFIX "d")
    endif()

    set(CONF_CMD "")

    set(CONFIGURE_CMD "echo")
    set(CONFIGURE_ARGS "Done")

    set(BUILD_CMD "msbuild")
    #set(BUILD_ARGS ${WIN_BUILD_OPTIONS} "src/cppunit/cppunit_dll.vcxproj")
    set(BUILD_ARGS ${WIN_BUILD_OPTIONS} "src/cppunit/cppunit.vcxproj")

    set(INSTALL_CMD
        xcopy "include\\cppunit" "${PROJECT_INT_DIST_DIR}\\include\\cppunit" /e /i /h /y &&
        copy "lib\\cppunit${BUILD_SUFFIX}.lib" "${PROJECT_INT_DIST_DIR}\\lib\\cppunit.lib" /y)
else()
    set(CONF_CMD "autoreconf -if")

    set(CONFIGURE_CMD "./configure")
    set(CONFIGURE_ARGS
        "--prefix=${PROJECT_INT_DIST_DIR}"
        "--disable-shared"
        "--enable-static"
        ${CONFIGURE_ARGS_INIT})

    set(BUILD_CMD "make")

    set(INSTALL_CMD "make")
    set(INSTALL_ARGS "install")
endif()

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

    COMMAND ${CONF_CMD}
    CONFIGURE_COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
    BUILD_COMMAND ${BUILD_CMD} ${BUILD_ARGS}
    INSTALL_COMMAND ${INSTALL_CMD} ${INSTALL_ARGS}
)
