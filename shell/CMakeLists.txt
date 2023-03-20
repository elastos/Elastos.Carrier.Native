include(ProjectDefaults)

include_directories(
    .
    ../include
    ../src
    ${CARRIER_INT_DIST_DIR}/include)

list(APPEND SHELL_SOURCES
    main.cc
)

if(ENABLE_SHARED)
    set(CARRIER_LIB carrier-shared)
else()
    set(CARRIER_LIB carrier-static)
endif()

add_executable(carriershell ${SHELL_SOURCES})
target_link_libraries(carriershell LINK_PUBLIC ${CARRIER_LIB} utf8proc)

install(TARGETS carriershell
    RUNTIME DESTINATION "bin"
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")
