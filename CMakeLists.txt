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
    set(ENABLE_APPS_DEFAULT FALSE)
    set(ENABLE_TESTS_DEFAULT FALSE)
    set(ENABLE_LAUNCHER_DEFAULT FALSE)
else()
    set(ENABLE_APPS_DEFAULT TRUE)
    set(ENABLE_TESTS_DEFAULT TRUE)
    set(ENABLE_LAUNCHER_DEFAULT TRUE)
endif()

set(ENABLE_SHARED FALSE CACHE BOOL "Build shared library")
set(ENABLE_STATIC TRUE CACHE BOOL "Build static library")
set(ENABLE_ADDONS TRUE CACHE BOOL "Build addons")
set(ENABLE_TESTS ${ENABLE_TESTS_DEFAULT} CACHE BOOL "Build test cases")
set(ENABLE_APPS ${ENABLE_APPS_DEFAULT} CACHE BOOL "Build applications")
set(ENABLE_DOCS FALSE CACHE BOOL "Build APIs documentation")
set(ENABLE_CARRIER_DEVELOPMENT FALSE CACHE BOOL "Eanble carrier development mode")

add_subdirectory(deps)
add_subdirectory(src)

if(ENABLE_ADDONS)
    add_subdirectory(addons)
endif()

if(ENABLE_TESTS)
    add_subdirectory(tests/apitests)
endif()

if (ENABLE_APPS)
    add_subdirectory(apps/shell)
    add_subdirectory(apps/launcher)
endif()

if(ENABLE_DOCS)
   add_subdirectory(docs)
endif()

add_custom_target(dist
    COMMAND ${CMAKE_MAKE_PROGRAM} package
    COMMENT "Build distrubtion package")
