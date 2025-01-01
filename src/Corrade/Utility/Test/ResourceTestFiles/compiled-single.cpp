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

extern const unsigned int resourceSize_ResourceTestData = 8;
extern const unsigned char resourceData_ResourceTestData[] = {
    0xd1,0x5e,0xa5,0xed,0xea,0xdd,0x00,0x0d
};
