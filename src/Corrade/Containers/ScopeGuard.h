#ifndef Corrade_Containers_ScopeGuard_h
#define Corrade_Containers_ScopeGuard_h
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
 * @brief Class @ref Corrade::Containers::ScopeGuard
 */

#include <utility>

#include "Corrade/Containers/Tags.h"

namespace Corrade { namespace Containers {

/**
@brief Scope guard

Use it to call close/destroy/exit functions on pointer-like or integer-like
object handles at the end of scope. Useful when you have many early returns and
want to ensure the exit function gets called every time. Example:

@snippet Containers.cpp ScopeGuard-usage

You can also specify a non-capturing lambda for more involved operations. Note
that the handle is copied by value, so references won't work.

@snippet Containers.cpp ScopeGuard-lambda

For calling global functions with no extra state, the constructor can also take
just a parameter-less function or lambda:

@snippet Containers.cpp ScopeGuard-usage-no-handle

<b></b>

@m_class{m-block m-warning}

@par Using lambdas in MSVC 2015
    Note that due to a @ref CORRADE_MSVC2015_COMPATIBILITY "MSVC 2015"
    limitation ([source](https://developercommunity.visualstudio.com/content/problem/155715/visual-c-compiler-is-unable-to-convert-lambda-clos.html))
    it's not possible to use lambdas with non-@cpp void @ce return type. Either
    remove the @cpp return @ce statement or explicitly convert the lambda to a
    function pointer before passing it to the constructor to work aroung the
    limitation:
@par
    @snippet Containers.cpp ScopeGuard-returning-lambda

@section Containers-ScopeGuard-deferred Deferred guard creation

Using the @ref NoCreate tag, it's possible to create an empty instance that's
populated later by moving another object over it, for example to have a
conditional guard:

@snippet Containers.cpp ScopeGuard-deferred

<b></b>

@m_class{m-block m-success}

@par Single-header version
    This class is also available as a single-header, dependency-less
    [CorradeScopeGuard.h](https://github.com/mosra/magnum-singles/tree/master/CorradeScopeGuard.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information.
*/
class ScopeGuard {
    public:
        /** @brief Constructor */
        template<class T, class Deleter> explicit ScopeGuard(T handle, Deleter deleter);

        /** @overload */
        template<class Deleter> explicit ScopeGuard(Deleter deleter);

        #ifdef CORRADE_MSVC2015_COMPATIBILITY
        template<class T, class U> explicit ScopeGuard(T handle, U(*deleter)(T));
        template<class U> explicit ScopeGuard(U(*deleter)());
        #endif

        /**
         * @brief Construct without creating a guard
         * @m_since_latest
         *
         * The constructed instance is equivalent to a moved-from state. Move
         * another object over it to make it useful.
         * @see @ref Containers-ScopeGuard-deferred
         */
        explicit ScopeGuard(NoCreateT) noexcept: _deleterWrapper{}, _deleter{}, _handle{} {}

        /** @brief Copying is not allowed */
        ScopeGuard(const ScopeGuard&) = delete;

        /**
         * @brief Move constructor
         * @m_since_latest
         */
        ScopeGuard(ScopeGuard&& other) noexcept;

        /** @brief Copying is not allowed */
        ScopeGuard& operator=(const ScopeGuard&) = delete;

        /**
         * @brief Move assignment
         * @m_since_latest
         */
        ScopeGuard& operator=(ScopeGuard&&) noexcept;

        /**
         * @brief Release the handle ownership
         *
         * Causes the deleter passed in constructor to not get called on
         * destruction. The instance is then equivalent to a moved-out state.
         */
        void release() { _deleterWrapper = nullptr; }

        /**
         * @brief Destructor
         *
         * Executes the deleter passed in constructor. Does nothing if
         * @ref release() has been called.
         */
        ~ScopeGuard() {
            if(_deleterWrapper) _deleterWrapper(&_deleter, &_handle);
        }

    private:
        void(*_deleterWrapper)(void(**)(), void**);
        void(*_deleter)();
        void* _handle;
};

template<class T, class Deleter> ScopeGuard::ScopeGuard(T handle, Deleter deleter): _deleter{
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

template<class Deleter> ScopeGuard::ScopeGuard(Deleter deleter): _deleter{
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    reinterpret_cast<void(*)()>(+deleter) /* https://stackoverflow.com/a/18889029 */
    #else
    reinterpret_cast<void(*)()>(static_cast<void(*)()>(deleter)) /* Details why below */
    #endif
}, _handle{nullptr} {
    _deleterWrapper = [](void(**deleter)(), void**) {
        (*reinterpret_cast<Deleter*>(deleter))();
    };
}

inline ScopeGuard::ScopeGuard(ScopeGuard&& other) noexcept: _deleterWrapper{other._deleterWrapper}, _deleter{other._deleter}, _handle{other._handle} {
    other._deleterWrapper = nullptr;
}

inline ScopeGuard& ScopeGuard::operator=(ScopeGuard&& other) noexcept {
    using std::swap;
    swap(other._deleterWrapper, _deleterWrapper);
    swap(other._deleter, _deleter);
    swap(other._handle, _handle);
    return *this;
}

#ifdef CORRADE_MSVC2015_COMPATIBILITY
/* MSVC 2015 has ambiguous operator+ on lambdas, thus I need to work around
   that by converting the lambda to a function pointer explicitly above. There
   is no chance of knowing the return type, so assuming the lambda is void.
   That's a limitation compared to all other tystems, but since I want the
   normal function pointers to work well regardless of the return type, I need
   this overload as well. Source:
   https://developercommunity.visualstudio.com/content/problem/155715/visual-c-compiler-is-unable-to-convert-lambda-clos.html */
template<class T, class U> ScopeGuard::ScopeGuard(T handle, U(*deleter)(T)): _deleter{reinterpret_cast<void(*)()>(deleter)}, _handle{reinterpret_cast<void*>(handle)} {
    static_assert(sizeof(T) <= sizeof(void*), "handle too big to store");
    _deleterWrapper = [](void(**deleter)(), void** handle) {
        (*reinterpret_cast<U(**)(T)>(deleter))(*reinterpret_cast<T*>(handle));
    };
}

template<class U> ScopeGuard::ScopeGuard(U(*deleter)()): _deleter{reinterpret_cast<void(*)()>(deleter)}, _handle{nullptr} {
    _deleterWrapper = [](void(**deleter)(), void**) {
        (*reinterpret_cast<U(**)()>(deleter))();
    };
}
#endif

}}

#endif
