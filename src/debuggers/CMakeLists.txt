#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020, 2021, 2022
#             Vladimír Vondruš <mosra@centrum.cz>
#   Copyright © 2022 Guillaume Jacquemin <williamjcm@users.noreply.github.com>
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

# TODO: GDB also has an ability to auto-load script files from a system-wide
# directory, which would make it possible to load them without having to
# manually update `.gdbinit` files. Unfortunately, this requires GDB to be
# compiled with `--with-system-gdbinit-dir`, which neither ArchLinux nor Ubuntu
# does, both set only `--with-system-gdbinit` which would mean having to
# *append* to that file on install. Not the way to go.
#
# TODO: there's also an option to install those next to the libraries (such as
# having /usr/lib/CorradeContainers-gdb.py), or embedded in the binaries in the
# .debug_gdb_scripts section. Both still require some whitelisting from the
# user, though, so no automagic either:
#   https://sourceware.org/gdb/onlinedocs/gdb/Python-Auto_002dloading.html#Python-Auto_002dloading
#   https://sourceware.org/gdb/onlinedocs/gdb/dotdebug_005fgdb_005fscripts-section.html#dotdebug_005fgdb_005fscripts-section
if(NOT CORRADE_TARGET_MSVC AND NOT CORRADE_TARGET_EMSCRIPTEN)
    install(DIRECTORY gdb DESTINATION ${CORRADE_DATA_INSTALL_DIR}/debuggers)
endif()

# TODO embed those directly in the PDB:
#   https://reviews.llvm.org/D48703
#   https://gitlab.kitware.com/cmake/cmake/-/issues/16874
if(CORRADE_TARGET_MSVC)
    install(DIRECTORY natvis DESTINATION ${CORRADE_DATA_INSTALL_DIR}/debuggers)
endif()
