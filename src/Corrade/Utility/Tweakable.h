#ifndef Corrade_Utility_Tweakable_h
#define Corrade_Utility_Tweakable_h
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
 * @brief Class @ref Corrade::Utility::Tweakable, macro @ref CORRADE_TWEAKABLE()
 */

#include "Corrade/configure.h"

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)) || defined(CORRADE_TARGET_EMSCRIPTEN)
#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/Pointer.h"
#include "Corrade/Utility/TweakableParser.h"
#include "Corrade/Utility/StlForwardString.h"

namespace Corrade { namespace Utility {

namespace Implementation {
    enum: std::size_t { TweakableStorageSize = 16 };
}

/**
@brief Tweakable constants

Provides a mechanism to immediately reflect changes to literals in source code
to a running application. Works best combined with a traditional hot-reload
approach (such as loading a shared library via @ref PluginManager), which can
take care of source code changes that tweakable constants alone can't.

@section Utility-Tweakable-usage Basic usage

Common usage is to first define a shorter alias to the @ref CORRADE_TWEAKABLE()
macro in every source file where you're going to use tweakable values. It's
possible to define any alias and you can also use the `CORRADE_TWEAKABLE` name
directly. Here we'll use a single underscore:

@snippet Utility.cpp Tweakable-define

After that, enable it using @ref enable() (it's disabled by default). From that
point onwards, all literals wrapped with this macro invocation will get
recognized by it. To reflect source changes in the app, periodically call
@ref update(), for best responsiveness ideally in each event loop iteration.

@snippet Utility.cpp Tweakable-wrap-update

The @ref update() function will monitor changes to contents of wrapped literals
in original source files and provide the new values next time code using these
literals is executed.

The implementation ensures the runtime-modified values are interpreted exactly
the same way as if the code would be compiled directly from the modified source
file. If that's not possible for whatever reason, @ref update() exits with an
error state.

@subsection Utility-Tweakable-usage-scope Using scopes

Not all code is running in every iteration of an event loop --- and it's not
desirable to put it there just to be able to use tweakable constants. To fix
that, there's the @ref scope() function. It takes a single-parameter function
(or a lambda) and runs the contents as if the code was placed directly in the
containing block. But for every tweakable constant inside, it remembers its
surrounding scope lambda. Then, during @ref update(), whenever one of these
constants is changed, the corresponding scope lambda gets called again (with
the same parameter). So for example this way you can execute part of a
constructor again in a response to a change of one of its init parameters:

@snippet Utility.cpp Tweakable-scope

Note that lambdas passed to @ref scope() may be called from @ref update() in a
random order and multiple times, so be sure to handle their reentrancy
properly.

@subsection Utility-Tweakable-usage-disabling Disabling tweakable values

Even though the implementation is designed for @f$ \mathcal{O}(1) @f$ lookup
of tweakable values (a hashmap lookup for the file and direct indexing for
given value), you may still want to disable it entirely. There are two possibilities:

<ul><li>
    You can remove all overhead *at compile time* by defining your alias to an
    empty value, thus all tweakable literals become just surrounded by
    parentheses:

    @snippet Utility.cpp Tweakable-disable

    If you're using `CORRADE_TWEAKABLE` directly, define it to an empty value
    * *before* including @ref Corrade/Utility/Tweakable.h. The header will
    detect that and not redefine it.

    @snippet Utility.cpp Tweakable-disable-header
</li><li>
    Or you can disable it *at runtime* by not calling @ref enable(). That'll
    still make the values go through a function call, but they are simply
    passed through without any additional hashmap lookup.
</li></ul>

In both cases the @ref scope() function is practically just executing the
passed lambda and (in case the tweakable is enabled) also saving a pointer to
it, so its performance overhead is negligible. A non-enabled instance of
@ref Tweakable is internally just one pointer with no allocations involved.

@section Utility-Tweakable-limitations Limitations

This is not magic, so it comes with a few limitations:

-   It's only possible to affect values of literals annotated by the
    @ref CORRADE_TWEAKABLE() macro or its alias, this utility is not able to
    pick up changes to code around. Neither it's able to parse any arithmetic
    expressions done inside the tweakable macros --- but unary @cpp + @ce or
    @cpp - @ce for numeric types is supported.
-   Adding a new constant on a line already containing other constants might
    result in a false success, mixing up the constant values.
-   The @ref CORRADE_TWEAKABLE() macro depends on the @cpp __COUNTER__ @ce
    preprocessor variable in order to distinguish multiple tweakable constants
    on the same line. This implies that using tweakable constants in header
    files (or `*.cpp` files that get @cpp #include @ce d in other files) will
    break the counter and confuse the @ref update() function.
-   An alias to the @ref CORRADE_TWEAKABLE() has to be defined at most once in
    the whole file and all tweakable constants in that file have to use a
    single alias. On the other hand it's possible to have different aliases in
    different files.
-   Annotated literals are required to keep their type during edits --- so it's
    not possible to change e.g. @cpp _(42.0f) @ce to @cpp _(21.0) @ce, because
    that'll change the type from @cpp float @ce to @cpp double @ce. While it
    usually generates at most a warning from the compiler, such change may
    break source code change detection in unexpected ways.
-   Tweakable variables inside code that's compiled-out by the preprocessor
    (such as various @cpp #ifdef @ce s) will confuse the runtime parser, so
    avoid them entirely.
-   For simplicity of the implementation, comments are not allowed *inside* the
    tweakable macros, only whitespace.

At the moment, the implementation is *not* thread-safe.

@section Utility-Tweakable-how-it-works How it works

For each literal annotated with @ref CORRADE_TWEAKABLE() or its alias, the
class remembers its file, line and index (in order to correctly handle multiple
literals on a single line) when the code is first executed, together with a
@ref TweakableParser instance corresponding to type of the literal known at
compile time. Affected source files are then monitored with @ref FileWatcher
for changes

Upon calling @ref update(), modified files are parsed for occurences of the
defined macro and arguments of each macro call are parsed at runtime. If there
is any change, @ref TweakableState::Success is returned and the next time code
with given annotated literal is executed (either by the caller or directly
through one of the scopes), the class will supply the updated value instead. If
no files are modified or if the modification didn't result in any literal
update, @ref TweakableState::NoChange is returned.

If parsing the updated literals fails (because of a syntax error or because the
mark is not just a literal), the @ref update() function returns
@ref TweakableState::Error and doesn't update anything, waiting for the user to
fix the error. If there is some mismatch detected (such as the constant having
a different type than before or appearing on a different line),
@ref TweakableState::Recompile is returned and you are encouraged to trigger
the classical hot-reload approach (or restart a recompiled version of the app).

@section Utility-Tweakable-extending Extending for custom types

It's possible to extend the builtin support for custom user-defined C++11
literals by providing a specialization of the @ref TweakableParser class. See
its documentation for more information.

@section Utility-Tweakable-references References

*Original idea for the implementation was taken from the
[Tweakable Constants](https://www.gamedev.net/articles/programming/general-and-gameplay-programming/tweakable-constants-r2731/)
article by Joel Davis, thanks goes to Alexey Yurchenko ([\@alexesDev](https://github.com/alexesDev))
for sharing this article.

@experimental
@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms and on
    @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten".
*/
class CORRADE_UTILITY_EXPORT Tweakable {
    public:
        /**
         * @brief Current instance
         *
         * Expects that an instance exists.
         */
        static Tweakable& instance();

        /**
         * @brief Constructor
         *
         * Makes a global instance available to the @ref CORRADE_TWEAKABLE()
         * macro. Expects no global instance exists yet. Tweakable constants
         * are disabled by default, call @ref enable() before any of them is
         * used to enable them.
         */
        explicit Tweakable();

        /** @brief Copying is not allowed */
        Tweakable(const Tweakable&) = delete;

        /** @brief Moving is not allowed */
        Tweakable(Tweakable&&) = delete;

        /** @brief Copying is not allowed */
        Tweakable& operator=(const Tweakable&) = delete;

        /** @brief Moving is not allowed */
        Tweakable& operator=(Tweakable&&) = delete;

        /**
         * @brief Destructor
         *
         * Unregisters the global instance.
         */
        ~Tweakable();

        /**
         * @brief Whether tweakable constants are enabled
         *
         * @see @ref enable(), @ref Utility-Tweakable-usage-disabling
         */
        bool isEnabled() const { return !!_data; }

        /**
         * @brief Enable tweakable constants
         *
         * Tweakable constants are disabled by default, meaning all annotated
         * constants are just a pass-through for the compiled value and
         * @ref scope() just calls the passed lambda without doing anything
         * else.
         *
         * Be sure to call this function before any tweakable constant or
         * @ref scope() is used for consistent results. Calling the function
         * again after the tweakable was already enabled will cause the
         * instance to reset all previous internal state.
         * @see @ref isEnabled(), @ref enable(const std::string&, const std::string&),
         *      @ref Utility-Tweakable-usage-disabling
         */
        void enable();

        /**
         * @brief Enable tweakable constants with a relocated file watch prefix
         *
         * The @ref enable() function implicitly uses the information from
         * preprocessor @cpp __FILE__ @ce macros to locate the source files on
         * disk. With some buildsystems the @cpp __FILE__ @ce information is
         * relative to the build directory and in other cases you may want to
         * watch files in a directory different from the source tree. This
         * function strips @p prefix from all file paths and prepends
         * @p replace to them using @ref Directory::join().
         *
         * It's possible to have either @p prefix or @p replace empty, having
         * both empty is equivalent to calling the parameter-less @ref enable().
         *
         * Be sure to call this function before any tweakable constant or
         * @ref scope() is used for consistent results. Calling the function
         * again after the tweakable was already enabled will cause the
         * instance to reset all previous internal state.
         * @see @ref isEnabled(), @ref Utility-Tweakable-usage-disabling
         */
        void enable(const std::string& prefix, const std::string& replace);

        /**
         * @brief Update the tweakable constant values
         *
         * Parses all files that changed and updates tweakable values. For
         * every value that was changed and was part of a @ref scope() call,
         * executes the corresponding scope lambda --- but every lambda only
         * once.
         *
         * If the tweakable is not enabled, does nothing and returns
         * @ref TweakableState::NoChange.
         * @see @ref isEnabled()
         */
        TweakableState update();

        /**
         * @brief Tweakable scope
         *
         * Executes passed lambda directly and also on every change to
         * tweakable variables inside the lambda. See
         * @ref Utility-Tweakable-usage-scope for an usage example.
         *
         * If the tweakable is not enabled, only calls the lambda without doing
         * anything else.
         * @see @ref isEnabled()
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class T> void scope(void(*lambda)(T&), T& userData);
        #else
        /* Otherwise the user would be forced to use the + operator to convert
           a lambda to a function pointer and (besides being weird and
           annoying) it's also not portable because it doesn't work on MSVC
           2015 and older versions of MSVC 2017. OTOH, putting this in the docs
           would say nothing about how the signature should look. */
        template<class Lambda, class T> void scope(Lambda lambda, T& userData);
        #endif

        /**
         * @brief Tweakable scope
         *
         * Equivalent to the above, but for lambdas with a generic typeless
         * parameter. Or when you don't need any parameter at all and so the
         * lambda gets just @cpp nullptr @ce.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        void scope(void(*lambda)(void*), void* userData = nullptr);
        #else /* See above why */
        template<class Lambda> void scope(Lambda lambda, void* userData = nullptr);
        #endif

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #endif
        /* Internal API used by the CORRADE_TWEAKABLE() macro */
        template<class T> T operator()(const char* filename, int line, int variable, T&& value);

    private:
        struct Data;

        std::pair<bool, void*> registerVariable(const char* file, int line, std::size_t variable, TweakableState(*parser)(Containers::StringView, Containers::StaticArrayView<Implementation::TweakableStorageSize, char>));

        void scopeInternal(void(*lambda)(void(*)(), void*), void(*userCall)(), void* userData);

        Containers::Pointer<Data> _data;
};

/** @relatesalso Corrade::Utility::Tweakable
@brief Tweakable constant annotation

See @ref Corrade::Utility::Tweakable for more information. Expects that an
instance of the class exists when this macro is used. If the tweakable is not
enabled, simply passes the value through.
*/
#ifndef CORRADE_TWEAKABLE
#define CORRADE_TWEAKABLE(...) Corrade::Utility::Tweakable::instance()(__FILE__, __LINE__, __COUNTER__, __VA_ARGS__)
#endif

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class Lambda, class T> void Tweakable::scope(Lambda lambda, T& userData) {
    auto lambdaPtr = static_cast<void(*)(T&)>(lambda);
    scopeInternal([](void(*userCall)(), void* userData) {
        reinterpret_cast<void(*)(T&)>(userCall)(*static_cast<T*>(userData));
    }, reinterpret_cast<void(*)()>(lambdaPtr), &userData);
}

template<class Lambda> void Tweakable::scope(Lambda lambda, void* userData) {
    auto lambdaPtr = static_cast<void(*)(void*)>(lambda);
    scopeInternal([](void(*userCall)(), void* userData) {
        reinterpret_cast<void(*)(void*)>(userCall)(userData);
    }, reinterpret_cast<void(*)()>(lambdaPtr), userData);
}
#endif

namespace Implementation {
    template<class T> struct TweakableTraits {
        static_assert(sizeof(T) <= TweakableStorageSize,
            "tweakable storage size too small for this type, save it via a Pointer instead");
        #if (!defined(__GNUC__) && !defined(__clang__)) || __GNUC__ >= 5 || (defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE >= 5)
        /* https://gcc.gnu.org/onlinedocs/gcc-4.9.2/libstdc++/manual/manual/status.html#status.iso.2011
           vs https://gcc.gnu.org/onlinedocs/gcc-5.5.0/libstdc++/manual/manual/status.html#status.iso.2011.
           Also, until GCC 7 it's not possible to detect what libstdc++ version
           is used when on Clang, because __GLIBCXX__ is a RELEASE DATE that
           has absolutely no relation to the version and is completely useless:
           https://gcc.gnu.org/onlinedocs/libstdc++/manual/abi.html#abi.versioning.__GLIBCXX__
           So in case of Clang I'm trying to use _GLIBCXX_RELEASE, but that
           cuts off libstdc++ 5 or libstdc++ 6 because these don't have this
           macro yet. */
        static_assert(std::is_trivially_copyable<T>::value,
            "tweakable type is not trivially copyable, use the advanced parser signature instead");
        static_assert(std::is_trivially_destructible<T>::value,
            "tweakable type is not trivially destructible, use the advanced parser signature instead");
        #endif

        static TweakableState parse(Containers::StringView value, Containers::StaticArrayView<TweakableStorageSize, char> storage) {
            std::pair<TweakableState, T> parsed = TweakableParser<T>::parse(value);
            if(parsed.first != TweakableState::Success)
                return parsed.first;

            T& current = *reinterpret_cast<T*>(storage.data());
            if(current == parsed.second)
                return TweakableState::NoChange;

            current = parsed.second;
            return TweakableState::Success;
        }
    };
}

template<class T> T Tweakable::operator()(const char* file, int line, int variable, T&& value) {
    if(!_data) return value;

    /* This function registers the variable, if not already, saving the
       file/line/counter, parser and getter function pointer. Returns a
       reference to the internal storage, which may not be initialized yet, in
       which case we save the initial value to it. */
    std::pair<bool, void*> registered = registerVariable(file, line, variable, Implementation::TweakableTraits<T>::parse);
    if(!registered.first) *static_cast<T*>(registered.second) = value;
    return *static_cast<T*>(registered.second);
}

}}
#else
#error this header is available only on Unix, non-RT Windows and Emscripten
#endif

#endif
