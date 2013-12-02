/* Compiled resource file. DO NOT EDIT! */

#include "Utility/Macros.h"
#include "Utility/Resource.h"
#include "corradeCompatibility.h"

int resourceInitializer_ResourceTestNothingData();
int resourceInitializer_ResourceTestNothingData() {
    Corrade::Utility::Resource::registerData("nothing", 0, nullptr, nullptr, nullptr);
    return 1;
} CORRADE_AUTOMATIC_INITIALIZER(resourceInitializer_ResourceTestNothingData)

int resourceFinalizer_ResourceTestNothingData();
int resourceFinalizer_ResourceTestNothingData() {
    Corrade::Utility::Resource::unregisterData("nothing");
    return 1;
} CORRADE_AUTOMATIC_FINALIZER(resourceFinalizer_ResourceTestNothingData)
