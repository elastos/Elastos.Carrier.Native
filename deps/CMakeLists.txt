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

add_submodule(libuv
    DEPENDS platform-specific)

if (ENABLE_TESTS)
    add_submodule(CppUnit
        DEPENDS platform-specific)
endif()
