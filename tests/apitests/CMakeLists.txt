include(ProjectDefaults)

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

include_directories(
    .
    ../../include
    ../../src
    ${CARRIER_INT_DIST_DIR}/include)

list(APPEND TEST_SOURCES
    main.cc
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
    node_tests.cc
    address_tests.cc
)

if(ENABLE_SHARED)
    set(CARRIER_LIB carrier-shared)
else()
    set(CARRIER_LIB carrier-static)
endif()

add_executable(apitests ${TEST_SOURCES})
target_link_libraries(apitests LINK_PUBLIC ${CARRIER_LIB} cppunit pthread dl)

configure_file(log_test.conf log_test.conf COPYONLY)

enable_testing()
add_test(TEST apitests)
