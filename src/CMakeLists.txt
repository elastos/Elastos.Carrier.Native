project(carrier-core CXX)

include(ProjectDefaults)
include(CheckIncludeFile)

add_definitions(-DSODIUM_STATIC)

if(WIN32)
    add_definitions(
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
        -D_CRT_SECURE_NO_WARNINGS
        -DWIN32_LEAN_AND_MEAN)
endif()

if (ENABLE_CARRIER_DEVELOPMENT)
    add_definitions(-DCARRIER_DEVELOPMENT)
endif()

if (ENABLE_CARRIER_CRAWLER)
    add_definitions(-DCARRIER_CRAWLER)
endif()

set(INCLUDE_DIR ../include)

list(APPEND CARRIER_SOURCES
    core/utils/addr.cc
    core/utils/blob.cc
    core/utils/log.cc
    core/utils/hex.cc
    core/utils/socket_address.cc
    core/utils/json_to_any.cc
    core/crypto/crypto_core.cc
    core/crypto/base58.cc
    core/messages/message.cc
    core/messages/announce_peer_request.cc
    core/messages/error_message.cc
    core/messages/find_peer_response.cc
    core/messages/find_value_request.cc
    core/messages/find_value_response.cc
    core/messages/lookup_request.cc
    core/messages/lookup_response.cc
    core/messages/store_value_request.cc
    core/task/task.cc
    core/task/task_manager.cc
    core/task/lookup_task.cc
    core/task/value_lookup.cc
    core/task/value_announce.cc
    core/task/peer_announce.cc
    core/task/peer_lookup.cc
    core/task/node_lookup.cc
    core/task/ping_refresh_task.cc
    core/task/closest_candidates.cc
    core/serializers.cc
    core/id.cc
    core/node_info.cc
    core/peer_info.cc
    core/value.cc
    core/version.cc
    core/prefix.cc
    core/crypto_context.cc
    core/kbucket_entry.cc
    core/kclosest_nodes.cc
    core/kbucket.cc
    core/routing_table.cc
    core/dht.cc
    core/node.cc
    core/token_manager.cc
    core/rpccall.cc
    core/rpcserver.cc
    core/sqlite_storage.cc
    core/default_configuration.cc
    core/constants.cc
)

list(APPEND CARRIER_HEADERS
    ${INCLUDE_DIR}/carrier/addon.h
    ${INCLUDE_DIR}/carrier/blob.h
    ${INCLUDE_DIR}/carrier/def.h
    ${INCLUDE_DIR}/carrier/types.h
    ${INCLUDE_DIR}/carrier/crypto_core.h
    ${INCLUDE_DIR}/carrier/socket_address.h
    ${INCLUDE_DIR}/carrier/prefix.h
    ${INCLUDE_DIR}/carrier/id.h
    ${INCLUDE_DIR}/carrier/configuration.h
    ${INCLUDE_DIR}/carrier/default_configuration.h
    ${INCLUDE_DIR}/carrier/lookup_option.h
    ${INCLUDE_DIR}/carrier/node_info.h
    ${INCLUDE_DIR}/carrier/peer_info.h
    ${INCLUDE_DIR}/carrier/value.h
    ${INCLUDE_DIR}/carrier/version.h
    ${INCLUDE_DIR}/carrier/node_status.h
    ${INCLUDE_DIR}/carrier/node_status_listener.h
    ${INCLUDE_DIR}/carrier/node.h
)

include_directories(
    ${INCLUDE_DIR}
    ./core
    ./addons/activeproxy
    )

list(APPEND CARRIER_DEPENDS
    sqlite
    libsodium
    spdlog
    spdlog-setup
    nlohmann
)

set(LIBS
    sqlite3)

if(WIN32)
    set(LIBS
        ${LIBS}
        libsodium.lib
        Ws2_32
        crypt32
        Rpcrt4
        iphlpapi
        Shlwapi)
else()
    set(LIBS
        ${LIBS}
        sodium)
endif()

if(ENABLE_ADDONS)
    list(APPEND CARRIER_SOURCES
        addons/activeproxy/activeproxy.cc
        addons/activeproxy/connection.cc
        addons/addons.cc
    )

    list(APPEND CARRIER_DEPENDS
        libuv)

    set(LIBS
        ${LIBS}
        uv_a)

if(WIN32)
    set(LIBS
        ${LIBS}
        Userenv
        )
endif()

endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIBS pthread dl)
endif()

add_custom_target(carrier0)

if(ENABLE_STATIC)
    add_library(carrier-static STATIC ${CARRIER_SOURCES} ${CARRIER_HEADERS})
    add_dependencies(carrier-static ${CARRIER_DEPENDS})
    add_dependencies(carrier0 carrier-static)

    target_compile_definitions(carrier-static PRIVATE CARRIER_BUILD CARRIER_STATIC)
    set_target_properties(carrier-static PROPERTIES OUTPUT_NAME carrier${STATIC_LIB_DECORATOR})

    target_include_directories(carrier-static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}../include>
        $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(carrier-static INTERFACE ${LIBS} ${SYSTEM_LIBS})

    install(TARGETS carrier-static
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

if(ENABLE_SHARED)
    add_library(carrier-shared SHARED ${CARRIER_SOURCES} ${CARRIER_HEADERS})
    add_dependencies(carrier-shared ${CARRIER_DEPENDS})
    add_dependencies(carrier0 carrier-shared)

    target_compile_definitions(carrier-shared PRIVATE CARRIER_BUILD CARRIER_DYNAMIC)
    set_target_properties(carrier-shared PROPERTIES
        OUTPUT_NAME carrier${SHARED_LIB_DECORATOR}
        VERSION ${CMAKE_PROJECT_VERSION}
        SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

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
endif()

install(FILES ${CARRIER_HEADERS} DESTINATION "include/carrier")
install(FILES "${INCLUDE_DIR}/carrier.h" DESTINATION "include")
