project(libuv)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    libuv

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/libuv/libuv/archive/refs/tags/v1.44.2.tar.gz"
    URL_HASH SHA256=e6e2ba8b4c349a4182a33370bb9be5e23c51b32efb9b9e209d0e8556b73a48da
    DOWNLOAD_NAME "libuv-1.44.2.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
        -DLIBUV_BUILD_TESTS=OFF
        -DLIBUV_BUILD_BENCH=OFF
        ${CMAKE_ARGS_INIT}
)

if(ENABLE_STATIC)
    export_static_library(uv_a)
endif()