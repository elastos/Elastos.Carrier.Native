include(ProjectDefaults)


check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

include_directories(
    .
    ../common
    ../../include
    )

list(APPEND LAUNCHER_SOURCES
    launcher.cc)

list(APPEND LAUNCHER_DEPENDS
    libuv
    carrier0
    addons0
    CLI11)

if(ENABLE_SHARED)
    set(LIBS carrier-shared addons-shared)
    set(SYSTEM_LIBS pthread dl)
else()
    set(LIBS carrier-static addons-static)
endif()

add_executable(carrier-launcher ${LAUNCHER_SOURCES})
target_link_libraries(carrier-launcher LINK_PUBLIC ${LIBS} ${SYSTEM_LIBS})
add_dependencies(carrier-launcher ${LAUNCHER_DEPENDS})

if(APPLE AND (${CMAKE_BUILD_TYPE} STREQUAL "Debug"))
    add_custom_command(TARGET carrier-launcher POST_BUILD
        COMMAND /usr/libexec/PlistBuddy -c "Add :com.apple.security.get-task-allow bool true" coredump.entitlements
        COMMAND codesign -s - -f --entitlements coredump.entitlements $<TARGET_FILE:carrier-launcher>
        COMMAND rm -f coredump.entitlements
        VERBATIM)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_custom_target(meerkat-deb
	    COMMAND BINARY_DIR=${CMAKE_BINARY_DIR} SOURCE_DIR=${CMAKE_SOURCE_DIR} DIST_DIR=${CMAKE_INSTALL_PREFIX} make -f ${CMAKE_SOURCE_DIR}/apps/launcher/Makefile
	        DEPENDS carrier-launcher
        COMMENT "Packaging Carrier Meerkat Service Distrubtion")
endif()

install(TARGETS carrier-launcher
    RUNTIME DESTINATION "bin"
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")

install(FILES default.conf
    DESTINATION "etc/carrier-launcher")
