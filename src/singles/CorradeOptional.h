/*
    Corrade::Containers::Optional
        — a lightweight alternative to std::optional

    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1Optional.html

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    v2020.06-0-g61d1b58c (2020-06-27)
    -   Working around various compiler-specific issues and standard defects
        when using {}-initialization for aggregate types
    v2019.10-0-g162d6a7d (2019-10-24)
    -   Minor simplifications in the internals
    v2019.01-107-g80d9f347 (2019-03-23)
    -   Including <cassert> only when needed
    v2018.10-232-ge927d7f3 (2019-01-28)
    -   Ability to "take" a value out of a r-value optional using operator*
    -   Opt-in compatibility with <optional> from C++17
    v2018.10-183-g4eb1adc0 (2019-01-23)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"
// {{includes}}
#if !defined(CORRADE_ASSERT) && !defined(NDEBUG)
#include <cassert>
#endif

/* We don't need much from configure.h here */
#pragma ACME enable Corrade_configure_h
#ifdef __GNUC__
#define CORRADE_TARGET_GCC
#endif

/* Disable asserts that are not used. CORRADE_ASSERT is used, wrapping the
   #include <cassert> above. When enabling additional asserts, be sure to
   update it above as well. */
#pragma ACME enable CORRADE_CONSTEXPR_ASSERT
#pragma ACME enable CORRADE_ASSERT_OUTPUT
#pragma ACME enable CORRADE_ASSERT_UNREACHABLE
#pragma ACME enable CORRADE_INTERNAL_ASSERT
#pragma ACME enable CORRADE_INTERNAL_CONSTEXPR_ASSERT
#pragma ACME enable CORRADE_INTERNAL_ASSERT_OUTPUT
#pragma ACME enable CORRADE_INTERNAL_ASSERT_UNREACHABLE

#include "Corrade/Containers/Optional.h"
#ifdef CORRADE_OPTIONAL_STL_COMPATIBILITY
// {{includes}}
#include "Corrade/Containers/OptionalStl.h"
#endif
