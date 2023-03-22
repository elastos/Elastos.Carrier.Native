include(ProjectDefaults)

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

include_directories(
    .
    ../include
    ../addons/activeproxy
    )

list(APPEND LAUNCHER_SOURCES
    launcher.cc
)

if(ENABLE_SHARED)
    set(CARRIER_LIB carrier-shared)
else()
    set(CARRIER_LIB carrier-static)
endif()

add_executable(launcher ${LAUNCHER_SOURCES})
target_link_libraries(launcher LINK_PUBLIC ${CARRIER_LIB} activeproxy pthread dl)
