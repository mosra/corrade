/* Compiled resource file. DO NOT EDIT! */

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

extern const unsigned int resourceSize_ResourceTestData = 0;
extern const unsigned char resourceData_ResourceTestData[] = {
    0x00
};
