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
    0x0000000c,0x00000008
};

const unsigned char resourceFilenames[] = {
    /* hýždě.bin */
    0x68,0xc3,0xbd,0xc5,0xbe,0x64,0xc4,0x9b,0x2e,0x62,0x69,0x6e
};

const unsigned char resourceData[] = {
    /* hýždě.bin */
    0xd1,0x5e,0xa5,0xed,0xea,0xdd,0x00,0x0d
};

Corrade::Utility::Implementation::ResourceGroup resource;

}

int corradeResourceInitializer_ResourceTestUtf8Data();
int corradeResourceInitializer_ResourceTestUtf8Data() {
    resource.name = "unicode";
    resource.count = 1;
    resource.positions = resourcePositions;
    resource.filenames = resourceFilenames;
    resource.data = resourceData;
    Corrade::Utility::Resource::registerData(resource);
    return 1;
} CORRADE_AUTOMATIC_INITIALIZER(corradeResourceInitializer_ResourceTestUtf8Data)

int corradeResourceFinalizer_ResourceTestUtf8Data();
int corradeResourceFinalizer_ResourceTestUtf8Data() {
    Corrade::Utility::Resource::unregisterData(resource);
    return 1;
} CORRADE_AUTOMATIC_FINALIZER(corradeResourceFinalizer_ResourceTestUtf8Data)
