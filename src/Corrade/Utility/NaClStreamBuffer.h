#ifndef Corrade_Utility_NaClStreamBuffer_h
#define Corrade_Utility_NaClStreamBuffer_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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

/** @file
 * @brief Class @ref Corrade::Utility::NaClConsoleStreamBuffer, @ref Corrade::Utility::NaClMessageStreamBuffer
 */

#include <sstream>
#include <ppapi/c/ppb_console.h>

#include "Corrade/compatibility.h"
#include "Corrade/Utility/visibility.h"

#ifndef CORRADE_TARGET_NACL
#error This file is available only in Google Chrome Native Client target
#endif

namespace pp {
    class Instance;
}

namespace Corrade { namespace Utility {

/**
@brief Stream buffer that sends the data to JavaScript console

Usable in conjunction with `std::ostream` to redirect the output to JavaScript
console. The data are sent on each `flush()` call and then the internal buffer
is cleared. The data are written line by line to avoid exceeding log message
limit. Example usage:
@code
NaClConsoleStreamBuffer buffer(instance, NaClConsoleStreamBuffer::LogLevel::Log);

std::ostream out(&buffer);

out << "Hello World!" << std::endl;
@endcode
The output stream can be also used with @ref Debug classes.
@see @ref NaClMessageStreamBuffer
@partialsupport Available only in @ref CORRADE_TARGET_NACL "NaCl", see
    @ref AndroidLogStreamBuffer for similar functionality in
    @ref CORRADE_TARGET_ANDROID "Android".
@todo Remove line-by-line when Chrome/NaCl SDK has this fixed
*/
class CORRADE_UTILITY_EXPORT NaClConsoleStreamBuffer: public std::stringbuf {
    public:
        /**
         * @brief Log level
         *
         * @see NaClConsoleStreamBuffer()
         */
        enum class LogLevel: std::uint32_t {
            Tip = PP_LOGLEVEL_TIP,          /**< Tip */
            Log = PP_LOGLEVEL_LOG,          /**< Log */
            Warning = PP_LOGLEVEL_WARNING,  /**< Warning */
            Error = PP_LOGLEVEL_ERROR       /**< Error */
        };

        /**
         * @brief Constructor
         * @param instance  Native Client module instance
         * @param level     Log level
         * @param source    Message source. If empty, module name is used.
         */
        explicit NaClConsoleStreamBuffer(pp::Instance* instance, LogLevel level, std::string source = std::string());

        ~NaClConsoleStreamBuffer();

    protected:
        /**
         * @brief Send current data to JavaScript console
         *
         * After sending the message the internal data buffer is cleared to
         * avoid sending the same data repeatedly.
         */
        int sync() override;

    private:
        pp::Instance* instance;
        LogLevel level;
        std::string source;
};

/**
@brief Stream buffer that sends the data as message to JavaScript

Usable in conjunction with `std::ostream` to pass the output as messages to
JavaScript. The data are sent on each `flush()` call and then the internal
buffer is cleared. You can set message prefix to differentiate among various
outputs. Example usage:
@code
NaClMessageStreamBuffer buffer(instance, "Log: ");

std::ostream out(&buffer);

out << "Hello World!" << std::endl;
@endcode

In JavaScript you then need to just add event listener, for example:
@code
var listener = document.getElementById('listener');
listener.addEventListener('message', function(message) {
    alert(message.data);
}, true);
@endcode

The output stream can be also used with @ref Debug classes.
@see @ref NaClConsoleStreamBuffer
@partialsupport Available only in @ref CORRADE_TARGET_NACL "NaCl".
*/
class CORRADE_UTILITY_EXPORT NaClMessageStreamBuffer: public std::stringbuf {
    public:
        /**
         * @brief Constructor
         * @param instance  Native Client module instance
         * @param prefix    Prefix for each sent message.
         */
        explicit NaClMessageStreamBuffer(pp::Instance* instance, std::string prefix = std::string());

        ~NaClMessageStreamBuffer();

    protected:
        /**
         * @brief Send current data to JavaScript console
         *
         * After sending the message the internal data buffer is cleared to
         * avoid sending the same data repeatedly.
         */
        int sync() override;

    private:
        pp::Instance* instance;
        std::string prefix;
};

}}

#endif
