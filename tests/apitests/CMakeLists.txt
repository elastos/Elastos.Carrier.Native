include(ProjectDefaults)

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

include_directories(
    .
    ../../include
    ../../src/core
    ${CARRIER_INT_DIST_DIR}/include)

list(APPEND APITESTS_SOURCES
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

list(APPEND APITESTS_DEPENDS
    CppUnit
    carrier0
    addons0)

if(ENABLE_SHARED)
    set(LIBS carrier-shared addons-shared cppunit)
    set(SYSTEM_LIBS pthread dl)
else()
    set(LIBS carrier-static addons-static cppunit)
endif()

add_executable(apitests ${APITESTS_SOURCES})
target_link_libraries(apitests LINK_PUBLIC ${LIBS} ${SYSTEM_LIBS})
add_dependencies(apitests ${APITESTS_DEPENDS})

configure_file(log_test.conf log_test.conf COPYONLY)

enable_testing()
add_test(TEST apitests)

install(TARGETS apitests
    RUNTIME DESTINATION "bin"
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")
