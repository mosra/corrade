/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include <string>

#include "Corrade/Utility/Arguments.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/Resource.h"

namespace Corrade {

/** @page corrade-rc Resource compiler
@brief Utility for compiling data resources via command-line.

Produces compiled C++ file with data in hexadecimal representation to be used
with @ref Utility::Resource. See @ref resource-management for brief
introduction.

This utility is built if `WITH_RC` is enabled when building Corrade. To use
this utility with CMake, see the
@ref corrade-cmake-add-resource "corrade_add_resource()" macro. To use it
directly, you need to request the `rc` component of the `Corrade` package and
use the `Corrade::rc` target for example in a custom command:

@code{.cmake}
find_package(Corrade REQUIRED rc)

add_custom_command(OUTPUT ... COMMAND Corrade::rc ...)
@endcode

See @ref building-corrade, @ref corrade-cmake and the @ref Utility namespace
for more information.

@section corrade-rc-usage Usage

@code{.sh}
corrade-rc [-h|--help] [--] name resources.conf outfile.cpp
@endcode

Arguments:

-   `resources.conf` --- resource configuration file (see @ref Utility::Resource
    for format description)
-   `outfile.cpp` --- output file
-   `-h`, `--help` --- display this help message and exit
*/

}

#ifndef DOXYGEN_GENERATING_OUTPUT /* LCOV_EXCL_START */
int main(int argc, char** argv) {
    Corrade::Utility::Arguments args;
    args.addArgument("name")
        .addArgument("conf").setHelp("conf", "resource configuration file", "resources.conf")
        .addArgument("out").setHelp("out", "output file", "outfile.cpp")
        .setCommand("corrade-rc")
        .setGlobalHelp("Resource compiler for Corrade.")
        .parse(argc, argv);

    /* Remove previous output file */
    Corrade::Utility::Directory::rm(args.value("out"));

    /* Compile file */
    const std::string compiled = Corrade::Utility::Resource::compileFrom(args.value("name"), args.value("conf"));

    /* Compilation failed */
    if(compiled.empty()) return 2;

    /* Save output */
    if(!Corrade::Utility::Directory::writeString(args.value("out"), compiled)) {
        Corrade::Utility::Error() << "Cannot write output file " << '\'' + args.value("out") + '\'';
        return 3;
    }

    return 0;
}
#endif /* LCOV_EXCL_STOP */
