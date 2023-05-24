include(ProjectDefaults)
include(CheckSymbolExists)

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
    add_definitions(-DHAVE_UNISTD_H=1)
endif()

check_include_file(io.h HAVE_IO_H)
if(HAVE_IO_H)
    add_definitions(-DHAVE_IO_H=1)
endif()

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
    add_definitions(-DHAVE_UNISTD_H=1)
endif()

check_symbol_exists(SIGKILL signal.h HAVE_SIGKILL)
if(HAVE_SIGKILL)
    add_definitions(-DHAVE_SIGKILL=1)
endif()

check_symbol_exists(SIGHUP signal.h HAVE_SIGHUP)
if(HAVE_SIGHUP)
    add_definitions(-DHAVE_SIGHUP=1)
endif()

include_directories(
    .
    ../../include
    ../../src/core
    ../common
    ${CARRIER_INT_DIST_DIR}/include)

list(APPEND APITESTS_SOURCES
    main.cc
    ../common/utils.cc
    messages/ping_tests.cc
    messages/find_node_tests.cc
    messages/store_value_tests.cc
    messages/find_value_tests.cc
    messages/announce_peer_tests.cc
    messages/find_peer_tests.cc
    messages/error_message_tests.cc
    task/closest_candidates_tests.cc
    id_tests.cc
    value_tests.cc
    nodeinfo_tests.cc
    peerinfo_tests.cc
    prefix_tests.cc
    crypto_tests.cc
    log_tests.cc
    storage_tests.cc
    store_find_value_tests.cc
    announce_find_peer_tests.cc
    address_tests.cc
    node_api_tests.cc
)

set(LIBS
    sqlite3
    cppunit)

if(WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_WARNINGS)

    set(LIBS
        ${LIBS}
        libsodium.lib
        Ws2_32
        crypt32
        iphlpapi
        Shlwapi)
else()
    set(LIBS
        ${LIBS}
        sodium)
endif()

list(APPEND APITESTS_DEPENDS
    sqlite
    CppUnit
    carrier0
    addons0
    libsodium)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIBS pthread dl)
endif()

if(ENABLE_SHARED)
    set(LIBS carrier-shared addons-shared ${LIBS})
else()
    set(LIBS carrier-static addons-static ${LIBS})
endif()

add_executable(apitests ${APITESTS_SOURCES})
target_link_libraries(apitests ${LIBS} ${SYSTEM_LIBS})
add_dependencies(apitests ${APITESTS_DEPENDS})

configure_file(log_test.conf log_test.conf COPYONLY)

enable_testing()
add_test(TEST apitests)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    install(TARGETS apitests
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

install(FILES log_test.conf DESTINATION "bin")
