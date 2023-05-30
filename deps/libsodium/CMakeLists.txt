project(libsodium)

include(ProjectDefaults)
include(ExternalProject)

if(NOT WIN32)
    include(ExternalConfigureArgs)

    set(CONFIGURE_CMD "./configure")
    set(CONFIGURE_ARGS
        "--prefix=${PROJECT_INT_DIST_DIR}"
        "--enable-shared=no"
        "--disable-shared"
        "--enable-static"
        "--with-pic=yes"
        "--with-pthreads=yes"
        ${CONFIGURE_ARGS_INIT})

    set(BUILD_CMD "make")

    set(INSTALL_CMD "make")
    set(INSTALL_ARGS "install")

    ExternalProject_Add(
        libsodium

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "https://github.com/jedisct1/libsodium/releases/download/1.0.18-RELEASE/libsodium-1.0.18.tar.gz"
        URL_HASH SHA256=6f504490b342a4f8a4c4a02fc9b866cbef8622d5df4e5452b46be121e46636c1
        DOWNLOAD_NAME "libsodium-1.0.18.tar.gz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
        BUILD_COMMAND ${BUILD_CMD} ${BUILD_ARGS}
        INSTALL_COMMAND ${INSTALL_CMD} ${INSTALL_ARGS}
    )
else()
    include(ExternalCMakeArgs)

    ExternalProject_Add(
        libsodium

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "https://github.com/jedisct1/libsodium/releases/download/1.0.18-RELEASE/libsodium-1.0.18.tar.gz"
        URL_HASH SHA256=6f504490b342a4f8a4c4a02fc9b866cbef8622d5df4e5452b46be121e46636c1
        DOWNLOAD_NAME "libsodium-1.0.18.tar.gz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        PATCH_COMMAND ${PATCH_EXE} -s -p1 < ${CMAKE_CURRENT_LIST_DIR}/libsodium.patch

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
            -DENABLE_SHARED=OFF
            -DENABLE_STATIC=ON
            ${CMAKE_ARGS_INIT}
    )

    ExternalProject_Add_Step(
        libsodium post-install
        DEPENDEES install
        WORKING_DIRECTORY "${PROJECT_INT_DIST_DIR}/lib"
        COMMAND copy libsodium_s.lib libsodium.lib
    )
endif()

if(ENABLE_STATIC)
    export_static_library(libsodium)
endif()
