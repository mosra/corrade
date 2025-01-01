/* Compiled resource file. DO NOT EDIT! */

#include "Corrade/Corrade.h"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Resource.h"

#if CORRADE_RESOURCE_VERSION != 2
#ifdef CORRADE_TARGET_CLANG
#pragma GCC error "resource file compiled in version 2 but version " _CORRADE_HELPER_STR2(CORRADE_RESOURCE_VERSION) " expected, update your corrade-rc binary"
#else
#error resource file compiled in unexpected version 2, update your corrade-rc binary
#if defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_MSVC)
#pragma message("resource file version " _CORRADE_HELPER_STR2(CORRADE_RESOURCE_VERSION) " expected instead")
#endif
#endif
#endif

namespace {

/* Pair `i` is offset of filename `i + 1` in the low 24 bits, padding after
   data `i` in the upper 8 bits, and a 32bit offset of data `i + 1`. Offset of
   the first filename and data is implicitly 0. */
const unsigned int resourcePositions[] = {
    0x00000009,0x00000000
};

const unsigned char resourceFilenames[] = {
    /* empty.bin */
    0x65,0x6d,0x70,0x74,0x79,0x2e,0x62,0x69,0x6e
};

// const unsigned char resourceData[] = {
    /* empty.bin */
// };

Corrade::Utility::Implementation::ResourceGroup resource;

}

int corradeResourceInitializer_ResourceTestData();
int corradeResourceInitializer_ResourceTestData() {
    resource.name = "test";
    resource.count = 1;
    resource.positions = resourcePositions;
    resource.filenames = resourceFilenames;
    resource.data = nullptr;
    Corrade::Utility::Resource::registerData(resource);
    return 1;
} CORRADE_AUTOMATIC_INITIALIZER(corradeResourceInitializer_ResourceTestData)

int corradeResourceFinalizer_ResourceTestData();
int corradeResourceFinalizer_ResourceTestData() {
    Corrade::Utility::Resource::unregisterData(resource);
    return 1;
} CORRADE_AUTOMATIC_FINALIZER(corradeResourceFinalizer_ResourceTestData)
