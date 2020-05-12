#ifndef Corrade_Utility_AndroidLogStreamBuffer_h
#define Corrade_Utility_AndroidLogStreamBuffer_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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

#if defined(CORRADE_TARGET_ANDROID) || defined(DOXYGEN_GENERATING_OUTPUT)
/** @file
 * @brief Class @ref Corrade::Utility::AndroidLogStreamBuffer
 */
#endif

#include "Corrade/configure.h"

#if defined(CORRADE_TARGET_ANDROID) || defined(DOXYGEN_GENERATING_OUTPUT)
#include <sstream>
#include <android/log.h>

#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief Stream buffer that sends the data to Android log

Usable in conjunction with @ref std::ostream to redirect the output to Android
log buffer, which can later be accessed through the @cb{.sh} adb logcat @ce
utility. The data are sent on each @ref std::ostream::flush() call (which is
called implicitly on each @ref std::endl) and then the internal buffer is
cleared. Example usage:

@snippet android.cpp AndroidLogStreamBuffer

From the console you can then use @cb{.sh} adb logcat @ce and filter out
everything except the @cpp "my-application" @ce tag. The output might then look
like this:

@code{.shell-session}
$ adb logcat *:S my-application
...
03-16 17:02:21.203 16442 16442 I my-application: Hello World!
@endcode

The output stream can be also used with @ref Debug classes --- simply pass the
@ref std::ostream instance to the constructor.
@partialsupport Available only on @ref CORRADE_TARGET_ANDROID "Android".
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

    private:
        int sync() override;

        LogPriority _priority;
        std::string _tag;
};

}}

#else
#error this file is available only on Android build
#endif

#endif
