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

add_executable(launcher ${LAUNCHER_SOURCES})
target_link_libraries(launcher LINK_PUBLIC ${LIBS} ${SYSTEM_LIBS})
add_dependencies(launcher ${LAUNCHER_DEPENDS})

if(APPLE AND (${CMAKE_BUILD_TYPE} STREQUAL "Debug"))
    add_custom_command(TARGET launcher POST_BUILD
        COMMAND /usr/libexec/PlistBuddy -c "Add :com.apple.security.get-task-allow bool true" coredump.entitlements
        COMMAND codesign -s - -f --entitlements coredump.entitlements $<TARGET_FILE:launcher>
        COMMAND rm -f coredump.entitlements
        VERBATIM)
endif()

install(TARGETS launcher
    RUNTIME DESTINATION "bin"
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")
