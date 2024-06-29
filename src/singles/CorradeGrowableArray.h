/*
    Growable APIs for Corrade::Containers::Array

    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1Array.html#Containers-Array-growable

    Depends on CorradeArray.h.

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    The library makes use of AddressSanitizer annotations if compiling with
    ASan enabled, you can `#define CORRADE_CONTAINERS_NO_SANITIZER_ANNOTATIONS`
    to disable them.

    v2020.06-1687-g6b5f (2024-06-29)
    -   Minor cleanup, some macro logic is now moved to CorradeArrayView.h
    v2020.06-1507-gfbd9 (2023-09-13)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"

/* CorradeArray.h is a dependency, remove all of Containers/Array.h and hide
   everything it already includes */
#pragma ACME noexpand CorradeArray.h
#pragma ACME enable Corrade_configure_h
#pragma ACME enable Corrade_Containers_Array_h
#pragma ACME enable Corrade_Tags_h
#pragma ACME enable Corrade_Containers_constructHelpers_h
#include "CorradeArray.h"

/* CORRADE_NO_STD_IS_TRIVIALLY_TRAITS, CORRADE_ASSERT and CORRADE_DEBUG_ASSERT
   are already in CorradeArray.h */

/* From Containers.h we need just the array forward declarations, the array
   view ones are again already in CorradeArrayView.h. */
#pragma ACME enable Corrade_Containers_Containers_h
#include "Corrade/Containers/GrowableArray.h"
