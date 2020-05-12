#ifndef Corrade_Utility_Sha1_h
#define Corrade_Utility_Sha1_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2019 Jonathan Hale <squareys@googlemail.com>

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
 * @brief Class @ref Corrade::Utility::Sha1
 */

#include "Corrade/Containers/Containers.h"
#include "Corrade/Utility/AbstractHash.h"
#include "Corrade/Utility/StlForwardString.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief SHA-1

Implementation of the [Secure Hash Algorithm 1](https://en.wikipedia.org/wiki/SHA-1).
Example usage:

@snippet Utility.cpp Sha1-usage
*/
class CORRADE_UTILITY_EXPORT Sha1: public AbstractHash<20> {
    public:
        /**
         * @brief Digest of given data
         *
         * Convenience function for @cpp (Utility::Sha1{} << data).digest() @ce.
         */
        static Digest digest(const std::string& data) {
            return (Sha1() << data).digest();
        }

        explicit Sha1();

        /** @brief Add data for digesting */
        Sha1& operator<<(Containers::ArrayView<const char> data);

        /** @overload */
        Sha1& operator<<(const std::string& data);

        /**
         * @brief @cpp operator<< @ce with C strings is not allowed
         *
         * To clarify your intent with handling the @cpp '\0' @ce delimiter,
         * cast to @ref Containers::ArrayView or @ref std::string instead.
         */
        Sha1& operator<<(const char*) = delete;

        /** @brief Digest of all added data */
        Digest digest();

    private:
        CORRADE_UTILITY_LOCAL void processChunk(const char* data);

        char _buffer[128];
        std::size_t _bufferSize = 0;
        unsigned long long _dataSize = 0;
        unsigned int _digest[5];
};

}}

#endif
