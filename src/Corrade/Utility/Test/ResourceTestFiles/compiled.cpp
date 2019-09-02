/* Compiled resource file. DO NOT EDIT! */

#include "Corrade/Corrade.h"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Resource.h"

namespace {

const unsigned int resourcePositions[] = {
    0x0000000f,0x00000008,
    0x00000021,0x00000010
};

const unsigned char resourceFilenames[] = {
    /* consequence.bin */
    0x63,0x6f,0x6e,0x73,0x65,0x71,0x75,0x65,0x6e,0x63,0x65,0x2e,0x62,0x69,0x6e,

    /* predisposition.bin */
    0x70,0x72,0x65,0x64,0x69,0x73,0x70,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x2e,
    0x62,0x69,0x6e
};

const unsigned char resourceData[] = {
    /* consequence.bin */
    0xd1,0x5e,0xa5,0xed,0xea,0xdd,0x00,0x0d,

    /* predisposition.bin */
    0xba,0xdc,0x0f,0xfe,0xeb,0xad,0xf0,0x0d
};

Corrade::Utility::Implementation::ResourceGroup resource;

}

int resourceInitializer_ResourceTestData();
int resourceInitializer_ResourceTestData() {
    resource.name = "test";
    resource.count = 2;
    resource.positions = resourcePositions;
    resource.filenames = resourceFilenames;
    resource.data = resourceData;
    Corrade::Utility::Resource::registerData(resource);
    return 1;
} CORRADE_AUTOMATIC_INITIALIZER(resourceInitializer_ResourceTestData)

int resourceFinalizer_ResourceTestData();
int resourceFinalizer_ResourceTestData() {
    Corrade::Utility::Resource::unregisterData(resource);
    return 1;
} CORRADE_AUTOMATIC_FINALIZER(resourceFinalizer_ResourceTestData)
