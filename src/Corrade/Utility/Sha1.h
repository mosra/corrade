#ifndef Corrade_Utility_Sha1_h
#define Corrade_Utility_Sha1_h
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
 * @brief Class @ref Corrade::Utility::Sha1
 */

#include <string>

#include "Corrade/Utility/AbstractHash.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/** @brief SHA-1 */
class CORRADE_UTILITY_EXPORT Sha1: public AbstractHash<20> {
    public:
        /**
         * @brief Digest of given data
         *
         * Convenience function for
         * @code
         * (Sha1() << data).digest()
         * @endcode
         */
        static Digest digest(const std::string& data) {
            return (Sha1() << data).digest();
        }

        explicit Sha1();

        /** @brief Add data for digesting */
        Sha1& operator<<(const std::string& data);

        /** @brief Digest of all added data */
        Digest digest();

    private:
        CORRADE_UTILITY_LOCAL void processChunk(const char* data);

        std::string _buffer;
        unsigned long long _dataSize;
        unsigned int _digest[5];
};

}}

#endif
