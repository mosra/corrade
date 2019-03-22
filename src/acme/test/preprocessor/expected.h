#include <type_traits>

// this gets included
int this_too = 3;

#ifdef FOOBAR // with a comment
// this gets as well
enum Yep {};
#else
/* this too */
using Yay = int;
#endif

#if defined(__GNUC__) && __GNUC__ > 5
// complex expression stays as-is
constexpr bool eh = std::is_trivially_constructible<int>::value;
#endif

/* Corner cases for #else */
Yay yay;

/* Corner cases for #elif */
#ifndef BADTOOTH
Yep wow(Yay hah);
#elif !defined(FOOBAR)
wow(Yay{});
#else
auto this_gets_enabled() -> Yay { return {}; }
#endif

    #ifdef FOOBAR
    #define THIS_IS_VISIBLE YAY
    #else
    #define THIS_TOO(but, unconditionally)
    #endif

/* Nested if/elif should not flip the outer and leak visibility, so nothing
   gets out */

/* Output only the third branch */
// this goes to the output

/* Appends one more endif */
#ifdef ACME
// this gets to the output
#else
#ifdef FOOBAR
// this again
#endif
#endif

/* No extraneous endifs */
// yes

/* Remove both these, since they're empty */

/* Don't remove anything here */
#ifdef A
#elif defined(B)
#endif

/* Don't remove anything here either */
#ifdef A
#else
#endif

/* Parse this correctly (the slash is there) */
#if __has_include(<bits/c++config.h>) /* the __GLIBCXX__ define */
// yay!
#endif
