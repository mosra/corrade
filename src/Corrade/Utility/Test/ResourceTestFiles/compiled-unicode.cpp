/* Compiled resource file. DO NOT EDIT! */

#include "Corrade/Corrade.h"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Resource.h"

namespace {

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

int resourceInitializer_ResourceTestUtf8Data();
int resourceInitializer_ResourceTestUtf8Data() {
    resource.name = "unicode";
    resource.count = 1;
    resource.positions = resourcePositions;
    resource.filenames = resourceFilenames;
    resource.data = resourceData;
    Corrade::Utility::Resource::registerData(resource);
    return 1;
} CORRADE_AUTOMATIC_INITIALIZER(resourceInitializer_ResourceTestUtf8Data)

int resourceFinalizer_ResourceTestUtf8Data();
int resourceFinalizer_ResourceTestUtf8Data() {
    Corrade::Utility::Resource::unregisterData(resource);
    return 1;
} CORRADE_AUTOMATIC_FINALIZER(resourceFinalizer_ResourceTestUtf8Data)
