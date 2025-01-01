/* Compiled resource file. DO NOT EDIT! */

#include "Corrade/Corrade.h"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Resource.h"

#if CORRADE_RESOURCE_VERSION != 1
#ifdef CORRADE_TARGET_CLANG
#pragma GCC error "resource file compiled in version 1 but version " _CORRADE_HELPER_STR2(CORRADE_RESOURCE_VERSION) " expected, update your corrade-rc binary"
#else
#error resource file compiled in unexpected version 1, update your corrade-rc binary
#if defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_MSVC)
#pragma message("resource file version " _CORRADE_HELPER_STR2(CORRADE_RESOURCE_VERSION) " expected instead")
#endif
#endif
#endif

namespace {

Corrade::Utility::Implementation::ResourceGroup resource;

}

int resourceInitializer_ResourceTestNothingData();
int resourceInitializer_ResourceTestNothingData() {
    resource.name = "nothing";
    resource.count = 0;
    resource.positions = nullptr;
    resource.filenames = nullptr;
    resource.data = nullptr;
    Corrade::Utility::Resource::registerData(resource);
    return 1;
} CORRADE_AUTOMATIC_INITIALIZER(resourceInitializer_ResourceTestNothingData)

int resourceFinalizer_ResourceTestNothingData();
int resourceFinalizer_ResourceTestNothingData() {
    Corrade::Utility::Resource::unregisterData(resource);
    return 1;
} CORRADE_AUTOMATIC_FINALIZER(resourceFinalizer_ResourceTestNothingData)
