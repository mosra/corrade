#ifndef Corrade_Containers_ScopedExit_h
#define Corrade_Containers_ScopedExit_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::Containers::ScopedExit
 */

#include <Corrade/configure.h>

namespace Corrade { namespace Containers {

/**
@brief Scoped exit

Use it to call close/destroy/exit functions on pointer-like or integer-like
object handles at the end of scope. Useful when you have many early returns and
want to ensure the exit function gets called every time. Example:

@snippet Containers.cpp ScopedExit-usage

You can also specify a non-capturing lambda for more involved operations. Note
that the handle is copied by value, so references won't work.

@snippet Containers.cpp ScopedExit-lambda

@attention
    Note that due to @ref CORRADE_MSVC2015_COMPATIBILITY "MSVC 2015" limitation
    ([source](https://developercommunity.visualstudio.com/content/problem/155715/visual-c-compiler-is-unable-to-convert-lambda-clos.html))
    it's not possible to use lambdas with non-@cpp void @ce return type. Either
    remove the @cpp return @ce statement or explicitly convert the lambda to a
    function pointer before passing it to the constructor to work aroung the
    limitation:
@attention
    @snippet Containers.cpp ScopedExit-returning-lambda
*/
class ScopedExit {
    public:
        /** @brief Constructor */
        template<class T, class Deleter> explicit ScopedExit(T handle, Deleter deleter);

        #ifdef CORRADE_MSVC2015_COMPATIBILITY
        template<class T, class U> explicit ScopedExit(T handle, U(*deleter)(T));
        #endif

        /** @brief Copying is not allowed */
        ScopedExit(const ScopedExit&) = delete;

        /** @brief Moving is not allowed */
        ScopedExit(ScopedExit&&) = delete;

        /** @brief Copying is not allowed */
        ScopedExit& operator=(const ScopedExit&) = delete;

        /** @brief Moving is not allowed */
        ScopedExit& operator=(ScopedExit&&) = delete;

        /**
         * @brief Release the handle ownership
         *
         * Causes the deleter passed in constructor to not get called on
         * destruction.
         */
        void release() { _deleterWrapper = nullptr; }

        /**
         * @brief Destructor
         *
         * Executes the deleter passed in constructor. Does nothing if
         * @ref release() has been called.
         */
        ~ScopedExit() {
            if(_deleterWrapper) _deleterWrapper(&_deleter, &_handle);
        }

    private:
        void(*_deleterWrapper)(void(**)(), void**);
        void(*_deleter)();
        void* _handle;
};

template<class T, class Deleter> ScopedExit::ScopedExit(T handle, Deleter deleter): _deleter{
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    reinterpret_cast<void(*)()>(+deleter) /* https://stackoverflow.com/a/18889029 */
    #else
    reinterpret_cast<void(*)()>(static_cast<void(*)(T)>(deleter)) /* Details why below */
    #endif
}, _handle{reinterpret_cast<void*>(handle)} {
    static_assert(sizeof(T) <= sizeof(void*), "handle too big to store");
    _deleterWrapper = [](void(**deleter)(), void** handle) {
        (*reinterpret_cast<Deleter*>(deleter))(*reinterpret_cast<T*>(handle));
    };
}

#ifdef CORRADE_MSVC2015_COMPATIBILITY
/* MSVC 2015 has ambiguous operator+ on lambdas, thus I need to work around
   that by converting the lambda to a function pointer explicitly above. There
   is no chance of knowing the return type, so assuming the lambda is void.
   That's a limitation compared to all other tystems, but since I want the
   normal function pointers to work well regardless of the return type, I need
   this overload as well. Source:
   https://developercommunity.visualstudio.com/content/problem/155715/visual-c-compiler-is-unable-to-convert-lambda-clos.html */
template<class T, class U> ScopedExit::ScopedExit(T handle, U(*deleter)(T)): _deleter{reinterpret_cast<void(*)()>(deleter)}, _handle{reinterpret_cast<void*>(handle)} {
    static_assert(sizeof(T) <= sizeof(void*), "handle too big to store");
    _deleterWrapper = [](void(**deleter)(), void** handle) {
        (*reinterpret_cast<U(**)(T)>(deleter))(*reinterpret_cast<T*>(handle));
    };
}
#endif

}}

#endif
