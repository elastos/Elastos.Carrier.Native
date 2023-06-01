project(platform-specific)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalConfigureArgs)

if(ANDROID)
    set(MAKE_STANDALONE_TOOLCHAIN
        "${CMAKE_ANDROID_NDK}/build/tools/make_standalone_toolchain.py")

    if(NOT EXISTS ${MAKE_STANDALONE_TOOLCHAIN})
        message(FATAL_ERROR
            "Android NDK: make_standalone_toolchain.py is missing.")
    endif()

    if(${CMAKE_ANDROID_STL_TYPE} MATCHES "^c\\+\\+_")
        set(STL_LIBRARY "libc++")
    elseif(${CMAKE_ANDROID_STL_TYPE} MATCHES "^stlport_")
        set(STL_LIBRARY "stlport")
    else()
        set(STL_LIBRARY "gnustl")
    endif()

    ExternalProject_Add(
        android-toolchain

        SOURCE_DIR ${CMAKE_ANDROID_NDK}
        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND ${MAKE_STANDALONE_TOOLCHAIN}
            --force
            --verbose
            --arch ${CMAKE_ANDROID_ARCH}
            --api ${CMAKE_SYSTEM_VERSION}
            --stl ${STL_LIBRARY}
            --install-dir ${XDK_TOOLCHAIN}
    )

    add_custom_target(platform-specific
        COMMENT "Build platform-specific: Android Standalone Toolchain"
        COMMAND echo Done)
    add_dependencies(platform-specific
        android-toolchain
        flatcc-parser)
elseif(WIN32)
    set(CYGWIN_RUNTIME_HASH "821840842061c1cfc734a0bcfb607bd7")
    set(CYGWIN_LIBATTR_HASH "486d0440f560d516879bfb22317f805d")
    set(CYGWIN_LIBICONV_HASH "af70e0b4817c78e9d9db93de32665d90")
    set(CYGWIN_LIBSIGSEGV_HASH "3C74C7599DCEAAF18EE182F54F66BBCC")
    set(CYGWIN_LIBINTL8_HASH "01e53b522f96dac03e19dfc489626a33")
    set(CYGWIN_PCRE2_HASH "069e2f9115b839fa3a22613717c3effe")
    set(CYGWIN_GMP10_HASH "1ad0e65cb6eeba404fb7da22c8d48ee6")
    set(CYGWIN_MPFR6_HASH "7ba134f28d1a7583f254cfddbaa2166b")
    set(CYGWIN_NCURESESW_HASH "c27315d1ea3f01910696a2b22f410e91")
    set(CYGWIN_GCC_HASH "4465a5bcf03586c0734f4adeec9115f0")
    set(CYGWIN_READLINE7_HASH "6bced7ea9c1b5ad1e8fb17d62a431cc9")
    set(CYGWIN_PATCH_HASH "c254e28e0687685e74b8213af0b2c6c0")
    set(CYGWIN_DIFF_HASH "7a2adbf1d2c3fda1d7471e799c1eb47b")
    set(CYGWIN_GREP_HASH "11903e139a6d670f802b65768e054619")
    set(CYGWIN_GAWK_HASH "8815ecae4648abd44b9465e2efd51ad1")

    set(URL_PREFIX "http://mirrors.kernel.org/sourceware/cygwin/x86_64/release")

    ExternalProject_Add(
        cygwin-runtime

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/cygwin/cygwin-3.4.6-1.tar.xz"
        URL_HASH MD5=${CYGWIN_RUNTIME_HASH}
        DOWNLOAD_NAME "cygwin-3.4.6-1.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND if not exist ${PROJECT_HOST_TOOLS_DIR} mkdir ${PROJECT_HOST_TOOLS_DIR}
                COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}
    )

    ExternalProject_Add(
        cygwin-libattr

        DEPENDS cygwin-runtime

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/attr/libattr1/libattr1-2.5.1-1.20.g0981a7bfe487.tar.xz"
        URL_HASH MD5=${CYGWIN_LIBATTR_HASH}
        DOWNLOAD_NAME "libattr1-2.5.1-1.20.g0981a7bfe487.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}
    )

    ExternalProject_Add(
        cygwin-libiconv

        DEPENDS cygwin-runtime

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/libiconv/libiconv2/libiconv2-1.17-1.tar.xz"
        URL_HASH MD5=${CYGWIN_LIBICONV_HASH}
        DOWNLOAD_NAME "libiconv2-1.17-1.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-libsigsegv

        DEPENDS cygwin-runtime

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/libsigsegv/libsigsegv2/libsigsegv2-2.10-2.tar.xz"
        URL_HASH MD5=${CYGWIN_LIBSIGSEGV_HASH}
        DOWNLOAD_NAME "libsigsegv2-2.10-2.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-libintl8

        DEPENDS cygwin-runtime

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/gettext/libintl8/libintl8-0.21.1-2.tar.xz"
        URL_HASH MD5=${CYGWIN_LIBINTL8_HASH}
        DOWNLOAD_NAME "libintl8-0.21.1-2.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-pcre2

        DEPENDS cygwin-runtime

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/pcre2/libpcre2_8_0/libpcre2_8_0-10.42-1.tar.zst"
        URL_HASH MD5=${CYGWIN_PCRE2_HASH}
        DOWNLOAD_NAME "libpcre2_8_0-10.42-1.tar"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-gmp10

        DEPENDS cygwin-runtime

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/gmp/libgmp10/libgmp10-6.2.1-2.tar.zst"
        URL_HASH MD5=${CYGWIN_GMP10_HASH}
        DOWNLOAD_NAME "libgmp10-6.2.1-2.tar"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-ncursesw

        DEPENDS cygwin-runtime

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/ncurses/libncursesw10/libncursesw10-6.4-5.20230514.tar.xz"
        URL_HASH MD5=${CYGWIN_NCURESESW_HASH}
        DOWNLOAD_NAME "libncursesw10-6.4-5.20230514.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-gcc

        DEPENDS cygwin-runtime

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/gcc/libgcc1/libgcc1-11.3.1+20230310-0.1.tar.zst"
        URL_HASH MD5=${CYGWIN_GCC_HASH}
        DOWNLOAD_NAME "libgcc1-11.3.1+20230310-0.1.tar"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-readline7

        DEPENDS cygwin-runtime cygwin-ncursesw

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/readline/libreadline7/libreadline7-8.2-2.tar.xz"
        URL_HASH MD5=${CYGWIN_READLINE7_HASH}
        DOWNLOAD_NAME "libreadline7-8.2-2.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-mpfr6

        DEPENDS cygwin-runtime cygwin-gmp10 cygwin-gcc

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/mpfr/libmpfr6/libmpfr6-4.2.0-1.tar.zst"
        URL_HASH MD5=${CYGWIN_MPFR6_HASH}
        DOWNLOAD_NAME "libmpfr6-4.2.0-1.tar"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-patch

        DEPENDS cygwin-runtime cygwin-libattr cygwin-libintl8 cygwin-libiconv

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/patch/patch-2.7.6-17.tar.xz"
        URL_HASH MD5=${CYGWIN_PATCH_HASH}
        DOWNLOAD_NAME "patch-2.7.6-17.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-diff

        DEPENDS cygwin-runtime cygwin-libattr cygwin-libintl8 cygwin-libiconv

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/diffutils/diffutils-3.9-1.tar.xz"
        URL_HASH MD5=${CYGWIN_DIFF_HASH}
        DOWNLOAD_NAME "diffutils-3.9-1.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-gawk

        DEPENDS cygwin-runtime cygwin-libattr cygwin-libintl8 cygwin-libiconv cygwin-gmp10 cygwin-mpfr6 cygwin-readline7

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/gawk/gawk-5.2.1-2.tar.xz"
        URL_HASH MD5=${CYGWIN_GAWK_HASH}
        DOWNLOAD_NAME "gawk-5.2.1-2.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y .\\usr ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-grep

        DEPENDS cygwin-runtime cygwin-libattr cygwin-libintl8 cygwin-libiconv cygwin-pcre2

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/grep/grep-3.9-1.tar.xz"
        URL_HASH MD5=${CYGWIN_GREP_HASH}
        DOWNLOAD_NAME "grep-3.9-1.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    add_custom_target(platform-specific
        COMMENT "Build platform-specific: Cygwin Runtime"
        COMMAND echo Done
    )
    add_dependencies(platform-specific
        cygwin-grep
        cygwin-gawk
        cygwin-patch
        cygwin-diff
    )
else()
    add_custom_target(platform-specific
        COMMENT "Build platform-specific: None"
        COMMAND echo Done
    )
endif()
