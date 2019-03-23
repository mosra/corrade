// Some random comment

/* An {{includes}} that has nothing to add will get ignored */

#include <cmath>
#include <cstdint>
#include <cstring>

#include "noexpand.h"

// And after

enum EnumInRelativePath {};
/* This has to be added *after* Relative.h for correct definition order */
int function(EnumInRelativePath);
typedef int Integer;
#ifndef Local_h
#define Local_h

#ifdef FOOBAR
#include <foobar>
#endif

enum: std::int32_t { SomeValue = 42 };

#endif

void stuffAtTheEnd(Integer = SomeValue);

/* this ifdef stays here even though the file doesn't have an include guard */
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef INCLUDE_ALSO_HEAVY_STUFF
#include <functional>
#include <iostream>
#include <memory>
#include <regex>

std::unique_ptr<std::ostream> ugh(std::istream& in, std::function<std::int8_t(std::regex)> foo);
#endif
