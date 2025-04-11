/*
    Corrade::Containers::EnumSet
        — a type-safe set of bits

    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1EnumSet.html

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    v2020.06-1890-g77f9f (2025-04-11)
    -   Cleanup and unification of SFINAE code, no functional change
    v2020.06-1506-g43e1c (2023-09-13)
    -   Preventing a conflict with the EnumSet declaration in Corrade's
        Containers.h due to default template arguments being used in both
    v2020.06-1454-gfc3b7 (2023-08-27)
    -   It's now possible to construct the EnumSet directly from the underlying
        enum's type instead of having to cast to the enum type first
    -   Removed unnecessary function calls for improved debug performace
    v2020.06-1075-gdd71 (2022-10-13)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"
// {{includes}}
#include <type_traits>

/* We don't need anything from configure.h here */
#pragma ACME enable Corrade_configure_h

#ifndef CorradeEnumSet_h
#define CorradeEnumSet_h
namespace Corrade { namespace Containers {

/* In case Corrade/Containers/Containers.h is included too, these two would
   conflict */
#ifndef Corrade_Containers_Containers_h
template<class T, typename std::underlying_type<T>::type fullValue = typename std::underlying_type<T>::type(~0)> class EnumSet;
#endif

}}
#endif
/* From Containers.h we need just the forward declaration with default
   arguments */
#pragma ACME enable Corrade_Containers_Containers_h
#include "Corrade/Containers/EnumSet.h"
