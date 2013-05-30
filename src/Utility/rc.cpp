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

    corrade-rc name group_name infile [-a alias] [infile2 [-a alias2] ... ] > outfile.cpp

Produces compiled C++ file with data in hexadecimal representation. File
is printed to `stdout`, status and error messages are printed to `stderr`.

See @ref resource-management for brief introduction.

@todo Test it
@todo Check empty files
*/

#include <fstream>

#include "Utility/Debug.h"
#include "Utility/Resource.h"

using Corrade::Utility::Debug;
using Corrade::Utility::Error;

#ifndef DOXYGEN_GENERATING_OUTPUT
int main(int argc, char** argv) {
    if(argc < 5) {
        Debug() << "Resource compiler for Corrade.";
        Debug() << "";
        Debug() << "Usage:";
        Debug() << "   " << argv[0] << "name group_name outfile infile [-a alias] [infile2 [-a alias2] ...] > outfile.cpp";
        Debug() << "";
        return 2;
    }

    std::vector<std::pair<std::string, std::string>> files;

    bool isAlias = false;
    std::string filename, alias;
    for(int i = 4; i != argc; ++i) {
        /* If argument is -a, next argument will be an alias */
        if(std::string(argv[i]) == "-a") {
            /* Check for "infile -a -a alias" */
            if(isAlias) {
                Error() << "Error: two subsequent aliases";
                return 4;
            }
            isAlias = true;
            continue;
        }

        /* Previous argument was -a */
        if(isAlias) {
            /* Check for "infile -a alias1 -a alias2" */
            if(!alias.empty()) {
                Error() << "Error: two subsequent aliases";
                return 3;
            }

            alias = argv[i];
            isAlias = false;

        } else {
            /* Check for "infile -a alias" */
            if(filename.empty() && !alias.empty()) {
                Error() << "Error: alias specified without filename";
                return 5;
            }

            /* Save previously gathered file name and alias */
            if(!filename.empty()) {
                files.push_back(make_pair(filename, alias));
                alias.clear();
            }

            filename = argv[i];
        }
    }

    /* Check for "infile -a" */
    if(isAlias) {
        Error() << "Error: no alias specified";
        return 5;
    }

    /* Check for no files present */
    if(filename.empty()) {
        Error() << "Error: no input file specified";
        return 6;
    }

    /* Save last gathered filename and alias */
    files.push_back(make_pair(filename, alias));

    /* Read all files */
    std::vector<std::pair<std::string, std::string>> data;
    for(auto it = files.cbegin(); it != files.cend(); ++it) {
        Debug() << "Reading file" << it-files.begin()+1 << "of" << files.size();
        Debug() << "   " << it->first;
        if(!it->second.empty())
            Debug() << " ->" << it->second;

        std::ifstream f(it->first.c_str(), std::ifstream::binary);

        if(f.bad()) {
            Error() << "Cannot open file " << it->first;
            return 1;
        }

        f.seekg(0, std::ios::end);
        std::string d(f.tellg(), '\0');
        f.seekg(0, std::ios::beg);
        f.read(&d[0], d.size());

        /* Compile file under real filename or alias, if set */
        data.emplace_back(it->second.empty() ? std::move(it->first) : std::move(it->second), std::move(d));
    }

    Debug() << "Compiling...";
    const std::string compiled = Corrade::Utility::Resource::compile(argv[1], argv[2], data);
    Debug() << "Done.";

    std::ofstream out(argv[3], std::ofstream::binary);
    if(!out.good()) {
        Error() << "Cannot open output file " << argv[3];
        return 7;
    }

    out.write(compiled.data(), compiled.size());
    return 0;
}
#endif
