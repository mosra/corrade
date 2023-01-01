/* Compiled resource file. DO NOT EDIT! */

#include "Corrade/Corrade.h"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Resource.h"

namespace {

/* Pair `i` is offset of filename `i + 1` in the low 24 bits, padding after
   data `i` in the upper 8 bits, and a 32bit offset of data `i + 1`. Offset of
   the first filename and data is implicitly 0. */
const unsigned int resourcePositions[] = {
    0x01000015,0x00000012
};

const unsigned char resourceFilenames[] = {
    /* 0-null-terminated.bin */
    0x30,0x2d,0x6e,0x75,0x6c,0x6c,0x2d,0x74,0x65,0x72,0x6d,0x69,0x6e,0x61,0x74,
    0x65,0x64,0x2e,0x62,0x69,0x6e
};

const unsigned char resourceData[] = {
    /* 0-null-terminated.bin */
    0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,
    0x66,0x66,   0
};

Corrade::Utility::Implementation::ResourceGroup resource;

}

int resourceInitializer_ResourceTestNullTerminatedLastFileData();
int resourceInitializer_ResourceTestNullTerminatedLastFileData() {
    resource.name = "nullTerminatedLastFile";
    resource.count = 1;
    resource.positions = resourcePositions;
    resource.filenames = resourceFilenames;
    resource.data = resourceData;
    Corrade::Utility::Resource::registerData(resource);
    return 1;
} CORRADE_AUTOMATIC_INITIALIZER(resourceInitializer_ResourceTestNullTerminatedLastFileData)

int resourceFinalizer_ResourceTestNullTerminatedLastFileData();
int resourceFinalizer_ResourceTestNullTerminatedLastFileData() {
    Corrade::Utility::Resource::unregisterData(resource);
    return 1;
} CORRADE_AUTOMATIC_FINALIZER(resourceFinalizer_ResourceTestNullTerminatedLastFileData)
