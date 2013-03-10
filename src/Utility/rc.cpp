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
@todo Write to file (and don't create any on error)
@todo Use Debug classes
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "Resource.h"

#ifndef DOXYGEN_GENERATING_OUTPUT
int main(int argc, char** argv) {
    if(argc < 4) {
        std::cerr << "Resource compiler for Corrade.\n\nUsage:\n"
             << "    " << argv[0] << " name group_name infile [-a alias] [infile2 [-a alias2] ...] > outfile.cpp"
             << std::endl << std::endl;
        return 2;
    }

    Corrade::Utility::Resource r(argv[2]);

    std::vector<std::pair<std::string, std::string>> files;

    bool isAlias = false;
    std::string filename, alias;
    for(int i = 3; i != argc; ++i) {
        /* If argument is -a, next argument will be an alias */
        if(std::string(argv[i]) == "-a") {
            /* Check for "infile -a -a alias" */
            if(isAlias) {
                std::cerr << "Error: two subsequent aliases!" << std::endl;
                return 4;
            }
            isAlias = true;
            continue;
        }

        /* Previous argument was -a */
        if(isAlias) {
            /* Check for "infile -a alias1 -a alias2" */
            if(!alias.empty()) {
                std::cerr << "Error: two subsequent aliases!" << std::endl;
                return 3;
            }

            alias = argv[i];
            isAlias = false;

        } else {
            /* Check for "infile -a alias" */
            if(filename.empty() && !alias.empty()) {
                std::cerr << "Error: specified alias without filename!" << std::endl;
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
        std::cerr << "Error: no alias specified!" << std::endl;
        return 5;
    }

    /* Check for no files present */
    if(filename.empty()) {
        std::cerr << "Error: no input file specified!" << std::endl;
        return 6;
    }

    /* Save last gathered filename and alias */
    files.push_back(make_pair(filename, alias));

    /* Read all files */
    std::map<std::string, std::string> data;
    for(auto it = files.cbegin(); it != files.cend(); ++it) {
        std::cerr << "Reading file " << it-files.begin()+1 << "/" << files.size() << std::endl
                  << "    " << it->first << std::endl;
        if(!it->second.empty())
            std::cerr << " -> " << it->second << std::endl;

        std::ifstream f(it->first.c_str(), std::ifstream::in|std::ifstream::binary);

        if(!f.is_open()) {
            std::cerr << "Cannot open file " << it->first << std::endl;
            return 1;
        }

        std::ostringstream d;
        d << f.rdbuf();

        /* Compile file under real filename or alias, if set */
        if(it->second.empty())
            data.insert(std::make_pair(it->first, d.str()));
        else
            data.insert(std::make_pair(it->second, d.str()));
    }

    std::cerr << "Compiling..." << std::endl;
    std::cout << r.compile(argv[1], data);
    std::cerr << "Done." << std::endl;

    return 0;
}
#endif
