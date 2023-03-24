include(ProjectDefaults)
include(CheckIncludeFile)

if(WIN32)
    add_definitions(
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
        -D_CRT_SECURE_NO_WARNINGS
        -DWIN32_LEAN_AND_MEAN)
endif()

if (ENABLE_CARRIER_DEVELOPMENT)
    add_definitions(-DCARRIER_DEVELOPMENT)
endif()

set(INCLUDE_DIR ../include)

list(APPEND CARRIER_SOURCES
    utils/log.cc
    utils/hex.cc
    utils/socket_address.cc
    utils/json_to_any.cc
    crypto/crypto_core.cc
    crypto/base58.cc
    messages/message.cc
    messages/announce_peer_request.cc
    messages/error_message.cc
    messages/find_peer_response.cc
    messages/find_value_request.cc
    messages/find_value_response.cc
    messages/lookup_request.cc
    messages/lookup_response.cc
    messages/store_value_request.cc
    task/task.cc
    task/task_manager.cc
    task/lookup_task.cc
    task/value_lookup.cc
    task/value_announce.cc
    task/peer_announce.cc
    task/peer_lookup.cc
    task/node_lookup.cc
    task/ping_refresh_task.cc
    task/closest_candidates.cc
    serializers.cc
    id.cc
    node_info.cc
    peer_info.cc
    value.cc
    version.cc
    prefix.cc
    crypto_context.cc
    kbucket_entry.cc
    kclosest_nodes.cc
    kbucket.cc
    routing_table.cc
    dht.cc
    node.cc
    token_manager.cc
    rpccall.cc
    rpcserver.cc
    sqlite_storage.cc
    default_configuration.cc
    constants.cc
)

list(APPEND CARRIER_HEADERS
    ${INCLUDE_DIR}/carrier/node.h
    ${INCLUDE_DIR}/carrier/configuration.h
    ${INCLUDE_DIR}/carrier/id.h
    ${INCLUDE_DIR}/carrier/node_info.h
    ${INCLUDE_DIR}/carrier/peer_info.h
    ${INCLUDE_DIR}/carrier/value.h
    ${INCLUDE_DIR}/carrier/node_status.h
    ${INCLUDE_DIR}/carrier/node_status_listener.h
    ${INCLUDE_DIR}/carrier/lookup_option.h
)

include_directories(
    .
    ${INCLUDE_DIR}
    ${CARRIER_INT_DIST_DIR}/include)

if(WIN32)
    list(APPEND CARRIER_HEADERS compat/msvc/unistd.h)
endif()

list(APPEND CARRIER_DEPENDS
    sqlite
    libsodium
    spdlog
    spdlog-setup
    nlohmann
    LibreSSL
)

list(APPEND LIBS
    sqlite3
    sodium
    crypto
    pthread
    dl)

if(WIN32)
    set(LIBS
        ${LIBS}
        libsodium.lib)
else()
    set(LIBS
        ${LIBS}
        sodium)
endif()

add_custom_target(carrier)

if(ENABLE_STATIC)
    add_library(carrier-static STATIC ${CARRIER_SOURCES} ${CARRIER_HEADERS})
    add_dependencies(carrier-static ${CARRIER_DEPENDS})
    add_dependencies(carrier carrier-static)

    target_compile_definitions(carrier-static PRIVATE CARRIER_BUILD CARRIER_STATIC)
    set_target_properties(carrier-static PROPERTIES OUTPUT_NAME carrier${STATIC_LIB_DECORATOR})

    target_include_directories(carrier-static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}../include>
        $<INSTALL_INTERFACE:include>
    )

    target_link_libraries(carrier-static
        INTERFACE ${LIBS} ${SYSTEM_LIBS})

    install(TARGETS carrier-static
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

if(ENABLE_SHARED)
    add_library(carrier-shared SHARED ${CARRIER_SOURCES} ${CARRIER_HEADERS})
    add_dependencies(carrier-shared ${CARRIER_DEPENDS})
    add_dependencies(carrier carrier-shared)

    target_compile_definitions(carrier-shared PRIVATE CARRIER_BUILD CARRIER_DYNAMIC)
    set_target_properties(carrier-shared PROPERTIES
        OUTPUT_NAME carrier${SHARED_LIB_DECORATOR}
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR})

    #if(WIN32)
    #    set_target_properties(carrier-shared PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
    #endif()

    target_include_directories(carrier-shared PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}../include>
        $<INSTALL_INTERFACE:include>
    )

    if(NOT APPLE)
        target_link_libraries(carrier-shared
            PRIVATE ${LIBS} ${SYSTEM_LIBS})
    else()
        target_link_libraries(carrier-shared
            PRIVATE ${LIBS} ${SYSTEM_LIBS}
            SYSTEM "-framework CoreFoundation" "-framework Security")
    endif()

    install(TARGETS carrier-shared
        EXPORT ElastosCarrierConfig
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")

    install(EXPORT
            ElastosCarrierConfig
        NAMESPACE
            "ElastosCarrier::"
        DESTINATION
            share/ElastosCarrier/cmake
    )

    export(
        TARGETS
            carrier-shared
        FILE
            ElastosCarrierConfig.cmake
    )
endif()

install(FILES ${CARRIER_HEADERS} DESTINATION "include/carrier")
install(FILES "${INCLUDE_DIR}/carrier.h" DESTINATION "include")
