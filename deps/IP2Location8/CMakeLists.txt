project(IP2Location8)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

if(WIN32)
    message(FATAL_ERROR "TODO: not support windows build")
else()
    include(ExternalConfigureArgs)

    set(CONFIGURE_CMD
        autoreconf -i -v --force &&
        ./configure)

    set(CONFIGURE_ARGS
        "--prefix=${PROJECT_INT_DIST_DIR}"
        "--enable-shared=no"
        "--disable-shared"
        "--enable-static=yes"
        ${CONFIGURE_ARGS_INIT})

    set(BUILD_CMD "make")

    set(INSTALL_CMD "make")
    set(INSTALL_ARGS "install")
endif()

ExternalProject_Add(
    IP2Location8

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/chrislim2888/IP2Location-C-Library/archive/refs/tags/8.5.1.tar.gz"
    URL_HASH SHA256=934e09017ab440cb5979c31d279340d8ba6b9271fc7b8ad598f3be903a1afaee
    DOWNLOAD_NAME "IP2Location-C-Library-8.5.1.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    BUILD_IN_SOURCE 1

    PATCH_COMMAND ${PATCH_EXE} -s -p1 < ${CMAKE_CURRENT_LIST_DIR}/ip2location.patch
    CONFIGURE_COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
    BUILD_COMMAND ${BUILD_CMD} ${BUILD_ARGS}
    INSTALL_COMMAND ${INSTALL_CMD} ${INSTALL_ARGS}
)

if(ENABLE_STATIC)
    set(_LIBNAME IP2Location)
    export_static_library(${_LIBNAME})
endif()
