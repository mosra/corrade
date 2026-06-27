#ifndef Corrade_Interconnect_Connection_h
#define Corrade_Interconnect_Connection_h
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
 * @brief Class @ref Corrade::Interconnect::Connection
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

#include "Corrade/Containers/Reference.h"
#include "Corrade/Interconnect/Interconnect.h"
#include "Corrade/Interconnect/visibility.h"

/* File deprecation warning printed in Interconnect.h */

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
            /* Still broken even on MSVC 2022. Maybe 2025 will be the year when
               MSVC can finally do plain C++11? */
            #if !defined(CORRADE_TARGET_MSVC) || defined(CORRADE_TARGET_CLANG_CL) || _MSC_VER >= 1940
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
                    if(data[i] != other.data[i])
                        return false;
                return true;
            }

            bool operator!=(const SignalData& other) const {
                return !operator==(other);
            }

        private:
            /* https://bugzilla.gnome.org/show_bug.cgi?id=776986 */
            #ifndef DOXYGEN_GENERATING_OUTPUT
            CORRADE_IGNORE_DEPRECATED_PUSH
            friend Interconnect::Emitter;
            CORRADE_IGNORE_DEPRECATED_POP
            friend SignalDataHash;
            #endif

            /* Still broken even on MSVC 2022. Maybe 2025 will be the year when
               MSVC can finally do plain C++11? */
            #if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG_CL) && _MSC_VER < 1940
            SignalData(): data() {}
            #endif

            std::size_t data[FunctionPointerSize];
    };
}

/**
@brief Connection
@m_deprecated_since_latest Design of the @ref Interconnect library relies on
    member function pointers being unique, which is impossible to guarantee
    across all platform configurations and compilers, leading to subtle
    hard-to-discover bugs. The library is thus scheduled for removal, at the
    moment with no builtin replacement.

Returned by @ref Interconnect::connect(), allows to remove the connection
later using @ref Interconnect::disconnect(). Destruction of the @ref Connection
object does not remove the connection, after that the only possibility to
remove the connection is to disconnect the whole emitter or receiver or
disconnect everything connected to given signal using
@ref Emitter::disconnectSignal(), @ref Emitter::disconnectAllSignals() or
@ref Receiver::disconnectAllSlots(), or destroy either the emitter or receiver
object.

@see @ref Emitter, @ref Receiver
*/
class CORRADE_DEPRECATED("the Interconnect library is broken by design and thus obsolete") CORRADE_INTERCONNECT_EXPORT Connection {
    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #else
    public:
    #endif
        explicit Connection(Implementation::SignalData signal, Implementation::ConnectionData& data): _signal{signal}, _data{&data} {}

    private:
        /* https://bugzilla.gnome.org/show_bug.cgi?id=776986 */
        #ifndef DOXYGEN_GENERATING_OUTPUT
        CORRADE_IGNORE_DEPRECATED_PUSH
        friend Emitter;
        friend Receiver;
        /* Interestingly enough, unlike in Emitter.h, here MinGW GCC doesn't
           warn about disconnect() being redeclared without a dllimport
           attribute. */
        friend CORRADE_INTERCONNECT_EXPORT bool disconnect(Emitter&, const Connection&);
        CORRADE_IGNORE_DEPRECATED_POP
        #endif

        Implementation::SignalData _signal;
        /* Note: this might become dangling at some point */
        Implementation::ConnectionData* _data;
};

}}
#else
#error the Interconnect library is broken by design and thus obsolete
#endif

#endif
