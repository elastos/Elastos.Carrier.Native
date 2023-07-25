project(libutf8proc)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    libutf8proc

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/JuliaStrings/utf8proc/archive/refs/tags/v2.6.1.tar.gz"
    URL_HASH SHA256=4c06a9dc4017e8a2438ef80ee371d45868bda2237a98b26554de7a95406b283b
    DOWNLOAD_NAME "utf8proc-2.6.1.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
        ${CMAKE_ARGS_INIT}
)

if(ENABLE_STATIC)
    export_static_library(utf8proc)
endif()

if(WIN32)
    ExternalProject_Add_Step(
        libutf8proc post-install
        DEPENDEES install
        WORKING_DIRECTORY "${PROJECT_INT_DIST_DIR}/lib"
        COMMAND copy utf8proc_static.lib utf8proc.lib /y
    )
endif()