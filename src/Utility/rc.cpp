/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Map2X.

    Map2X is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Map2X is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Utility for compiling data resources via command-line.
 *
 * Usage:
 * <tt>map2x-dl name group_name infile [-a alias] [infile2 [-a alias2] ... ] >
 * outfile.cpp</tt>
 *
 * Produces compiled C++ file with data in hexadecimal representation. File
 * is printed to @c stdout, status and error messages are printed to @c stderr.
 *
 * See also @ref Map2X::Utility::Resource.
 * @todo Test it
 * @todo Check empty files
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "Resource.h"

using namespace std;

#ifndef DOXYGEN_GENERATING_OUTPUT
int main(int argc, char** argv) {
    if(argc < 4) {
        cerr << "Resource compiler for Map2X.\n\nUsage:\n"
             << "    " << argv[0] << " name group_name infile [-a alias] [infile2 [-a alias2] ...] > outfile.cpp"
             << endl << endl;
        return 2;
    }

    Map2X::Utility::Resource r(argv[2]);

    vector<pair<string, string> > files;

    bool isAlias = false;
    string filename, alias;
    for(int i = 3; i != argc; ++i) {
        /* If argument is -a, next argument will be an alias */
        if(string(argv[i]) == "-a") {
            /* Check for "infile -a -a alias" */
            if(isAlias) {
                cerr << "Error: two subsequent aliases!" << endl;
                return 4;
            }
            isAlias = true;
            continue;
        }

        /* Previous argument was -a */
        if(isAlias) {
            /* Check for "infile -a alias1 -a alias2" */
            if(!alias.empty()) {
                cerr << "Error: two subsequent aliases!" << endl;
                return 3;
            }

            alias = argv[i];
            isAlias = false;

        } else {
            /* Save previously gathered file name and alias */
            if(!filename.empty()) {
                files.push_back(pair<string, string>(filename, alias));
                alias.clear();
            }

            filename = argv[i];
        }
    }

    /* Check for "infile -a" */
    if(isAlias) {
        cerr << "Error: no alias specified!" << endl;
        return 5;
    }

    /* Check for no files present */
    if(filename.empty()) {
        cerr << "Error: no input file specified!" << endl;
        return 6;
    }

    /* Save last gathered filename and alias */
    files.push_back(pair<string, string>(filename, alias));

    /* Read all files */
    map<string, string> data;
    for(vector<pair<string, string> >::const_iterator it = files.begin(); it != files.end(); ++it) {
        cerr << "Reading file " << it-files.begin()+1 << "/" << files.size() << endl
             << "    " << it->first << endl;
        if(!it->second.empty())
            cerr << " -> " << it->second << endl;

        ifstream f(it->first.c_str(), ifstream::in|ifstream::binary);

        if(!f.is_open()) {
            cerr << "Cannot open file " << it->first << endl;
            return 1;
        }

        ostringstream d;
        d << f.rdbuf();

        /* Compile file under real filename or alias, if set */
        if(it->second.empty())
            data.insert(pair<string, string>(it->first, d.str()));
        else
            data.insert(pair<string, string>(it->second, d.str()));
    }

    cerr << "Compiling..." << endl;
    cout << r.compile(argv[1], data);
    cerr << "Done." << endl;

    return 0;
}
#endif
