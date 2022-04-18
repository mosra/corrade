#ifndef Corrade_Utility_Resource_h
#define Corrade_Utility_Resource_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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

/** @file
 * @brief Class @ref Corrade::Utility::Resource
 */

#include "Corrade/Containers/Containers.h"
#include "Corrade/Utility/visibility.h"

#ifdef CORRADE_BUILD_DEPRECATED
/* Most APIs took a std::string before, provide implicit conversions */
#include "Corrade/Containers/StringStl.h"
#endif

namespace Corrade { namespace Utility {

namespace Implementation {
    struct ResourceGroup;
}

/**
@brief Access to compiled-in resources

This class provides access to data files compiled into the executable using the
@ref corrade-cmake-add-resource "corrade_add_resource()" CMake macro or the
@ref corrade-rc "corrade-rc" utility.

@m_class{m-note m-success}

@par
    The @ref resource-management tutorial shows a complete setup for
    compiled-in resources including a CMake project.

@section Utility-Resource-compilation Resource compilation

Resources are organized in groups, where a group is a set of files that are
encoded in a hexadecimal form into a single `*.cpp` file which is then compiled
alongside your other sources.

The @ref corrade-rc "corrade-rc" executable and the
@ref corrade-cmake-add-resource "corrade_add_resource()" CMake macro take a
configuration file as an input, which lists files to be compiled as resources.
All filenames are expected to be in UTF-8. A configuration file can look for
example like this, with syntax matching what @ref Configuration understands:

@code{.ini}
group=game-data

[file]
filename=license.txt

[file]
filename=../resources/intro-new-final.ogg
alias=intro.ogg

[file]
filename=levels/insane.conf
alias=levels/easy.conf
@endcode

The @cb{.ini} group @ce is an identifier that you'll subsequently pass to the
@ref Resource() constructor, each @cb{.ini} [file] @ce section then describes
one file to be compiled in, with paths relative to location of the
configuration file. By default, the @cb{.ini} filename @ce is the name under
which the files will be available when calling @ref getRaw() or @ref getString()
later, including any directory separators. Use the @cpp alias @ce option to
override the name.

There can be just one resource group or several, organization of the files is
completely up to you. For example, if there's a set of files used only by a
particular library but not other parts of the application, it might be useful
to have them in a dedicated group. Or if there's a lot of files, you might wish
to split them up into multiple groups to speed up the compilation and reduce
compiler memory use.

@subsection Utility-Resource-compilation-cmake Using CMake

Assuming the above file was named `resources.conf`, the following CMake snippet
will compile the referenced files into a C++ source stored inside the build
directory. Its filename gets saved into a @cb{.sh} ${MyGame_RESOURCES} @ce
variable, which subsequently gets passed to the @cb{.cmake} add_executable() @ce
call:

@code{.cmake}
corrade_add_resource(MyGame_RESOURCES resources.conf)

add_executable(MyGame … ${MyGame_RESOURCES})
@endcode

The @ref corrade-cmake-add-resource "corrade_add_resource()" macro also takes
care of dependency management --- if either the configuration file or any files
referenced by it are changed, it triggers a recompilation of the resources,
same as with usual C++ sources.

The variable name also acts as a name used for symbols in the generated file
--- it has to be a valid C identifier and has to be unique among all resources
compiled into the same executable. But apart from that, you'd need the name
only if you deal with @ref Utility-Resource-usage-static "resources in static libraries"
as explained below.

@m_class{m-block m-danger}

@par Resources and CMake targets in different directories
    Due to limitations of the CMake @cmake add_custom_command() @ce command,
    it's important to have the @cmake corrade_add_resource() @ce call in the
    same directory as the @cmake add_executable() @ce or
    @cmake add_library() @ce consuming its output. Otherwise CMake will attempt
    to find the to-be-generated file already during a configure step and fail.
@par
    If you have the `resources.conf` file in a different directory, you can
    reference it via an absolute or relative path, such as
@par
    @code{.cmake}
    corrade_add_resource(MyGame_RESOURCES Data/resources.conf)
    @endcode

@subsection Utility-Resource-compilation-manual Compiling the resources manually

If you're not using CMake, you can execute the @ref corrade-rc "corrade-rc"
utility manually to produce a C++ file that you then compile together with your
project. The following invocation would be equivalent to the above CMake macro
call:

@code{.shell-session}
corrade-rc MyGame_RESOURCES path/to/resources.conf output.cpp
@endcode

This will generate `output.cpp` in current directory, which you then compile
together with your sources. The first parameter is again a name used for the
symbols in the generated file.

@section Utility-Resource-usage Accessing the resources

If you compiled the resources directly into an executable or into a shared
library, you can access them from the C++ code without having to do anything
else. First instantiate the class with a group name matching the
@cb{.ini} group @ce value in the configuration file, and then access the files
by their filenames:

@snippet Utility.cpp Resource-usage

Because the data are coming from a readonly memory inside the executable
itself, the class returns non-owning views. In most conditions you can assume
unlimited lifetime of the data, see @ref getRaw() and @ref getString() for
details.

@subsection Utility-Resource-usage-static Resources in static libraries

If you compile the resources into a static library, the linker will implicitly
treat the data as unreferenced and won't include them in the final executable,
leading to a not-found assertion during @ref Resource construction. To prevent
this, you need to reference them. This can be done using the
@ref CORRADE_RESOURCE_INITIALIZE() macro, to which you pass the symbol name
used in the CMake macro or command-line invocation earlier:

@snippet Utility.cpp Resource-usage-static

It's important to call it outside of any namespace, otherwise you'll get a
linker error. The @cpp main() @ce function is ideal for this, or you can create
a dedicated function outside a namespace and then call it from within a
namespace.

@subsection Utility-Resource-usage-override Overriding compiled-in resources

For shorter turnaround times when iterating on compiled-in resources it's
possible to override them at runtime using @ref overrideGroup(). That way you
won't need to wait for a recompilation, relink and restart of the application
when making changes --- instead you tell the application itself to fetch the
data from the same location the resource compiler would, by pointing it to
the original `resource.conf` file on disk:

@snippet Utility.cpp Resource-usage-override

@ref Resource instance created after this point will parse the configuration
file and fetch the data from there, or fall back to the compiled-in resource on
error. The files get cached for the lifetime of a particular @ref Resource
instance, any subsequent changes in files thus get picked up only next time an
instance is created.

@m_class{m-note m-success}

@par
    See also @ref Tweakable and @ref FileWatcher for other means of runtime
    editing without recompilation and file-change-triggered operations.

@section Utility-Resource-access Memory access and operation complexity

Resource registration (either automatic or using
@ref CORRADE_RESOURCE_INITIALIZE()) is a simple operation without any heap
access or other operations that could potentially fail. When using only the
@ref hasGroup() and @ref getRaw() APIs with compile-time string literals, no
memory allocation or heap access is involved either.

The group lookup during construction and @ref hasGroup() is done with a
@f$ \mathcal{O}(n) @f$ complexity as the resource groups register themselves
into a linked list. Actual file lookup after is done in-place on the
compiled-in data in a @f$ \mathcal{O}(\log{}n) @f$ time.

@section Utility-Resource-multithreading Thread safety

The resources register themselves into a global storage. If done
implicitly, the registration is executed before entering @cpp main() @ce and
thus serially. If done explicitly via @ref CORRADE_RESOURCE_INITIALIZE() /
@ref CORRADE_RESOURCE_FINALIZE(), these macros *have to* be called from a
single thread or externally guarded to avoid data races. Same goes for the
@ref overrideGroup() function.

On the other hand, all other functionality only reads from the global storage
and thus is thread-safe.

@todo Ad-hoc resources
 */
class CORRADE_UTILITY_EXPORT Resource {
    public:
        /**
         * @brief Override a group
         * @param group         Group name
         * @param configurationFile Filename of the configuration file. Use an
         *      empty string to discard a previously set override.
         *
         * Overrides compiled-in resources of given group with live data
         * specified in given configuration file, which is useful during
         * development and debugging. Subsequently created @ref Resource
         * instances with the same group will take data from a live filesystem
         * instead and fallback to compiled-in resources only for files that
         * are not found in the overriden file.
         *
         * @attention Unlike all other methods of this class, this one is *not*
         *      thread-safe. See @ref Utility-Resource-multithreading for more
         *      information.
         */
        static void overrideGroup(Containers::StringView group, Containers::StringView configurationFile);

        /** @brief Whether given group exists */
        static bool hasGroup(Containers::StringView group);

        /**
         * @brief Constructor
         *
         * Expects that the group exists.
         * @see @ref hasGroup()
         */
        explicit Resource(Containers::StringView group);

        ~Resource();

        /**
         * @brief List of all files in the group
         *
         * The resource group has no concept of a directory hierarchy --- if
         * filenames in the input configuration file contain path separators,
         * the returned list will contain them verbatim. The returned strings
         * all have @ref Containers::StringViewFlag::Global set, but are *not*
         * @ref Containers::StringViewFlag::NullTerminated.
         *
         * Note that the list contains only the compiled-in files, no
         * additional filenames supplied by an
         * @ref Utility-Resource-usage-override "overriden group" are included.
         * This is done to avoid overrides causing unexpected behavior in code
         * that assumes a fixed set of files.
         */
        Containers::Array<Containers::StringView> list() const;

        /**
         * @brief Get resource data
         *
         * Expects that the group contains given @p filename. If the file is
         * empty, returns a zero-sized @cpp nullptr @ce view. If the file is
         * not coming from an
         * @ref Utility-Resource-usage-override "overriden group", the returned
         * view can be assumed to have unlimited lifetime, otherwise it's alive
         * only until the next @ref overrideGroup() call on the same group.
         *
         * The @p filename is expected to be in in UTF-8. Unlike with
         * @ref Path::read(), no OS-specific treatment of non-null terminated
         * strings nor any encoding conversion is done --- this function never
         * allocates.
         * @todo when get() is gone and enough time passes, this should be
         *      renamed to get() to be consistent with Path
         */
        Containers::ArrayView<const char> getRaw(Containers::StringView filename) const;

        /**
         * @brief Get resource data as a string
         * @m_since_latest
         *
         * Expects that the group contains given @p filename. If the file is
         * empty, returns a zero-sized @cpp nullptr @ce view. If the file is
         * not coming from an
         * @ref Utility-Resource-usage-override "overriden group", the returned
         * string has @ref Containers::StringViewFlag::Global set, otherwise
         * it's alive only until the next @ref overrideGroup() call on the same
         * group. The returned string is *not*
         * @ref Containers::StringViewFlag::NullTerminated.
         *
         * The @p filename is expected to be in in UTF-8. Unlike with
         * @ref Path::read(), no OS-specific treatment of non-null terminated
         * strings nor any encoding conversion is done --- this function never
         * allocates.
         */
        Containers::StringView getString(Containers::StringView filename) const;

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @copybrief getString()
         * @m_deprecated_since_latest Use @ref getString() instead.
         */
        CORRADE_DEPRECATED("use getString() instead") std::string get(const std::string& filename) const;
        #endif

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #endif
        /* Used internally from the CORRADE_RESOURCE_INITIALIZE() /
           CORRADE_RESOURCE_FINALIZE() macros */
        static void registerData(Implementation::ResourceGroup& resource);
        static void unregisterData(Implementation::ResourceGroup& resource);

    private:
        struct OverrideData;

        Implementation::ResourceGroup* _group;
        OverrideData* _overrideGroup;
};

/**
@brief Initialize a resource

If a resource is compiled into a dynamic library or directly into the
executable, it will be registered automatically thanks to the
@ref CORRADE_AUTOMATIC_INITIALIZER() macro. However, if the resource is
compiled into static library, it must be explicitly initialized via this macro,
e.g. at the beginning of @cpp main() @ce, otherwise it won't be known to
@ref Corrade::Utility::Resource "Utility::Resource". You can also wrap these
macro calls into another function (which will then be compiled into a dynamic
library or the main executable) and use the @ref CORRADE_AUTOMATIC_INITIALIZER()
macro for an automatic call:

@attention This macro should be called outside of any namespace. If you are
    running into linker errors with `resourceInitializer_*`, this could be the
    reason. If you are in a namespace and cannot call this macro from `main()`,
    try this:
@attention
    @code{.cpp}
    static void initialize() {
        CORRADE_RESOURCE_INITIALIZE(res)
    }

    namespace Foo {
        void bar() {
            initialize();

            //...
        }
    }
    @endcode

Functions called by this macro don't do any dynamic allocation or other
operations that could fail, so it's safe to call it even in restricted phases
of application execution. It's also safe to call this macro more than once.

@see @ref CORRADE_RESOURCE_FINALIZE()
*/
/* Contents of this macro are used in PluginManager/AbstractManager.h to avoid
   an include dependency. Keep in sync. */
#define CORRADE_RESOURCE_INITIALIZE(name)                                     \
    extern int resourceInitializer_##name();                                  \
    resourceInitializer_##name();

/**
@brief Finalize a resource

De-registers resource previously (even automatically) initialized via
@ref CORRADE_RESOURCE_INITIALIZE(). After this call,
@ref Corrade::Utility::Resource "Utility::Resource" will not know about given
resource anymore. Useful for example when a resource was a part of a
dynamically loaded library (or a @ref Corrade::PluginManager "plugin") and it
needs to be cleaned up after the library got unloaded again.

@attention This macro should be called outside of any namespace. See the
    @ref CORRADE_RESOURCE_INITIALIZE() macro for more information.

Functions called by this macro don't do any dynamic allocation or other
operations that could fail, so it's safe to call it even in restricted phases
of application execution. It's also safe to call this macro more than once.
*/
#define CORRADE_RESOURCE_FINALIZE(name)                                       \
    extern int resourceFinalizer_##name();                                    \
    resourceFinalizer_##name();

namespace Implementation {

struct ResourceGroup {
    const char* name;
    unsigned int count;
    const unsigned int* positions;
    const unsigned char* filenames;
    const unsigned char* data;
    /* This field shouldn't be written to by anything else than
       resourceInitializer() / resourceFinalizer(). It's zero-initilized by
       default and those use it to avoid inserting a single item to the linked
       list more than once. */
    ResourceGroup* next;
};

}

}}

#endif
