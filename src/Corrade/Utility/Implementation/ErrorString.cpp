/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
              Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2020 Jonathan Hale <squareys@googlemail.com>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include "ErrorString.h"

#ifdef CORRADE_TARGET_WINDOWS
#include <string>

#include "Corrade/Utility/Unicode.h"
#include "Corrade/Containers/ScopeGuard.h"

#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN
#include <windows.h>
#endif

namespace Corrade { namespace Utility { namespace Implementation {

#ifdef CORRADE_TARGET_WINDOWS
std::string windowsErrorString(unsigned int errorCode) {
    WCHAR* errorStringW = nullptr;
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,
        nullptr, errorCode, 0, reinterpret_cast<LPWSTR>(&errorStringW),
        0, nullptr);
    Containers::ScopeGuard e{errorStringW,
        #ifdef CORRADE_MSVC2015_COMPATIBILITY
        /* MSVC 2015 is unable to cast the parameter for LocalFree */
        [](WCHAR* p){ LocalFree(p); }
        #else
        LocalFree
        #endif
    };

    /* Convert to UTF-8 and cut off final newline that FormatMessages adds */
    return Unicode::narrow(Containers::arrayView<const wchar_t>(errorStringW,
        wcslen(errorStringW)).except(1));
}
#endif

}}}
