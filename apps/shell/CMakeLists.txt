project(carrier-shell CXX)
include(ProjectDefaults)

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

include_directories(
    .
    ../common
    ../../include
    ../../src
    ${CARRIER_INT_DIST_DIR}/include)

list(APPEND SHELL_SOURCES
    shell.cc
)

list(APPEND SHELL_DEPENDS
    CLI11
    libutf8proc
    editline
    carrier0
    addons0)

if(ENABLE_SHARED)
    set(LIBS carrier-shared addons-shared utf8proc edit ncurses)
    set(SYSTEM_LIBS pthread dl)
else()
    set(LIBS carrier-static addons-static utf8proc edit ncurses)
endif()

add_executable(carrier-shell ${SHELL_SOURCES})
target_link_libraries(carrier-shell LINK_PUBLIC ${LIBS} ${SYSTEM_LIBS})
add_dependencies(carrier-shell ${SHELL_DEPENDS})

if(APPLE AND (${CMAKE_BUILD_TYPE} STREQUAL "Debug"))
    add_custom_command(TARGET carrier-shell POST_BUILD
        COMMAND /usr/libexec/PlistBuddy -c "Add :com.apple.security.get-task-allow bool true" coredump.entitlements
        COMMAND codesign -s - -f --entitlements coredump.entitlements $<TARGET_FILE:carrier-shell>
        COMMAND rm -f coredump.entitlements
        VERBATIM)
endif()

install(TARGETS carrier-shell
    RUNTIME DESTINATION "bin"
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")

install(FILES shell.conf
    DESTINATION "etc/carrier")
