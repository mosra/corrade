#pragma ACME disable DOXYGEN_GENERATING_OUTPUT
#pragma ACME enable CORRADE_NO_DEBUG

#pragma ACME comments off

/* this comment gets removed */

/** @brief this one as well */

// yep

/*
    and this also
*/

int a;
/** @brief A comment between two lines should get removed too */
float b;

void foobar(); /**< trailing comments nuked as well */

#pragma ACME comments on

#ifdef DOXYGEN_GENERATING_OUTPUT
/**
Some very verbose docs that get hidden

blah.
*/
AndCrazy apiNames();
#elif defined(CORRADE_NO_DEBUG) || defined(NDEBUG)
// Only this gets to the output, in fact.
/** @brief Docs */
DebugStuff apiNames(int a = "disambiguate"[0]);
#else
auto apiNames(int a = 0) -> SomethingNormal; /* hidden as well */
#endif

#pragma ACME unknown

#pragma ACME disable input_h

#ifndef input_h
#define input_h
/* Forced defines also make it possible to get rid of include guards, so this
   comment is not wrapped in any */
#endif

/* Here <string> is included twice */
// {{includes}}
#include <string>
// {{includes}}
#pragma ACME forget <string>
#include <string>
/* Putting it again into the same {{includes}} doesn't make sense and so the
   below does nothing (with a warning) */
#pragma ACME forget <string>
#include <string>

/* This should define INCLUDED_ALWAYS, INCLUDED_FIRST with no guard,
   INCLUDED_ALWAYS again and then INCLUDED_SECOND with a guard (i.e., it's now
   neither always enabled nor always diabled). The included file is also on a
   custom path that should be automatically associated with it. */
#pragma ACME path some/
#pragma ACME disable INCLUDED_FIRST
#pragma ACME enable INCLUDED_SECOND
#include "path/included.h"
#pragma ACME forget INCLUDED_SECOND
#pragma ACME enable INCLUDED_FIRST
#pragma ACME forget "path/included.h"
#include "path/included.h"
