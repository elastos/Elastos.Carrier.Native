include(ProjectDefaults)

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

include_directories(
    .
    ../../include)

list(APPEND ACTIVEPROXY_SOURCES
    activeproxy.cc
    connection.cc
    main.cc
)

if(ENABLE_SHARED)
    set(CARRIER_LIB carrier-shared)
else()
    set(CARRIER_LIB carrier-static)
endif()

add_executable(activeproxy ${ACTIVEPROXY_SOURCES})
target_link_libraries(activeproxy LINK_PUBLIC ${CARRIER_LIB} uv_a pthread dl sodium)
