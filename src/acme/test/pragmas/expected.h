int a;
float b;

void foobar();

// Only this gets to the output, in fact.
/** @brief Docs */
DebugStuff apiNames(int a = "disambiguate"[0]);

/* Forced defines also make it possible to get rid of include guards, so this
   comment is not wrapped in any */

/* Here <string> is included twice */
#include <string>
#include <string>
/* Putting it again into the same {{includes}} doesn't make sense and so the
   below does nothing (with a warning) */

/* This should define INCLUDED_ALWAYS, INCLUDED_FIRST with no guard,
   INCLUDED_ALWAYS again and then INCLUDED_SECOND with a guard (i.e., it's now
   neither always enabled nor always diabled). The included file is also on a
   custom path that should be automatically associated with it. */
#define INCLUDED_ALWAYS always
#define INCLUDED_FIRST first
#define INCLUDED_ALWAYS always
#ifndef INCLUDED_SECOND
#define INCLUDED_SECOND second
#endif
