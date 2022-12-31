/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
              Vladimír Vondruš <mosra@centrum.cz>

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

#include "Corrade/Utility/Arguments.h"
#include "Corrade/Utility/DebugStl.h" /** @todo drop once Arguments is <string>-free */
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/Implementation/ResourceCompile.h"

namespace Corrade {

/** @page corrade-rc Resource compiler
@brief Resource compiling utility for @ref Utility::Resource.

@m_keywords{corrade-rc}

Produces a C++ file with data in a hexadecimal representation to be compiled
into an executable and used with @ref Utility::Resource. See also
@ref resource-management for a tutorial.

This utility is built if `CORRADE_WITH_RC` is enabled when building Corrade. To
use this utility with CMake, see the
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
corrade-rc [-h|--help] [--single] [--] name input output.cpp
@endcode

By default expects that `input` is a resource configuration file containing a
@cb{.ini} group @ce name and zero or more @cb{.ini} [file] @ce groups with
input filenames. If `--single` is specified, the `input` file is read and
directly compiled into a C++ source file, exposing the data under
@cpp extern const unsigned char resourceData_name[] @ce and
@cpp extern const unsigned int resourceSize_name @ce symbols, with no
dependency on @ref Utility::Resource or any other header.

Arguments:

-   `name` --- exported symbol name
-   `input` --- resource configuration file (see @ref Utility::Resource for
    format description) or a single file to process
-   `output.cpp` --- output file
-   `-h`, `--help` --- display this help message and exit
-   `--single` --- compile a single file instead of parsing a configuration
    file
*/

}

using namespace Corrade;

#ifndef DOXYGEN_GENERATING_OUTPUT /* LCOV_EXCL_START */
int main(int argc, char** argv) {
    Utility::Arguments args;
    args.addArgument("name").setHelp("name", "exported symbol name")
        .addArgument("input").setHelp("input", "resource configuration file or a single file to process", "input")
        .addArgument("output").setHelp("output", "output file", "output.cpp")
        .addBooleanOption("single").setHelp("single", "compile a single file instead of parsing a configuration file")
        .setCommand("corrade-rc")
        .setGlobalHelp(R"(Corrade resource compiler.

By default expects that input is a resource configuration file containing a
group name and zero or more [file] groups with input filenames. If --single
is specified, the input file is read and directly compiled into a C++ source
file, exposing the data under `extern const unsigned char resourceData_<name>[]`
and `extern const std::size_t resourceSize_<name>` symbols, with no dependency
on Corrade's resource system or any other header.)")
        .parse(argc, argv);

    /* Remove previous output file. Only if it exists, to not print an error
       message when compiling for the first time. If it fails, die as well --
       we'd not succeed after either. */
    if(Utility::Path::exists(args.value("output")) &&
      !Utility::Path::remove(args.value("output")))
        return 1;

    /* Compile file */
    const Containers::String compiled = args.isSet("single") ?
        Utility::Implementation::resourceCompileSingle(args.value("name"), args.value("input")) :
        Utility::Implementation::resourceCompileFrom(args.value("name"), args.value("input"));

    /* Compilation failed */
    if(!compiled) return 2;

    /* Save output */
    if(!Utility::Path::write(args.value("output"), compiled)) {
        Utility::Error{} << "Cannot write output file" << '\'' + args.value("output") + '\'';
        return 3;
    }

    return 0;
}
#endif /* LCOV_EXCL_STOP */
