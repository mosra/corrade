#ifndef Corrade_Interconnect_Receiver_h
#define Corrade_Interconnect_Receiver_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026
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

#ifdef CORRADE_BUILD_DEPRECATED
/** @file
 * @brief Class @ref Corrade::Interconnect::Receiver
 * @m_deprecated_since_latest Design of the @ref Corrade::Interconnect library
 *      relies on member function pointers being unique, which is impossible to
 *      guarantee across all platform configurations and compilers, leading to
 *      subtle hard-to-discover bugs. The library is thus scheduled for
 *      removal, at the moment with no builtin replacement.
 */
#endif

#include "Corrade/configure.h"

#ifdef CORRADE_BUILD_DEPRECATED
#include <cstddef>
#include <vector>

#include "Corrade/Interconnect/Interconnect.h" /* for file deprecation warning */
#include "Corrade/Interconnect/visibility.h"

/* File deprecation warning printed in Interconnect.h */

namespace Corrade { namespace Interconnect {

namespace Implementation { struct ReceiverConnection; }

/**
@brief Receiver object
@m_deprecated_since_latest Design of the @ref Interconnect library relies on
    member function pointers being unique, which is impossible to guarantee
    across all platform configurations and compilers, leading to subtle
    hard-to-discover bugs. The library is thus scheduled for removal, at the
    moment with no builtin replacement.

Contains member function slots.
@see @ref Emitter, @ref Connection
*/
class CORRADE_DEPRECATED("the Interconnect library is broken by design and thus obsolete") CORRADE_INTERCONNECT_EXPORT Receiver {
    public:
        explicit Receiver();

        /** @brief Copying is not allowed */
        CORRADE_IGNORE_DEPRECATED_PUSH /* GCC 4.8 warns here */
        Receiver(const Receiver&) = delete;
        CORRADE_IGNORE_DEPRECATED_POP

        /** @brief Moving is not allowed */
        CORRADE_IGNORE_DEPRECATED_PUSH /* GCC 4.8 warns here */
        Receiver(Receiver&&) = delete;
        CORRADE_IGNORE_DEPRECATED_POP

        /** @brief Copying is not allowed */
        CORRADE_IGNORE_DEPRECATED_PUSH /* GCC 4.8 warns here */
        Receiver& operator=(const Receiver&) = delete;
        CORRADE_IGNORE_DEPRECATED_POP

        /** @brief Moving is not allowed */
        CORRADE_IGNORE_DEPRECATED_PUSH /* GCC 4.8 warns here */
        Receiver& operator=(Receiver&&) = delete;
        CORRADE_IGNORE_DEPRECATED_POP

        /**
         * @brief Whether the receiver is connected to any signal
         *
         * @see @ref Emitter::hasSignalConnections(),
         *      @ref slotConnectionCount()
         */
        bool hasSlotConnections() const;

        /**
         * @brief Count of connections to this receiver slots
         *
         * @see @ref Emitter::signalConnectionCount(),
         *      @ref hasSlotConnections()
         */
        std::size_t slotConnectionCount() const;

        /**
         * @brief Disconnect everything from this receiver slots
         *
         * @see @ref Emitter::disconnectAllSignals(),
         *      @ref Connection::disconnect(),
         *      @ref Emitter::disconnectSignal(), @ref hasSlotConnections()
         */
        void disconnectAllSlots();

    protected:
        /* Nobody will need to have (and delete) Receiver*, thus this is faster
           than public pure virtual destructor */
        ~Receiver();

    private:
        /* https://bugzilla.gnome.org/show_bug.cgi?id=776986 */
        #ifndef DOXYGEN_GENERATING_OUTPUT
        friend Implementation::ConnectionData;
        CORRADE_IGNORE_DEPRECATED_PUSH
        friend Emitter;
        CORRADE_IGNORE_DEPRECATED_POP
        #endif

        std::vector<Implementation::ReceiverConnection> _connections;
};

}}
#else
#error the Interconnect library is broken by design and thus obsolete
#endif

#endif
