/* Compiled resource file. DO NOT EDIT! */

#include "Corrade/Corrade.h"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Resource.h"

CORRADE_ALIGNAS(4) static const unsigned char resourcePositions[] = {
    0x12,0x00,0x00,0x00,
    0x08,0x00,0x00,0x00,
    0x21,0x00,0x00,0x00,
    0x10,0x00,0x00,0x00
};

static const unsigned char resourceFilenames[] = {
    /* predisposition.bin */
    0x70,0x72,0x65,0x64,0x69,0x73,0x70,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x2e,
    0x62,0x69,0x6e,

    /* consequence.bin */
    0x63,0x6f,0x6e,0x73,0x65,0x71,0x75,0x65,0x6e,0x63,0x65,0x2e,0x62,0x69,0x6e
};

static const unsigned char resourceData[] = {
    /* predisposition.bin */
    0xba,0xdc,0x0f,0xfe,0xeb,0xad,0xf0,0x0d,

    /* consequence.bin */
    0xd1,0x5e,0xa5,0xed,0xea,0xdd,0x00,0x0d
};

int resourceInitializer_ResourceTestData();
int resourceInitializer_ResourceTestData() {
    Corrade::Utility::Resource::registerData("test", 2, resourcePositions, resourceFilenames, resourceData);
    return 1;
} CORRADE_AUTOMATIC_INITIALIZER(resourceInitializer_ResourceTestData)

int resourceFinalizer_ResourceTestData();
int resourceFinalizer_ResourceTestData() {
    Corrade::Utility::Resource::unregisterData("test");
    return 1;
} CORRADE_AUTOMATIC_FINALIZER(resourceFinalizer_ResourceTestData)
