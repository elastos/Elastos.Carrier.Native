project(carrier-addons CXX)
include(ProjectDefaults)

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

set(INCLUDE_DIR ../../include)

list(APPEND ADDONS_SOURCES
    activeproxy/activeproxy.cc
    activeproxy/connection.cc
    addons.cc
)

list(APPEND ADDONS_HEADERS
    ${INCLUDE_DIR}/carrier/addon.h)

include_directories(
    ${INCLUDE_DIR}
    ./activeproxy)

list(APPEND ADDONS_DEPENDS
    libsodium
    libuv
    carrier0)

set(LIBS
    uv_a)

if(WIN32)
    set(LIBS
        ${LIBS}
        libsodium.lib)
else()
    set(LIBS
        ${LIBS}
        sodium)
endif()

set(SYSTEM_LIBS
    dl)

add_custom_target(addons0)

if(ENABLE_STATIC)
    add_library(addons-static STATIC ${ADDONS_SOURCES} ${ADDONS_HEADERS})
    add_dependencies(addons-static ${ADDONS_DEPENDS})
    add_dependencies(addons0 addons-static)

    target_compile_definitions(addons-static PRIVATE CARRIER_STATIC)
    set_target_properties(addons-static PROPERTIES OUTPUT_NAME addons${STATIC_LIB_DECORATOR})

    target_include_directories(addons-static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}../include>
        $<INSTALL_INTERFACE:include>
    )

    target_link_libraries(addons-static
        INTERFACE ${LIBS} ${SYSTEM_LIBS})

    install(TARGETS addons-static
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

if(ENABLE_SHARED)
    add_library(addons-shared SHARED ${ADDONS_SOURCES} ${ADDONS_HEADERS})
    add_dependencies(addons-shared ${ADDONS_DEPENDS})
    add_dependencies(addons0 addons-shared)

    target_compile_definitions(addons-shared PRIVATE CARRIER_DYNAMIC)
    set_target_properties(addons-shared PROPERTIES
        OUTPUT_NAME addons${SHARED_LIB_DECORATOR}
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR})

    target_include_directories(addons-shared PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}../include>
        $<INSTALL_INTERFACE:include>
    )

    if(NOT APPLE)
        target_link_libraries(addons-shared
            PRIVATE ${LIBS} ${SYSTEM_LIBS})
    else()
        target_link_libraries(addons-shared
            PRIVATE ${LIBS} ${SYSTEM_LIBS}
            SYSTEM "-framework CoreFoundation" "-framework Security")
    endif()

    install(TARGETS addons-shared
        EXPORT ElastosCarrierConfig
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")

endif()

install(FILES ${ADDONS_HEADERS} DESTINATION "include/carrier")
