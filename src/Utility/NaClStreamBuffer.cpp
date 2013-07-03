/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
              Vladimír Vondruš <mosra@centrum.cz>

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

#include "NaClStreamBuffer.h"

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/var.h>

namespace Corrade { namespace Utility {

NaClConsoleStreamBuffer::NaClConsoleStreamBuffer(pp::Instance* const instance, const LogLevel level, std::string source): std::stringbuf(std::ios_base::out), instance(instance), level(level), source(std::move(source)) {}

NaClConsoleStreamBuffer::~NaClConsoleStreamBuffer() = default;

int NaClConsoleStreamBuffer::sync() {
    std::string message = str();

    /* Remove newline at the end, if present */
    if(message.back() == '\n') message.resize(message.size()-1);

    /* Send buffer data to console */
    if(source.empty())
        instance->LogToConsole(PP_LogLevel(level), message);
    else
        instance->LogToConsoleWithSource(PP_LogLevel(level), source, message);

    /* And clear them for next time so they aren't sent again every time */
    str({});

    return 0;
}

NaClMessageStreamBuffer::NaClMessageStreamBuffer(pp::Instance* const instance, std::string prefix): std::stringbuf(std::ios_base::out), instance(instance), prefix(std::move(prefix)) {
    str(prefix);
}

NaClMessageStreamBuffer::~NaClMessageStreamBuffer() = default;

int NaClMessageStreamBuffer::sync() {
    /* Post the data as message */
    instance->PostMessage(str());

    /* And reset the data to prefix for next time so they aren't sent again
       every time */
    str(prefix);

    return 0;
}

}}
