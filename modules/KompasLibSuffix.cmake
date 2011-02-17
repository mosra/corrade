include(KompasMacros)

if(NOT LIB_SUFFIX)
    message(STATUS "LIB_SUFFIX variable is not defined. It will be autodetected now.")
    message(STATUS "You can set it manually with -DLIB_SUFFIX=<value> (64 for example)")

    # All 32bit system have empty lib suffix
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # If there is /usr/lib64 and is not /usr/lib32, set suffix to 64
        if(IS_DIRECTORY /usr/lib64)
            message(STATUS "Found /usr/lib64, LIB_SUFFIX automatically set to 64")
            set_parent_scope(LIB_SUFFIX 64)
        elseif(IS_DIRECTORY /usr/lib)
            set_parent_scope(LIB_SUFFIX "")
        else()
            message(WARNING "LIB_SUFFIX cannot be autodetected. Libraries will be installed into ${CMAKE_INSTALL_PREFIX}/lib")
        endif()
    else()
        set_parent_scope(LIB_SUFFIX "")
    endif()
endif()
