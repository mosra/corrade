include(KompasMacros)

if(NOT DEFINED LIB_SUFFIX AND NOT __LIB_SUFFIX_SET)
    message(STATUS "LIB_SUFFIX variable is not defined. It will be autodetected now.")
    message(STATUS "You can set it manually with -DLIB_SUFFIX=<value> (64 for example)")

    # All 32bit system have empty lib suffix
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # If there is /usr/lib64 and is not /usr/lib32, set suffix to 64
        if(IS_DIRECTORY /usr/lib64)
            set_parent_scope(LIB_SUFFIX 64)
        elseif(IS_DIRECTORY /usr/lib)
            set_parent_scope(LIB_SUFFIX "")
        else()
            message(WARNING "LIB_SUFFIX cannot be autodetected. No /usr/lib neither /usr/lib64 found.")
            set_parent_scope(LIB_SUFFIX "")
        endif()
    else()
        set_parent_scope(LIB_SUFFIX "")
    endif()

    # Workaround for another CMake bug: When LIB_SUFFIX is empty,
    # set(LIB_SUFFIX "" PARENT_SCOPE) doesn't set anything.
    set_parent_scope(__LIB_SUFFIX_SET True)

    message(STATUS "LIB_SUFFIX autodetected as '${LIB_SUFFIX}', libraries will be installed into ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")
endif()
