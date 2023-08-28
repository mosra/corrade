/*
    Corrade::Containers::Reference
        — a lightweight alternative to std::reference_wrapper

    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1Reference.html

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    v2020.06-1454-gfc3b7 (2023-08-27)
    -   The underlying type is exposed in a new Reference::Type typedef
    -   Removed unnecessary function calls for improved debug performace
    v2018.10-232-ge927d7f3 (2019-01-28)
    -   Stricter matching for external representation conversion
    -   Fixed STL compatibility to not recurse infinitely
    v2018.10-183-g4eb1adc0 (2019-01-23)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"

/* We don't need anything from configure.h here */
#pragma ACME enable Corrade_configure_h

#include "Corrade/Containers/Reference.h"
#ifdef CORRADE_REFERENCE_STL_COMPATIBILITY
// {{includes}}
#include "Corrade/Containers/ReferenceStl.h"
#endif
