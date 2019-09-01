/* Compiled resource file. DO NOT EDIT! */

#include "Corrade/Corrade.h"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Resource.h"

namespace {

Corrade::Utility::Implementation::ResourceGroup resource;

}

int resourceInitializer_ResourceTestNothingData();
int resourceInitializer_ResourceTestNothingData() {
    resource = { "nothing", 0, nullptr, nullptr, nullptr, nullptr };
    Corrade::Utility::Resource::registerData(resource);
    return 1;
} CORRADE_AUTOMATIC_INITIALIZER(resourceInitializer_ResourceTestNothingData)

int resourceFinalizer_ResourceTestNothingData();
int resourceFinalizer_ResourceTestNothingData() {
    Corrade::Utility::Resource::unregisterData(resource);
    return 1;
} CORRADE_AUTOMATIC_FINALIZER(resourceFinalizer_ResourceTestNothingData)
