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
