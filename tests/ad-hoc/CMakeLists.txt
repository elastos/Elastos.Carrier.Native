include(ProjectDefaults)

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

include_directories(
    .
    ../../include
    ../../src/core
    ../common
    ${CARRIER_INT_DIST_DIR}/include)

list(APPEND TEST_SOURCES
    main.cc
    node_tests.cc
    routingtable_tests.cc
    activeproxy_tests.cc
)

list(APPEND ADHOC_DEPENDS
    CppUnit
    cpp-httplib
    carrier0
    addons0)

if(ENABLE_SHARED)
    set(CARRIER_LIB carrier-shared)
else()
    set(CARRIER_LIB carrier-static)
endif()

add_executable(ad-hoc ${TEST_SOURCES})
target_link_libraries(ad-hoc LINK_PUBLIC ${CARRIER_LIB} cppunit pthread dl)
add_dependencies(ad-hoc ${ADHOC_DEPENDS})

enable_testing()
add_test(TEST ad-hoc)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    install(TARGETS ad-hoc
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()
