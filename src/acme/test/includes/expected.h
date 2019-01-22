// Some random comment

#include <cstdint>
#include <cstring>

// And after

enum EnumInRelativePath {};
/* This has to be added *after* Relative.h for correct definition order */
int function(EnumInRelativePath);
typedef int Integer;
enum: std::int32_t { SomeValue = 42 };

void stuffAtTheEnd(Integer = SomeValue);

#ifdef INCLUDE_ALSO_HEAVY_STUFF
#include <functional>
#include <iostream>
#include <memory>
#include <regex>

std::unique_ptr<std::ostream> ugh(std::istream& in, std::function<std::int8_t(std::regex)> foo);
#endif
