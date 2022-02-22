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
#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Utility/Debug.h"

#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN
#include <windows.h>
#endif

namespace Corrade { namespace Utility { namespace Implementation {

#ifdef CORRADE_TARGET_WINDOWS
void printWindowsErrorString(Debug& debug, unsigned int errorCode) {
    WCHAR errorStringW[256];
    const std::size_t sizeW = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorCode, 0, errorStringW, Containers::arraySize(errorStringW), nullptr);

    /* Cut off final newline that FormatMessage adds and convert to UTF-8. Yes,
       a \r\n, IT'S WINDOWS, BABY!!! */
    char errorString[256];
    const std::size_t size = WideCharToMultiByte(CP_UTF8, 0, errorStringW, sizeW - 2, errorString, Containers::arraySize(errorString), nullptr, nullptr);
    debug << Containers::StringView{errorString, size};
}
#endif

}}}
