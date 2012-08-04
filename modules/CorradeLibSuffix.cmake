if(NOT DEFINED LIB_SUFFIX)
    message(STATUS "LIB_SUFFIX variable is not defined. It will be autodetected now.")
    message(STATUS "You can set it manually with -DLIB_SUFFIX=<value> (64 for example)")

    # All 32bit system have empty lib suffix
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # If there is /usr/lib64 and is not /usr/lib32, set suffix to 64
        # Ubuntu 64bit symlinks /usr/lib64 to /usr/lib, install to /usr/lib there
        if(IS_DIRECTORY /usr/lib64 AND NOT IS_SYMLINK /usr/lib64)
            set(LIB_SUFFIX 64)
        elseif(IS_DIRECTORY /usr/lib)
            set(LIB_SUFFIX "")
        else()
            message(WARNING "LIB_SUFFIX cannot be autodetected. No /usr/lib neither /usr/lib64 found.")
            set(LIB_SUFFIX "")
        endif()
    else()
        set(LIB_SUFFIX "")
    endif()

    # Put the value into cache
    set(LIB_SUFFIX "${LIB_SUFFIX}" CACHE STRING "Library directory suffix (e.g. 64 for /usr/lib64).")

    message(STATUS "LIB_SUFFIX autodetected as '${LIB_SUFFIX}', libraries will be installed into ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")
endif()
