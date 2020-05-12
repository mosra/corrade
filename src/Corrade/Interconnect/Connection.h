#ifndef Corrade_Interconnect_Connection_h
#define Corrade_Interconnect_Connection_h
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

/** @file
 * @brief Class @ref Corrade::Interconnect::Connection
 */

#include <cstddef>

#include "Corrade/Containers/Reference.h"
#include "Corrade/Interconnect/Interconnect.h"
#include "Corrade/Interconnect/visibility.h"

namespace Corrade { namespace Interconnect {

namespace Implementation {
    struct SignalDataHash;

    enum: std::size_t { FunctionPointerSize =
        #ifndef CORRADE_TARGET_WINDOWS
        2*sizeof(void*)/sizeof(std::size_t)
        #else
        /* On MSVC, pointers to members with a virtual base class
           are the biggest and have 16 bytes both on 32bit and 64bit. */
        16/sizeof(std::size_t)
        #endif
    };

    class SignalData {
        public:
            #ifndef CORRADE_MSVC2019_COMPATIBILITY
            template<class Emitter, class ...Args> SignalData(typename Emitter::Signal(Emitter::*signal)(Args...)): data() {
                typedef typename Emitter::Signal(Emitter::*BaseSignal)(Args...);
                *reinterpret_cast<BaseSignal*>(data) = signal;
            }
            #else
            /* MSVC is not able to detect template parameters, so I need to
               shovel these in explicitly using "static constructor" */
            template<class Emitter, class ...Args> static SignalData create(typename Emitter::Signal(Emitter::*signal)(Args...)) {
                /* Member function pointers on Windows have different size
                   based on whether the class has no/single inheritance,
                   multiple inheritance or virtual inheritance. Casting to
                   (Emitter::*) which has no inheritance (like done for other
                   platforms) would thus lose information and cause problems
                   when classes with multiple or virtual inheritance are used,
                   so we create a new type, virtually inherited from emitter,
                   cast the pointer to that and then save its representation. */
                SignalData d;
                struct VirtuallyInheritedEmitter: virtual Emitter {};
                typedef typename Emitter::Signal(VirtuallyInheritedEmitter::*VirtuallyDerivedSignal)(Args...);
                *reinterpret_cast<VirtuallyDerivedSignal*>(d.data) = signal;
                return d;
            }
            #endif

            bool operator==(const SignalData& other) const {
                for(std::size_t i = 0; i != FunctionPointerSize; ++i)
                    if(data[i] != other.data[i]) return false;
                return true;
            }

            bool operator!=(const SignalData& other) const {
                return !operator==(other);
            }

        private:
            /* https://bugzilla.gnome.org/show_bug.cgi?id=776986 */
            #ifndef DOXYGEN_GENERATING_OUTPUT
            friend Interconnect::Emitter;
            friend SignalDataHash;
            #endif

            #ifdef CORRADE_MSVC2019_COMPATIBILITY
            SignalData(): data() {}
            #endif

            std::size_t data[FunctionPointerSize];
    };
}

/**
@brief Connection

Returned by @ref Interconnect::connect(), allows to remove the connection
later using @ref Interconnect::disconnect(). Destruction of the @ref Connection
object does not remove the connection, after that the only possibility to
remove the connection is to disconnect the whole emitter or receiver or
disconnect everything connected to given signal using
@ref Emitter::disconnectSignal(), @ref Emitter::disconnectAllSignals() or
@ref Receiver::disconnectAllSlots(), or destroy either the emitter or receiver
object.

@see @ref interconnect, @ref Emitter, @ref Receiver
*/
class CORRADE_INTERCONNECT_EXPORT Connection {
    public:
        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @brief Whether the connection exists
         * @m_deprecated_since{2019,10} This function is dangerous as it has no
         *      way to check that the original @ref Emitter object still
         *      exists, use @ref Emitter::isConnected() instead.
         */
        CORRADE_DEPRECATED("dangerous, use Emitter::isConnected() instead") bool isConnected() const;

        /**
         * @brief Remove the connection
         * @m_deprecated_since{2019,10} This function is dangerous as it has no
         *      way to check that the original @ref Emitter object still
         *      exists, use @ref Interconnect::disconnect() instead.
         */
        CORRADE_DEPRECATED("dangerous, use Interconnect::disconnect() instead") void disconnect();

        /**
         * @brief Whether connection is possible
         * @m_deprecated_since{2019,10} Re-connecting a disconnected signal is
         *      not possible anymore in order to make the library more
         *      efficient. This function now just returns the value of (also
         *      deprecated) @ref isConnected().
         */
        CORRADE_DEPRECATED("re-connecting a disconnected signal is not possible anymore") bool isConnectionPossible() const {
            CORRADE_IGNORE_DEPRECATED_PUSH
            return isConnected();
            CORRADE_IGNORE_DEPRECATED_POP
        }

        /**
         * @brief Re-establish the connection
         * @m_deprecated_since{2019,10} Re-connecting a disconnected signal is
         *      not possible anymore in order to make the library more
         *      efficient. This function now just returns the value of (also
         *      deprecated) @ref isConnected().
         */
        CORRADE_DEPRECATED("re-connecting a disconnected signal is not possible anymore") bool connect() {
            CORRADE_IGNORE_DEPRECATED_PUSH
            return isConnected();
            CORRADE_IGNORE_DEPRECATED_POP
        }
        #endif

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #endif
        explicit Connection(
            #ifdef CORRADE_BUILD_DEPRECATED
            Emitter& emitter,
            #endif
            Implementation::SignalData signal, Implementation::ConnectionData& data);

    private:
        /* https://bugzilla.gnome.org/show_bug.cgi?id=776986 */
        #ifndef DOXYGEN_GENERATING_OUTPUT
        friend Emitter;
        friend Receiver;
        friend CORRADE_INTERCONNECT_EXPORT bool disconnect(Emitter&, const Connection&);
        #endif

        #ifdef CORRADE_BUILD_DEPRECATED
        Containers::Reference<Emitter> _emitter;
        #endif
        Implementation::SignalData _signal;
        /* Note: this might become dangling at some point */
        Implementation::ConnectionData* _data;
};

}}

#endif
