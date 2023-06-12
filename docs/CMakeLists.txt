project(docs)

include(ProjectDefaults)

set(DOXYGEN "doxygen")
set(SPHINXBUILD "sphinx-build")
set(DOXYGEN_CONF "carrier.cfg")

if(APPLE)
    message(FATAL_ERROR
        "Can not build APIs docs on MacOSX")
endif()

find_program(DOXYGEN_PROG NAMES ${DOXYGEN})
if(${DOXYGEN_PROG} STREQUAL "DOXYGEN_PROG-NOTFOUND")
    message(FATAL_ERROR
        "Doxygen is needed to build the docs")
endif()

find_program(SPHINXBUILD_PROG NAMES ${SPHINXBUILD})
if(${SPHINXBUILD_PROG} STREQUAL "SPHINXBUILD_PROG-NOTFOUND")
    message(FATAL_ERROR
        "sphinx-build is needed to build the docs")
endif()

set(CARRIERPUBLIC_INT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(CARRIERPUBLIC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include/carrier")
set(CARRIERPUBLIC_DOC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(CARRIERPUBLIC_SRC_HEADERS
    "${CARRIERPUBLIC_INCLUDE_DIR}/addon.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/blob.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/configuration.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/crypto_core.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/default_configuration.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/id.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/log.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/lookup_option.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/node_info.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/node_status_listener.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/node_status.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/node.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/peer_info.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/prefix.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/socket_address.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/types.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/value.h"
    "${CARRIERPUBLIC_INCLUDE_DIR}/version.h")

file(GLOB_RECURSE CARRIERPUBLIC_DOC_SOURCES *)

string(REPLACE ";" " " CARRIERPUBLIC_SRC_INPUT "${CARRIERPUBLIC_SRC_HEADERS}")

configure_file(
    source/carrier.cfg.in
    carrier.cfg
    "@ONLY")

configure_file(
    source/conf.py.in
    conf.py
    "@ONLY")

add_custom_command(
    OUTPUT html
    DEPENDS ${CARRIERPUBLIC_SRC_HEADERS} ${CARRIERPUBLIC_DOC_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${DOXYGEN_PROG} ${DOXYGEN_CONF}
    COMMAND ${SPHINXBUILD_PROG}
        -c .
        -b html
        ${CMAKE_CURRENT_SOURCE_DIR}/source
        html)

add_custom_target(docs
    ALL
    COMMENT "Build html APIs documentation"
    DEPENDS html)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
    DESTINATION share/elastos/carrier)
