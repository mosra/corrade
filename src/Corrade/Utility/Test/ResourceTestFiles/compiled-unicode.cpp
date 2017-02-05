/* Compiled resource file. DO NOT EDIT! */

#include "Corrade/Corrade.h"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Resource.h"

CORRADE_ALIGNAS(4) static const unsigned char resourcePositions[] = {
    0x0c,0x00,0x00,0x00,
    0x08,0x00,0x00,0x00
};

static const unsigned char resourceFilenames[] = {
    /* hýždě.bin */
    0x68,0xc3,0xbd,0xc5,0xbe,0x64,0xc4,0x9b,0x2e,0x62,0x69,0x6e
};

static const unsigned char resourceData[] = {
    /* hýždě.bin */
    0xd1,0x5e,0xa5,0xed,0xea,0xdd,0x00,0x0d
};

int resourceInitializer_ResourceTestUtf8Data();
int resourceInitializer_ResourceTestUtf8Data() {
    Corrade::Utility::Resource::registerData("unicode", 1, resourcePositions, resourceFilenames, resourceData);
    return 1;
} CORRADE_AUTOMATIC_INITIALIZER(resourceInitializer_ResourceTestUtf8Data)

int resourceFinalizer_ResourceTestUtf8Data();
int resourceFinalizer_ResourceTestUtf8Data() {
    Corrade::Utility::Resource::unregisterData("unicode");
    return 1;
} CORRADE_AUTOMATIC_FINALIZER(resourceFinalizer_ResourceTestUtf8Data)
