/* This file is generated from Corrade {{revision}}. Do not edit directly. */

/*
    This file is part of Corrade.

    {{copyright}}

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    … <snip> …
*/

// {{includes}}

/* Remove all comments from the files to make them smaller */
#pragma ACME comments off

/* Look for #includes starting with Corrade in the corrade/src directory */
#pragma ACME local Corrade
#pragma ACME path corrade/src

/* Look for Corrade/configure.h in the CMake-generated build dir, but don't
   use its contents and provide a simplified version instead. */
#pragma ACME path corrade/build/src
#pragma ACME enable Corrade_configure_h
#ifdef _WIN32
    #define CORRADE_TARGET_WINDOWS
#elif defined(__APPLE__)
    #define CORRADE_TARGET_APPLE
#elif defined(__unix__)
    #define CORRADE_TARGET_UNIX
#endif

/* Remove things included for backwards compatibility or doxygen docs, use
   standard <cassert> */
#pragma ACME enable CORRADE_STANDARD_ASSERT
#pragma ACME disable DOXYGEN_GENERATING_OUTPUT
#pragma ACME disable CORRADE_BUILD_DEPRECATED

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/StaticArray.h"
