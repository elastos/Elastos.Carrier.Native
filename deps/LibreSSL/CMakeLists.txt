project(LibreSSL)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    LibreSSL

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.7.3.tar.gz"
        "https://cloudflare.cdn.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.7.3.tar.gz"
        "https://mirrors.aliyun.com/pub/OpenBSD/LibreSSL/libressl-3.7.3.tar.gz"
        "https://cdn.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.7.3.tar.gz"
    URL_HASH SHA256=7948c856a90c825bd7268b6f85674a8dcd254bae42e221781b24e3f8dc335db3
    DOWNLOAD_NAME "libressl-3.7.3.tar.gz"
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
    set(SSL_MAJOR_VERSION 53)
    set(CRYPTO_MAJOR_VERSION 50)
    set(TLS_MAJOR_VERSION 26)

    ExternalProject_Add_Step(
        LibreSSL post-install
        DEPENDEES install
        WORKING_DIRECTORY "${PROJECT_INT_DIST_DIR}/lib"
        COMMAND copy ssl-${SSL_MAJOR_VERSION}.lib ssl.lib
        COMMAND copy crypto-${CRYPTO_MAJOR_VERSION}.lib crypto.lib
        COMMAND copy tls-${TLS_MAJOR_VERSION}.lib tls.lib
    )
endif()

if(ENABLE_STATIC)
    export_static_library(ssl)
endif()