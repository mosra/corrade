/* Compiled resource file. DO NOT EDIT! */

#include "Corrade/Corrade.h"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Resource.h"

CORRADE_ALIGNAS(4) static const unsigned char resourcePositions[] = {
    0x09,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00
};

static const unsigned char resourceFilenames[] = {
    /* empty.bin */
    0x65,0x6d,0x70,0x74,0x79,0x2e,0x62,0x69,0x6e
};

// static const unsigned char resourceData[] = {
    /* empty.bin */
// };

int resourceInitializer_ResourceTestData();
int resourceInitializer_ResourceTestData() {
    Corrade::Utility::Resource::registerData("test", 1, resourcePositions, resourceFilenames, nullptr);
    return 1;
} CORRADE_AUTOMATIC_INITIALIZER(resourceInitializer_ResourceTestData)

int resourceFinalizer_ResourceTestData();
int resourceFinalizer_ResourceTestData() {
    Corrade::Utility::Resource::unregisterData("test");
    return 1;
} CORRADE_AUTOMATIC_FINALIZER(resourceFinalizer_ResourceTestData)
