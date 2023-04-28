project(editline)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalConfigureArgs)

if(WIN32)
    ExternalProject_Add(
        editline

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "https://github.com/winlibs/wineditline/archive/refs/tags/WinEditLine-2.206.tar.gz"
        URL_HASH SHA256=9ddd504cd2885f2f7a2220d123026db5a434d56da93476c71d45781e7b60cac0
        DOWNLOAD_NAME "WinEditLine-2.206.tar.gz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        PATCH_COMMAND ${PATCH_EXE} -s -p1 < ${CMAKE_CURRENT_LIST_DIR}/editline.patch

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
                ${CMAKE_ARGS_INIT}
    )
else()
    set(CONFIGURE_CMD "./configure")
    set(CONFIGURE_ARGS
        "--prefix=${PROJECT_INT_DIST_DIR}"
        "--disable-shared"
        "--enable-static"
        "--with-pic"
        ${CONFIGURE_ARGS_INIT})

    ExternalProject_Add(
        editline

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "https://www.thrysoee.dk/editline/libedit-20221030-3.1.tar.gz"
        URL_HASH SHA256=f0925a5adf4b1bf116ee19766b7daa766917aec198747943b1c4edf67a4be2bb
        HTTP_HEADER "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 13_3) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/16.4 Safari/605.1.15"
        DOWNLOAD_NAME "libedit-20221030-3.1.tar.gz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
    )
endif()
