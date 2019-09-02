/* Compiled resource file. DO NOT EDIT! */

#include "Corrade/Corrade.h"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Resource.h"

namespace {

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

int resourceInitializer_ResourceTestData();
int resourceInitializer_ResourceTestData() {
    resource.name = "test";
    resource.count = 1;
    resource.positions = resourcePositions;
    resource.filenames = resourceFilenames;
    resource.data = nullptr;
    Corrade::Utility::Resource::registerData(resource);
    return 1;
} CORRADE_AUTOMATIC_INITIALIZER(resourceInitializer_ResourceTestData)

int resourceFinalizer_ResourceTestData();
int resourceFinalizer_ResourceTestData() {
    Corrade::Utility::Resource::unregisterData(resource);
    return 1;
} CORRADE_AUTOMATIC_FINALIZER(resourceFinalizer_ResourceTestData)
