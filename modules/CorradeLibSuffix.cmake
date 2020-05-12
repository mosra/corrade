
# (the blank line is here so CMake doesn't generate documentation from it)

#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>
#
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.
#

if(NOT DEFINED LIB_SUFFIX)
    message(STATUS "LIB_SUFFIX variable is not defined. It will be autodetected now.")
    message(STATUS "You can set it manually with -DLIB_SUFFIX=<value> (64 for example)")

    # All 32bit systems and OSX have empty lib suffix, decide based on
    # FIND_LIBRARY_USE_LIB64_PATHS on other 64bit systems
    if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT APPLE)
        # CMake might be right most of the time, but if /usr/lib64 is symlink
        # to somewhere else, it means that we should *really* not install there
        # (that's the case with ArchLinux)
        get_property(LIB_SUFFIX GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)
        if(LIB_SUFFIX AND NOT IS_SYMLINK /usr/lib64)
            set(LIB_SUFFIX "64")
        else()
            set(LIB_SUFFIX "")
        endif()
    else()
        set(LIB_SUFFIX "")
    endif()

    # Put the value into cache
    set(LIB_SUFFIX "${LIB_SUFFIX}" CACHE STRING "Library directory suffix (e.g. 64 for /usr/lib64).")

    message(STATUS "LIB_SUFFIX autodetected as '${LIB_SUFFIX}', libraries will be installed into ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")
endif()
