include(ProjectDefaults)

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

include_directories(
    ./activeproxy
    ../include)

list(APPEND ADDONS_SOURCES
    activeproxy/activeproxy.cc
    activeproxy/connection.cc
    addons.cc
)

if(ENABLE_SHARED)
    set(CARRIER_LIB carrier-shared)
else()
    set(CARRIER_LIB carrier-static)
endif()

#add_executable(activeproxy ${ADDONS_SOURCES})
add_library(addons ${ADDONS_SOURCES})
target_link_libraries(addons LINK_PUBLIC ${CARRIER_LIB} uv_a pthread dl sodium)

install(TARGETS addons
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")