#ifndef Local_h
#define Local_h

#include <cstdint>

#include "Dir/Bla.h" /* included twice, each time with different quotes */

#if 0
#include <foobar> // this should not get included
#endif

#if 1
#include <cmath> // this should, but globally
#include <cstring> // this should not, as it is already above
#endif

#ifdef FOOBAR
#include <foobar> // this should get included locally
#endif

#ifdef FOOBAR
#include <foobar> // this should not be repeated, it's above already
#endif

enum: std::int32_t { SomeValue = 42 };

#endif
