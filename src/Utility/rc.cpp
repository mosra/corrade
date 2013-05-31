/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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

/** @file
@brief Utility for compiling data resources via command-line.

Usage:

    corrade-rc name resources.conf outfile.cpp

Produces compiled C++ file with data in hexadecimal representation. Status
messages are printed to standard output, errors are printed to error output.

See @ref resource-management for brief introduction.
*/

#include <fstream>

#include "Utility/Debug.h"
#include "Utility/Directory.h"
#include "Utility/Resource.h"

using Corrade::Utility::Debug;
using Corrade::Utility::Error;

#ifndef DOXYGEN_GENERATING_OUTPUT
int main(int argc, char** argv) {
    if(argc != 4) {
        Debug() << "Resource compiler for Corrade.";
        Debug() << "";
        Debug() << "Usage:";
        Debug() << "   " << argv[0] << "name resources.conf outfile.cpp";
        Debug() << "";
        if(argc == 0) return 0;
        return 1;
    }

    /* Remove previous output file */
    Corrade::Utility::Directory::rm(argv[3]);

    /* Compile file */
    const std::string compiled = Corrade::Utility::Resource::compileFrom(argv[1], argv[2]);

    /* Compilation failed */
    if(compiled.empty()) return 2;

    /* Save output */
    std::ofstream out(argv[3], std::ofstream::binary);
    if(!out.good()) {
        Error() << "Cannot open output file " << '\'' + std::string(argv[3]) + '\'';
        return 3;
    }
    out.write(compiled.data(), compiled.size());

    return 0;
}
#endif
