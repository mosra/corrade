/*
    Corrade::Containers::Reference
        — a lightweight alternative to std::reference_wrapper

    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1Reference.html

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, changelogs and full docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"

/* We don't need anything from configure.h here */
#pragma ACME enable Corrade_configure_h

/* Disable asserts that are not used */
#pragma ACME enable CORRADE_CONSTEXPR_ASSERT
#pragma ACME enable CORRADE_ASSERT_OUTPUT
#pragma ACME enable CORRADE_INTERNAL_ASSERT
#pragma ACME enable CORRADE_INTERNAL_CONSTEXPR_ASSERT
#pragma ACME enable CORRADE_INTERNAL_ASSERT_OUTPUT
#pragma ACME enable CORRADE_ASSERT_UNREACHABLE

#include "Corrade/Containers/Reference.h"
#ifdef CORRADE_POINTER_STL_COMPATIBILITY
// {{includes}}
#include "Corrade/Containers/ReferenceStl.h"
#endif
