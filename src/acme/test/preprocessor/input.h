// {{includes}}

#include <type_traits>

#if 1
// this gets included
int this_too = 3;
#else
// this not
#include <this_neither>
/*
    and neither this
*/
struct NorThis {};
#endif // this not as well

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
#if 0 // this doesn't get included
nope();
#if 1
neither_this();
#endif
#if 0
#else
// and this not too
#endif
#else // this does
Yay yay;
#endif

/* Corner cases for #elif */
#ifndef BADTOOTH
Yep wow(Yay hah);
#elif !defined(FOOBAR)
wow(Yay{});
#elif 1
auto this_gets_enabled() -> Yay { return {}; }
#elif defined(ABC)
// this doesn't go anywhere
#else
NEITHER THIS! INTERCAL FTW
#endif

    #if 0 /* testing indented code */
    // this gets hidden
    #elif defined(FOOBAR)
    #define THIS_IS_VISIBLE YAY
    #elif 1
    #define THIS_TOO(but, unconditionally)
    #endif

/* Nested if/elif should not flip the outer and leak visibility, so nothing
   gets out */
#if 0
// this gets hidden
#ifdef SOMETHING
// this as well
#elif defined(OTHERTHING)
// this too
#elif 1
// neither this is shown
#elif 0
// nope
#else
// and also not
#endif
#endif

/* Output only the third branch */
#if 0
// this gets hidden
#elif 1
// this goes to the output
#else
// this not again
#endif

/* Appends one more endif */
#if defined(ACME)
// this gets to the output
#elif 0
// this not
#elif defined(FOOBAR)
// this again
#endif

/* No extraneous endifs */
#if 0
// no
#elif 0
// no
#elif 0
#if 1
// no
#endif
#elif 1
// yes
#endif

/* Remove both these, since they're empty */
#ifdef B
#endif

#ifdef A
#ifndef B
#endif
#endif

/* Don't remove anything here */
#ifdef A
#elif defined(B)
#endif

/* Don't remove anything here either */
#ifdef A
#elif 1
#endif

/* Parse this correctly (the slash is there) */
#if __has_include(<bits/c++config.h>) /* the __GLIBCXX__ define */
// yay!
#endif
