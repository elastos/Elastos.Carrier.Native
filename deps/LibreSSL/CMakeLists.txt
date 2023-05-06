project(LibreSSL)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    LibreSSL

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.3.3.tar.gz"
        "https://cloudflare.cdn.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.3.3.tar.gz"
        "https://mirrors.aliyun.com/pub/OpenBSD/LibreSSL/libressl-3.3.3.tar.gz"
        "https://cdn.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.3.3.tar.gz"
    URL_HASH SHA256=a471565b36ccd1a70d0bd7d37c6e95c43a26a62829b487d9d2cdebfe58be3066
    DOWNLOAD_NAME "libressl-3.3.3.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
        -DLIBRESSL_SKIP_INSTALL=OFF
        -DLIBRESSL_APPS=OFF
        -DLIBRESSL_TESTS=OFF
        -DBUILD_SHARED_LIBS=OFF
        -DENABLE_ASM=OFF
        -DENABLE_EXTRATESTS=OFF
        -DENABLE_NC=OFF
        ${CMAKE_ARGS_INIT}
)

if(WIN32)
    file(READ ${CMAKE_CURRENT_BINARY_DIR}/external/src/LibreSSL/ssl/VERSION SSL_VERSION)
    string(STRIP ${SSL_VERSION} SSL_VERSION)
    string(REPLACE ":" "." SSL_VERSION ${SSL_VERSION})
    string(REGEX REPLACE "\\..*" "" SSL_MAJOR_VERSION ${SSL_VERSION})

    message("==========================")
    message("${SSL_MAJOR_VERSION}")
    message("==========================")

    ExternalProject_Add_Step(
        LibreSSL post-install
        DEPENDEES install
        WORKING_DIRECTORY "${PROJECT_INT_DIST_DIR}/lib"
        COMMAND copy ssl-${SSL_MAJOR_VERSION}.lib ssl.lib
    )
endif()

if(ENABLE_STATIC)
    export_static_library(ssl)
endif()