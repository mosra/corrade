
# (the blank line is here so CMake doesn't generate documentation from it)

#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
#             Vladimír Vondruš <mosra@centrum.cz>
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
