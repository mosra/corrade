/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021
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

#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <cstdlib>

std::string wrap(
    const std::string& in,
    const std::string& before,
    const std::string& after)
{
  return before + in + after;
}

bool substitute(
    std::string& in,
    const std::string& what,
    const std::string& with) {
  bool success = false;
  size_t pos = 0;
  while ((pos = in.find(what, pos)) != std::string::npos) {
    success = true;
    in.replace(pos, what.length(), with);
    pos += with.length();
  }
  return success;
}

bool define(std::string& in, const std::string& what) {
  std::string cmakedefine = "#cmakedefine ";
  cmakedefine += what;

  std::string with = "#define ";
  with += what;

  return substitute(in, cmakedefine, with);
}

bool strip(std::string& in) {
    std::regex re(".*(\\$\\{.*\\}).*");
    std::smatch m;
    if (std::regex_match(in, m, re)) {
      return substitute(in, m[1].str(), "");
    }
    return false;
}

void undef(std::string& in) {
  if (substitute(in, "#cmakedefine", "#undef")) {
    in = wrap(in, "/* ", " */");
  }
}

int main(int argc, char* argv[]) {
  std::ifstream in{argv[1]};
  if (!in.is_open()) {
    std::cerr << "Failed to open" << std::endl;
    return EXIT_FAILURE;
  }
  std::ofstream out{argv[2]};

  std::map<std::string, std::string> substitutions;

  for (int i = 3; i < argc; ++i) {
    std::string s = argv[i];
    std::regex re("-D(.*)=(.*)");
    std::smatch m;
    if (std::regex_match(s, m, re)) {
      substitutions.emplace(m[1].str(), m[2].str());
    }
  }

  {
    std::string line;
    while (std::getline(in, line)) {
      for (const auto& p : substitutions) {
        substitute(line, wrap(p.first, "${", "}"), p.second);
        define(line, p.first);
      }
      strip(line);
      undef(line);
      out << line << "\n";
    }
  }
  out.close();

  return EXIT_SUCCESS;
}
