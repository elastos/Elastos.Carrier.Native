include(ProjectDefaults)
include(CheckSymbolExists)

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

check_symbol_exists(SIGKILL signal.h HAVE_SIGKILL)
if(HAVE_SIGKILL)
    add_definitions(-DHAVE_SIGKILL=1)
endif()

check_symbol_exists(SIGHUP signal.h HAVE_SIGHUP)
if(HAVE_SIGHUP)
    add_definitions(-DHAVE_SIGHUP=1)
endif()

check_symbol_exists(SIGTSTP signal.h HAVE_SIGTSTP)
if(HAVE_SIGTSTP)
    add_definitions(-DHAVE_SIGTSTP=1)
endif()

check_symbol_exists(SIGCHLD signal.h HAVE_SIGCHLD)
if(HAVE_SIGCHLD)
    add_definitions(-DHAVE_SIGCHLD=1)
endif()

check_symbol_exists(SIGTTOU signal.h HAVE_SIGTTOU)
if(HAVE_SIGTTOU)
    add_definitions(-DHAVE_SIGTTOU=1)
endif()

check_symbol_exists(SIGTTIN signal.h HAVE_SIGTTIN)
if(HAVE_SIGTTIN)
    add_definitions(-DHAVE_SIGTTIN=1)
endif()

include_directories(
    .
    ../common
    ../../include
    )

add_definitions(-DCARRIER_CRAWLER)

list(APPEND CRAWLER_SOURCES
    setting.cc
    crawler.cc
    main.cc)

list(APPEND CRAWLER_DEPENDS
    IP2Location8
    CLI11)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIBS rt pthread dl)
endif()


set(LIBS
    IP2Location)


if(ENABLE_SHARED)
set(LIBS carrier-shared ${LIBS})
else()
set(LIBS carrier-static ${LIBS})
endif()

add_executable(carrier-crawler ${CRAWLER_SOURCES})
target_link_libraries(carrier-crawler LINK_PUBLIC ${LIBS} ${SYSTEM_LIBS})
add_dependencies(carrier-crawler ${CRAWLER_DEPENDS})

if(APPLE AND (${CMAKE_BUILD_TYPE} STREQUAL "Debug"))
    add_custom_command(TARGET carrier-crawler POST_BUILD
        COMMAND /usr/libexec/PlistBuddy -c "Add :com.apple.security.get-task-allow bool true" coredump.entitlements
        COMMAND codesign -s - -f --entitlements coredump.entitlements $<TARGET_FILE:carrier-crawler>
        COMMAND rm -f coredump.entitlements
        VERBATIM)
endif()

install(TARGETS carrier-crawler
    RUNTIME DESTINATION "bin"
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")

install(FILES crawler.conf
    DESTINATION "etc/carrier-crawler")
