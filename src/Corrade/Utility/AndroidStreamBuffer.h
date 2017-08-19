#ifndef Corrade_Utility_AndroidStreamBuffer_h
#define Corrade_Utility_AndroidStreamBuffer_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::Utility::AndroidLogStreamBuffer
 */

#include <sstream>
#include <android/log.h>

#include "Corrade/Utility/visibility.h"

#ifndef CORRADE_TARGET_ANDROID
#error This file is available only in Android target
#endif

namespace Corrade { namespace Utility {

/**
@brief Stream buffer that sends the data to Android log

Usable in conjunction with `std::ostream` to redirect the output to Android log
buffer, which can later be accessed through the `logcat` utility. The data are
sent on each `flush()` call and then the internal buffer is cleared. Example
usage:
@code
AndroidLogStreamBuffer buffer(AndroidLogStreamBuffer::LogPriority::Info, "native-app");

std::ostream out(&buffer);

out << "Hello World!" << std::endl;
@endcode
The output stream can be also used with @ref Debug classes.
@partialsupport Available only in @ref CORRADE_TARGET_ANDROID "Android".
*/
class CORRADE_UTILITY_EXPORT AndroidLogStreamBuffer: public std::stringbuf {
    public:
        /**
         * @brief Log level
         *
         * @see @ref AndroidLogStreamBuffer()
         */
        enum class LogPriority: std::int32_t {
            Verbose = ANDROID_LOG_VERBOSE,  /**< Verbose debug message */
            Debug = ANDROID_LOG_DEBUG,      /**< Debug message */
            Info = ANDROID_LOG_INFO,        /**< Information */
            Warning = ANDROID_LOG_WARN,     /**< Warning */
            Error = ANDROID_LOG_ERROR,      /**< Error */
            Fatal = ANDROID_LOG_FATAL,      /**< Fatal error */
        };

        /**
         * @brief Constructor
         * @param priority  Log priority
         * @param tag       Message tag
         */
        explicit AndroidLogStreamBuffer(LogPriority priority, std::string tag);

        ~AndroidLogStreamBuffer();

    protected:
        /**
         * @brief Send current data to the log buffer
         *
         * After sending the message the internal data buffer is cleared to
         * avoid sending the same data repeatedly.
         */
        int sync() override;

    private:
        LogPriority _priority;
        std::string _tag;
};

}}

#endif
