#include <cstdint> /* Already there */
#include <functional>
#include <iostream>
#include <memory>
#include <regex>

#include "Local.h" /* Already there */

std::unique_ptr<std::ostream> ugh(std::istream& in, std::function<std::int8_t(std::regex)> foo);
