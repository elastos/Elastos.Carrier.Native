cmake_minimum_required(VERSION 3.10)
cmake_policy(VERSION 3.10)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0114 NEW)
    cmake_policy(SET CMP0135 NEW)
endif()

project(ElastosCarrier VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(ProjectDefaults)
include(DistPackage)
include(AddSubModule)

if(CMAKE_CROSSCOMPILING)
    set(ENABLE_ADDONS_DEFAULT FALSE)
    set(ENABLE_TESTS_DEFAULT FALSE)
    set(ENABLE_SHELL_DEFAULT FALSE)
else()
    set(ENABLE_ADDONS_DEFAULT TRUE)
    set(ENABLE_TESTS_DEFAULT TRUE)
    set(ENABLE_SHELL_DEFAULT TRUE)
endif()

set(ENABLE_SHARED FALSE CACHE BOOL "Build shared library")
set(ENABLE_STATIC TRUE CACHE BOOL "Build static library")
set(ENABLE_ADDONS ${ENABLE_ADDONS_DEFAULT} CACHE BOOL "Build addons")
set(ENABLE_SHELL ${ENABLE_SHELL_DEFAULT} CACHE BOOL "Build shell application")
set(ENABLE_TESTS ${ENABLE_TESTS_DEFAULT} CACHE BOOL "Build test cases")
set(ENABLE_DOCS FALSE CACHE BOOL "Build APIs documentation")
set(ENABLE_CARRIER_DEVELOPMENT FALSE CACHE BOOL "Eanble carrier development mode")

add_subdirectory(deps)
add_subdirectory(src)

if(ENABLE_ADDONS)
    add_subdirectory(addons)
endif()

add_subdirectory(launcher)

if(ENABLE_TESTS)
    add_subdirectory(tests)
endif()

if (ENABLE_SHELL)
    add_subdirectory(shell)
endif()

if(ENABLE_DOCS)
   add_subdirectory(docs)
endif()

add_custom_target(dist
    COMMAND ${CMAKE_MAKE_PROGRAM} package
    COMMENT "Build distrubtion package")
