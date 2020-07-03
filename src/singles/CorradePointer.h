/*
    Corrade::Containers::Pointer
        — a lightweight alternative to std::unique_ptr

    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1Pointer.html

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
    v2019.01-107-g80d9f347 (2019-03-23)
    -   Including <cassert> only when needed
    v2018.10-232-ge927d7f3 (2019-01-28)
    -   Stricter matching for external representation conversion
    -   Ability to convert from external representation also using pointer()
    v2018.10-183-g4eb1adc0 (2019-01-23)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"
// {{includes}}
#if !defined(CORRADE_ASSERT) && !defined(NDEBUG)
#include <cassert>
#endif

/* We don't need anything from configure.h here */
#pragma ACME enable Corrade_configure_h

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

#include "Corrade/Containers/Pointer.h"
#ifdef CORRADE_POINTER_STL_COMPATIBILITY
// {{includes}}
#include "Corrade/Containers/PointerStl.h"
#endif
