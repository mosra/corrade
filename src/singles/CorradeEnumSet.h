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

    v2020.06-1075-gdd71 (2022-10-13)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"
// {{includes}}
#include <type_traits>

/* We don't need anything from configure.h here */
#pragma ACME enable Corrade_configure_h

/* From Containers.h we need just the forward declaration with default
   arguments */
#pragma ACME enable Corrade_Containers_Containers_h

#ifndef CorradeEnumSet_h
#define CorradeEnumSet_h
namespace Corrade { namespace Containers {

template<class T, typename std::underlying_type<T>::type fullValue = typename std::underlying_type<T>::type(~0)> class EnumSet;

}}
#endif
#include "Corrade/Containers/EnumSet.h"
