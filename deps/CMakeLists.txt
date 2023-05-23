include(AddSubModule)

add_submodule(platform-specific)

add_submodule(libsodium
    DEPENDS platform-specific)

add_submodule(LibreSSL
    DEPENDS platform-specific)

add_submodule(sqlite
    DEPENDS platform-specific)

add_submodule(nlohmann
    DEPENDS platform-specific)

add_submodule(spdlog
    DEPENDS platform-specific)

add_submodule(spdlog-setup
    DEPENDS spdlog platform-specific)

if (ENABLE_ADDONS)
    add_submodule(libuv
        DEPENDS platform-specific)
endif()

if (ENABLE_APPS)
    add_submodule(editline
        DEPENDS platform-specific)

    add_submodule(CLI11
        DEPENDS platform-specific)

    add_submodule(libutf8proc
        DEPENDS platform-specific)
endif()

if(ENABLE_CARRIER_CRAWLER)
    add_submodule(IP2Location8
        DEPENDS platform-specific)
endif()

if (ENABLE_TESTS)
    add_submodule(CppUnit
        DEPENDS platform-specific)

    add_submodule(cpp-httplib
        DEPENDS platform-specific)
endif()
