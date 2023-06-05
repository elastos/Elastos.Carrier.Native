include(ProjectDefaults)

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

check_include_file(dirent.h HAVE_DIRENT_H)
if(HAVE_DIRENT_H)
    add_definitions(-DHAVE_DIRENT_H=1)
endif()

check_include_file(io.h HAVE_IO_H)
if(HAVE_IO_H)
    add_definitions(-DHAVE_IO_H=1)
endif()

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
    add_definitions(-DHAVE_UNISTD_H=1)
endif()

if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_WARNINGS)
endif()

include_directories(
    .
    ../../include
    ../../src/core
    ../common
    ${CARRIER_INT_DIST_DIR}/include)

list(APPEND TEST_SOURCES
    main.cc
    ../common/utils.cc
    node_tests.cc
    routingtable_tests.cc
    activeproxy_tests.cc
)

list(APPEND ADHOC_DEPENDS
    CppUnit
    cpp-httplib
    carrier0)

if(ENABLE_SHARED)
    set(CARRIER_LIB carrier-shared)
else()
    set(CARRIER_LIB carrier-static)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIBS pthread dl)
endif()

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

add_executable(ad-hoc ${TEST_SOURCES})
target_link_libraries(ad-hoc LINK_PUBLIC ${CARRIER_LIB} ${LIBS} ${SYSTEM_LIBS})
add_dependencies(ad-hoc ${ADHOC_DEPENDS})

enable_testing()
add_test(TEST ad-hoc)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    install(TARGETS ad-hoc
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()
