project(carrier-shell CXX)
include(ProjectDefaults)

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
    add_definitions(-DHAVE_UNISTD_H=1)
endif()

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

check_include_file(sys/file.h HAVE_SYS_FILE_H)
if(HAVE_SYS_FILE_H)
    add_definitions(-DHAVE_SYS_FILE_H=1)
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

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIBS dl)
endif()

add_definitions(-DUTF8PROC_STATIC)

set(LIBS
    utf8proc
    edit)

if(WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_WARNINGS)

    set(LIBS
        ${LIBS}
        libsodium.lib
        Ws2_32
        Rpcrt4
        crypt32
        iphlpapi)
else()
    set(LIBS
        ${LIBS}
        ncurses
        sodium)
endif()

if(ENABLE_SHARED)
    set(LIBS carrier-shared addons-shared ${LIBS})
else()
    set(LIBS carrier-static addons-static ${LIBS})
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

install(FILES shell.conf DESTINATION "bin")
