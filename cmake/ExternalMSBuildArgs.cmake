if(__external_msbuild_args_included)
    return()
endif()
set(__external_msbuild_args_included TRUE)

include(ProjectDefaults)

##Only suport for windows.
if(WIN32)
function(set_win_build_options build_options suffix)
    # check vs platform.
    # Notice: use CMAKE_SIZEOF_VOID_P to check whether target is
    # 64bit or 32bit instead of using CMAKE_SYSTEM_PROCESSOR.
    if(${CMAKE_SIZEOF_VOID_P} STREQUAL "8")
        set(_PLATFORM "x64")
    else()
        set(_PLATFORM "Win32")
    endif()

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(_CONFIGURATION "Debug${suffix}")
    else()
        set(_CONFIGURATION "Release${suffix}")
    endif()

    string(CONCAT _BUILD_OPTIONS
        "/p:"
        "Configuration=${_CONFIGURATION},"
        "Platform=${_PLATFORM},"
        "InstallDir=${PROJECT_INT_DIST_DIR}")

    # update parent scope variable.
    set(${build_options} "${_BUILD_OPTIONS}" PARENT_SCOPE)
endfunction()
endif()