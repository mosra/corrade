#ifndef Corrade_Utility_Arguments_h
#define Corrade_Utility_Arguments_h
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
 * @brief Class @ref Corrade::Utility::Arguments
 */

#include <string>
#include <utility>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/StringStl.h"
#include "Corrade/Utility/ConfigurationValue.h"
#include "Corrade/Utility/StlForwardVector.h"
#include "Corrade/Utility/Utility.h"
#include "Corrade/Utility/visibility.h"

#ifdef CORRADE_BUILD_DEPRECATED
#include "Corrade/Utility/Macros.h"
#endif

namespace Corrade { namespace Utility {

/**
@brief Command-line argument parser

Parses Unix-style command line, with positional and named arguments and options
both in a short (e.g., `-o file`) and long variant (e.g., `--output file`),
boolean options and array options. If needed, positional arguments can be
separated from named ones using `--`; short options can be packed together
(e.g. `-xzOfile.dat` is equivalent to `-x -z -O file.dat` providing `-x` and
`-z` are boolean options).

The parsing is semi-autonomous, which means that the parser will also exit with
failure or print help text (and exit) on its own. if `-h` or `--help` is
given anywhere on the command line, the parser prints full help text to the
output and exits, ignoring all other arguments. If a parse error occurs
(missing/unknown argument etc.), the parser prints a shorter variant of the
help text and exits.

@section Utility-Arguments-usage Example usage

Contrived example of command-line utility which prints given text given number
of times, optionally redirecting the output to a file:

@snippet Utility.cpp Arguments-usage

Upon requesting help, the utility prints the following:

@code{.shell-session}
Usage
  ./printer [-h|--help] -n|--repeat REPEAT [-v|--verbose] [--log LOG] [--] text

Repeats the text given number of times.

Arguments:
  text                 the text to print
  -h, --help           display this help message and exit
  -n, --repeat REPEAT  repeat count
  -v, --verbose        log verbosely
  --log LOG            save verbose log to given file
                       (default: log.txt)
@endcode

It doesn't end with just the above, check out the @ref addArrayArgument(),
@ref addArrayOption() and @ref addFinalOptionalArgument() APIs for more
involved uses.

@section Utility-Arguments-delegating Delegating arguments to different parts of the application

Sometimes you want to have some set of arguments for the application and some
for the underlying library (or libraries) without one interfering with another
and without writing code that would delegate the options from one to another.
It is possible to do it using prefixed arguments. The library would use (and
verify) only options with given prefix and on the other hand, the application
would skip those instead of reporting them as unknown. The prefixed arguments
are restricted to non-boolean options with long names to keep the usage simple
both for the application author and users. Example:

@snippet Utility.cpp Arguments-delegating

The application can be then called like the following, the prefixed and
unprefixed options and named arguments can be mixed without restriction:

@code{.sh}
./printer --repeat 30 --formatter-width 80 --formatter-color ff3366 "hello there"
@endcode

Upon calling `-h` or `--help` the application prints the following:

@code{.shell-session}
Usage
  ./printer [-h|--help] [--formatter-...] -n|--repeat REPEAT [--] text

Repeats the text given number of times.

Arguments:
  text                 the text to print
  -h, --help           display this help message and exit
  -n, --repeat REPEAT  repeat count
  --formatter-...      formatter options
                       (see --formatter-help for details)
@endcode

Upon calling `--formatter-help` the application prints the following:

@code{.shell-session}
Usage
  ./printer [--formatter-help] [--formatter-width WIDTH] [--formatter-color COLOR] ...

Arguments:
  ...                      main application arguments
                           (see -h or --help for details)
  --formatter-help         display this help message and exit
  --formatter-width WIDTH  number of columns
                           (default: 80)
  --formatter-color COLOR  output color
                           (default: auto)
@endcode

Boolean options would cause parsing ambiguity so they are not allowed, but you
can work around the limitation like this, for example:

@snippet Utility.cpp Arguments-delegating-bool

With @ref Flag::IgnoreUnknownOptions it's also possible for multiple subsystems
to share just a subset of the same prefixed options, ignoring the unknown ones.
However in order to have a good user experience, the first instance should
always understand all options to be able to provide full help text and properly
react to unknown options.

@snippet Utility.cpp Arguments-delegating-ignore-unknown

@section Utility-Arguments-parse-error-callback Advanced parsing logic

By default, when a parse error is encountered (such as a missing or superfluous
argument), @ref parse() exits the program. However sometimes the default logic
might not be flexible enough for your needs. Setting a callback via
@ref setParseErrorCallback() allows you to override this behavior on a
per-error basis. For example, the following will allow `output` to not be
specified when `--info` is passed:

@snippet Utility.cpp Arguments-parse-error-callback

Note that the autogenerated help text only understands the default logic and
thus you should explicitly mention special cases via @ref setGlobalHelp().
*/
class CORRADE_UTILITY_EXPORT Arguments {
    public:
        /**
         * @brief Flag
         *
         * @see @ref Flags, @ref Arguments(Flags),
         *      @ref Arguments(const std::string&, Flags)
         */
        enum class Flag: std::uint8_t {
            /**
             * For prefixed arguments (constructed with
             * @ref Arguments(const std::string&, Flags)) this makes
             * @ref parse() ignore unknown options. See
             * @ref Utility-Arguments-delegating for a complete overview about
             * delegating options and usage of this flag.
             *
             * It's not allowed to use this flag on unprefixed arguments.
             * @m_since{2019,10}
             */
            IgnoreUnknownOptions = 1 << 0
        };

        /**
         * @brief Flags
         *
         * @see @ref Arguments(Flags),
         *      @ref Arguments(const std::string&, Flags)
         */
        typedef Containers::EnumSet<Flag> Flags;

        /**
         * @brief Parse error
         * @m_since{2020,06}
         *
         * @see @ref setParseErrorCallback(),
         *      @ref Utility-Arguments-parse-error-callback
         */
        enum class ParseError: std::uint8_t {
            /**
             * Either an invalid one-letter argument (i.e., not satisfying the
             * `[a-zA-Z0-9]` regex). The callback receives the key, which is
             * always a single character (thus without the leading `-`). If not
             * handled, the default diagnostic is for example:
             *
             * @code{.shell-session}
             * Invalid command-line argument -?
             * @endcode
             *
             * Or a long argument with just one leading dash, in which case the
             * callback receives the multi-character argument name (again
             * without the leading `-`). If not handled, the default diagnostic
             * is for example:
             *
             * @code{.shell-session}
             * Invalid command-line argument -foo (did you mean --foo?)
             * @endcode
             */
            InvalidShortArgument,

            /**
             * Invalid long argument (i.e., not satisfying the `[a-zA-Z0-9-]+`
             * regex). The function receives the key without the leading `--`.
             * If not handled, the default diagnostic is for example:
             *
             * @code{.shell-session}
             * Invalid command-line argument --foo?
             * @endcode
             */
            InvalidArgument,

            /**
             * A short argument that was not added with @ref addArgument(),
             * @ref addNamedArgument(), @ref addOption() or
             * @ref addBooleanOption(). The function receives the key without
             * the leading `-`, and it's always a single character. If not
             * handled, the default diagnostic is for example:
             *
             * @code{.shell-session}
             * Unknown command-line argument -v
             * @endcode
             */
            UnknownShortArgument,

            /**
             * A short argument that was not added with @ref addArgument(),
             * @ref addNamedArgument(), @ref addOption() or
             * @ref addBooleanOption(). The function receives the key without
             * the leading `--`. If not handled, the default diagnostic is for
             * example:
             *
             * @code{.shell-session}
             * Unknown command-line argument --foo
             * @endcode
             */
            UnknownArgument,

            /**
             * Superfluous unnamed argument (i.e., there's more than how many
             * was added with @ref addArgument()). The function receives the
             * full argument value. If not handled, the default diagnostic is
             * for example:
             *
             * @code{.shell-session}
             * Superfluous command-line argument /dev/null
             * @endcode
             */
            SuperfluousArgument,

            /**
             * Missing value for an argument. Happens when a named argument or
             * non-boolean option name is specified as the last element of the
             * argument list and no value follows. The function receives the
             * long key name (even if short key might be specified on the
             * command line). At this point all arguments are parsed and you
             * can query the instance
             *
             * If not handled, the default diagnostic is for example:
             *
             * @code{.shell-session}
             * Missing value for command-line argument --output
             * @endcode
             */
            MissingValue,

            /**
             * Missing argument. The function receives the long key name. At
             * this point all arguments are parsed and you can access them via
             * @ref value() and @ref isSet(). If not handled, the default
             * diagnostic is for example:
             *
             * @code{.shell-session}
             * Missing command-line argument output
             * @endcode
             */
            MissingArgument
        };

        /**
         * @brief Parse error callback
         * @m_since{2020,06}
         *
         * @see @ref setParseErrorCallback(),
         *      @ref Utility-Arguments-parse-error-callback
         */
        typedef bool(*ParseErrorCallback)(const Arguments&, ParseError, const std::string&);

        /**
         * @brief Environment values
         *
         * Returns list of all environment values for information and debugging
         * purposes, encoded in UTF-8.
         * @note In @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten" the environment
         *      list is concatenated from local Emscripten environment and
         *      system environment provided by Node.js.
         * @see @ref setFromEnvironment()
         * @partialsupport Returns empty vector on
         *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT".
         */
        static std::vector<std::string> environment();

        /** @brief Constructor */
        explicit Arguments(Flags flags = {});

        /**
         * @brief Construct prefixed arguments
         *
         * Prefixed arguments are useful for example when you have some options
         * related to the application and some to the underlying library and
         * you want to handle them in separate steps. Prefixed version can have
         * only named arguments and long options.
         *
         * See class documentation for an example.
         * @see @ref addSkippedPrefix()
         */
        explicit Arguments(const std::string& prefix, Flags flags = {});

        /** @brief Copying is not allowed */
        Arguments(const Arguments&) = delete;

        /**
         * @brief Move constructor
         *
         * A moved-out instance behaves the same as newly created instance.
         */
        Arguments(Arguments&& other) noexcept;

        /** @brief Copying is not allowed */
        Arguments& operator=(const Arguments&) = delete;

        /**
         * @brief Move assignment
         *
         * Swaps contents of the two instances.
         */
        Arguments& operator=(Arguments&& other) noexcept;

        ~Arguments();

        /**
         * @brief Argument prefix
         *
         * If the class was instantiated with @ref Arguments(const std::string&, Flags),
         * returns the specified prefix. Otherwise returns empty string.
         */
        std::string prefix() const;

        /**
         * @brief Whether the arguments were successfully parsed
         *
         * Returns @cpp true @ce if @ref parse() was successfully called,
         * @cpp false @ce otherwise.
         */
        bool isParsed() const;

        /**
         * @brief Add mandatory argument
         *
         * After calling @cpp addArgument("argument") @ce the argument will be
         * displayed in argument list like the following. Call @ref setHelp()
         * to change the displayed key:
         *
         * @code{.shell-session}
         * Usage:
         *   ./app [--] argument
         *
         * Arguments:
         *   argument          help text
         * @endcode
         *
         * If no help text is set, the argument is not displayed in the
         * argument list. Call @ref setHelp() to set it. Argument value can be
         * retrieved using @ref value().
         *
         * Only non-boolean options are allowed in the prefixed version, no
         * arguments --- use @ref addOption() in that case instead.
         * @see @ref addArrayArgument(), @ref addFinalOptionalArgument()
         */
        Arguments& addArgument(std::string key);

        /**
         * @brief Add a mandatory array argument
         * @m_since_latest
         *
         * Compared to @ref addArgument(), which requires exactly one argument
         * to be present, this function requires one or more arguments. There
         * can be only one array argument and this function can't be combined
         * with @ref addFinalOptionalArgument(), but it can be placed at any
         * position relative to other positional arguments.
         *
         * After calling @cpp addArrayArgument("argument") @ce the option will
         * be displayed in help text like the following. Call @ref setHelp() to
         * change the displayed key:
         *
         * @code{.shell-session}
         * Usage:
         *   ./app [--] argument...
         *
         * Arguments:
         *   argument          help text
         * @endcode
         *
         * If no help text is set, the argument is not displayed in the
         * argument list. Call @ref setHelp() to set it. Array length and
         * values can be retrieved using @ref arrayValueCount() and
         * @ref arrayValue().
         *
         * Only non-boolean options are allowed in the prefixed version, no
         * arguments --- use @ref addArrayArgument() in that case instead.
         * @see @ref addFinalOptionalArgument(), @ref addArrayOption()
         */
        Arguments& addArrayArgument(std::string key);

        /**
         * @brief Add named mandatory argument with both short and long key alternative
         *
         * After calling @cpp addNamedArgument('a', "argument") @ce the
         * argument will be displayed in help text like the following. Argument
         * value is just uppercased key value, call @ref setHelp() to change
         * it:
         *
         * @code{.shell-session}
         * Usage:
         *   ./app -a|--argument ARGUMENT
         *
         * Arguments:
         *   -a, --argument    help text
         * @endcode
         *
         * If no help text is set, the argument is not displayed in the
         * argument list. Call @ref setHelp() to set it. Argument value can be
         * retrieved using @ref value().
         *
         * Only non-boolean options are allowed in the prefixed version, no
         * arguments --- use @ref addOption() in that case instead.
         */
        Arguments& addNamedArgument(char shortKey, std::string key);

        /**
         * @brief Add named mandatory argument with long key only
         *
         * Similar to the above, the only difference is that the usage and help
         * text does not mention the short option:
         *
         * @code{.shell-session}
         * Usage:
         *   ./app --argument ARGUMENT
         *
         * Arguments:
         *   --argument        help text
         * @endcode
         *
         * If no help text is set, the argument is not displayed in the
         * argument list. Call @ref setHelp() to set it. Argument value can be
         * retrieved using @ref value().
         *
         * Only non-boolean options are allowed in the prefixed version, no
         * arguments --- use @ref addOption() in that case instead.
         */
        Arguments& addNamedArgument(std::string key) {
            return addNamedArgument('\0', std::move(key));
        }

        /**
         * @brief Add option with both short and long key alternative
         *
         * After calling @cpp addOption('o', "option") @ce the option will be
         * displayed in help text like the following. Option value is just
         * uppercased key value, call @ref setHelp() to change it:
         *
         * @code{.shell-session}
         * Usage:
         *   ./app [-o|--option OPTION]
         * @endcode
         *
         * Default value, if nonempty, is displayed in option list like the
         * following, call @ref setHelp() to add descriptional help text. If
         * default value is empty and no help text is set, the option is not
         * displayed in the list at all.
         *
         * @code{.shell-session}
         * Arguments:
         *   -o, --option      help text
         *                     (default: defaultValue)
         * @endcode
         *
         * If no help text is set, the option is not displayed in the argument
         * list. Call @ref setHelp() to set it. Option value can be retrieved
         * using @ref value().
         *
         * Short key is not allowed in the prefixed version, use
         * @ref addOption(std::string, std::string) in that case instead.
         * @see @ref addArrayOption(), @ref addBooleanOption()
         */
        Arguments& addOption(char shortKey, std::string key, std::string defaultValue = std::string());

        /**
         * @brief Add option with long key only
         *
         * Similar to the above, the only difference is that the usage and help
         * text does not mention the short option:
         *
         * @code{.shell-session}
         * Usage:
         *   ./app [--option OPTION]
         *
         * Arguments:
         *   --option          help text
         *                     (default: defaultValue)
         * @endcode
         *
         * @see @ref addNamedArgument(), @ref addFinalOptionalArgument()
         */
        Arguments& addOption(std::string key, std::string defaultValue = std::string()) {
            return addOption('\0', std::move(key), std::move(defaultValue));
        }

        /**
         * @brief Add an array option with both short and long key alternative
         * @m_since{2020,06}
         *
         * Compared to @ref addOption(), which remembers only the last value
         * when multiple options of the same name are passed in the argument
         * list, this function remembers the whole sequence. That also means
         * there's no default value, the default is simply an empty sequence.
         *
         * After calling @cpp addArrayOption('o', "option") @ce the option will
         * be displayed in help text like the following. Option value is just
         * uppercased key value, call @ref setHelp() to change it:
         *
         * @code{.shell-session}
         * Usage:
         *   ./app [-o|--option OPTION]...
         *
         * Arguments:
         *   -o, --option      help text
         * @endcode
         *
         * If no help text is set, the option is not displayed in the argument
         * list. Call @ref setHelp() to set it. Array length and values can be
         * retrieved using @ref arrayValueCount() and @ref arrayValue().
         *
         * Short key is not allowed in the prefixed version, use
         * @ref addArrayOption(std::string) in that case instead.
         * @see @ref addArrayArgument()
         */
        Arguments& addArrayOption(char shortKey, std::string key);

        /**
         * @brief Add an array option with long key only
         * @m_since{2020,06}
         *
         * Similar to the above, the only difference is that the usage and help
         * text does not mention the short option:
         *
         * @code{.shell-session}
         * Usage:
         *   ./app [--option OPTION]...
         *
         * Arguments:
         *   --option          help text
         * @endcode
         */
        Arguments& addArrayOption(std::string key) {
            return addArrayOption('\0', std::move(key));
        }

        /**
         * @brief Add boolean option with both short and long key alternative
         *
         * If the option is present, the option has a @cpp true @ce value,
         * otherwise it has a @cpp false @ce value. Unlike above functions, the
         * usage text does not display the option value and you need to set a
         * help text with @ref setHelp() to make it appear in option list:
         *
         * @code{.shell-session}
         * Usage:
         *   ./app [-o|-option]
         *
         * Arguments:
         *   -o, --option      help text
         * @endcode
         *
         * If no help text is set, the option is not displayed in the argument
         * list. Call @ref setHelp() to set it, however setting displayed
         * key name in @ref setHelp() is not possible with boolean options.
         * Option presence can be queried with @ref isSet() Option for getting
         * help (`-h`, `--help`) is added automatically.
         *
         * Only non-boolean options are allowed in the prefixed version, use
         * @ref addOption() in that case instead.
         * @see @ref addOption(), @ref addArrayOption()
         */
        Arguments& addBooleanOption(char shortKey, std::string key);

        /**
         * @brief Add boolean option with long key only
         *
         * Similar to the above, the only difference is that the usage and help
         * text does not mention the short option:
         *
         * @code{.shell-session}
         * Usage:
         *   ./app [--option]
         *
         * Arguments:
         *   --option          help text
         * @endcode
         */
        Arguments& addBooleanOption(std::string key) {
            return addBooleanOption('\0', std::move(key));
        }

        /**
         * @brief Add final optional argument
         * @m_since{2019,10}
         *
         * Always parsed as the last after all other unnamed arguments.
         * Compared to arguments added with @ref addArgument() this one doesn't
         * need to be present; compared to options added with @ref addOption()
         * it doesn't need to be specified together with option name. There can
         * be only one final optional argument and this function can't be
         * combined with @ref addArrayArgument().
         *
         * After calling @cpp addFinalOptionalArgument("argument") @ce the
         * argument will be displayed in help text like the following. Call
         * @ref setHelp() to change the displayed key:
         *
         * @code{.shell-session}
         * Usage:
         *   ./app [--] [argument]
         *
         * Arguments:
         *   argument          help text
         *                     (default: defaultValue)
         * @endcode
         *
         * If no help text is set, the argument is not displayed in the
         * argument list. Call @ref setHelp() to set it. Argument value can be
         * retrieved using @ref value().
         *
         * Only non-boolean options are allowed in the prefixed version, no
         * arguments --- use @ref addOption() in that case instead.
         * @see @ref addArrayArgument()
         */
        Arguments& addFinalOptionalArgument(std::string key, std::string defaultValue = std::string());

        /**
         * @brief Skip given prefix
         *
         * Ignores all options with given prefix. See class documentation for
         * details.
         */
        Arguments& addSkippedPrefix(std::string prefix, std::string help = {});

        /**
         * @brief Set option from environment
         *
         * Allows the option to be taken from environment variable if it is not
         * specified on command line. If @p environmentVariable is not set,
         * uppercase @p key value with dashes converted to underscores is used
         * by default. For example, on Unix-based systems, calling
         * @cpp setFromEnvironment("some-option") @ce allows you to specify
         * that option either using
         *
         * @code{.sh}
         * ./app --some-option 42
         * @endcode
         *
         * or
         *
         * @code{.sh}
         * SOME_OPTION=42 ./app
         * @endcode
         *
         * Boolean options are set to @cpp true @ce if the environment value is
         * set to `ON` (case-insensitive). Values are encoded in UTF-8.
         * @note In @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten" the environment
         *      is combined from local Emscripten environment and system
         *      environment provided by Node.js. If a variable is in both
         *      environments, the local environment is preferred.
         * @see @ref environment()
         * @partialsupport Does nothing on @ref CORRADE_TARGET_WINDOWS_RT "Windows RT".
         */
        #ifndef CORRADE_TARGET_WINDOWS_RT
        Arguments& setFromEnvironment(const std::string& key, std::string environmentVariable);
        Arguments& setFromEnvironment(const std::string& key); /**< @overload */
        #else
        template<class T, class U> Arguments& setFromEnvironment(T&&, U&&) {
            return *this;
        }
        template<class T> Arguments& setFromEnvironment(T&&) {
            return *this;
        }
        #endif

        /**
         * @brief Set command name
         *
         * If empty, the command name is extracted from arguments passed to
         * @ref parse() on parsing, or set to @cb{.sh} ./app @ce if not parsed
         * yet. The command name is then used in @ref usage() and @ref help().
         * Default is empty.
         * @see @ref setHelp()
         */
        Arguments& setCommand(std::string name);

        /**
         * @brief Set global help text
         *
         * If nonempty, the text is printed between usage text and argument and
         * option list. Default is none.
         *
         * Help text can be set only in the unprefixed version.
         * @see @ref setCommand()
         */
        Arguments& setGlobalHelp(std::string help);

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @brief @copybrief setGlobalHelp()
         * @m_deprecated_since{2019,10} Use @ref setGlobalHelp() instead.
         */
        CORRADE_DEPRECATED("use setGlobalHelp() instead") Arguments& setHelp(std::string help);
        #endif

        /**
         * @brief Set help text for given key
         *
         * Arguments, boolean options and options with empty default values
         * are not displayed in argument and option list unless they have help
         * text set.
         *
         * If @p helpKey is set, it replaces the placeholder for arguments and
         * uppercased placeholder in named arguments and nonboolean options.
         * For example, calling @cpp setHelp("input", "...", "file.bin") @ce
         * and @cpp setHelp("limit", "...", "N") @ce will transform the
         * following usage text:
         *
         * @code{.shell-session}
         * ./app --limit LIMIT input
         * @endcode
         *
         * to:
         *
         * @code{.shell-session}
         * ./app --limit N file.bin
         * @endcode
         *
         * The displayed keys are changed also in argument and option list.
         */
        Arguments& setHelp(const std::string& key, std::string help, std::string helpKey = {});

        /**
         * @brief Parse error callback
         * @m_since{2020,06}
         *
         * The default callback unconditionally returns @cpp false @ce.
         */
        ParseErrorCallback parseErrorCallback() const { return _parseErrorCallback; }

        /**
         * @brief Parse error callback state
         * @m_since{2020,06}
         *
         * The default state is @cpp nullptr @ce.
         */
        void* parseErrorCallbackState() const { return _parseErrorCallbackState; }

        /**
         * @brief Set parse error callback
         * @m_since{2020,06}
         *
         * The @p callback function receives a reference to this instance, a
         * @ref ParseError enum describing what exactly is wrong, and a
         * corresponding key name or command-line argument value on which the
         * error occured. If the callback returns @cpp false @ce, an error
         * message is printed and the program exits. If the callback returns
         * @cpp true @ce, the error is ignored (assumed the application handles
         * it gracefully) and parsing continues. The callback is also allowed
         * to print an error message on its own and then call @ref std::exit()
         * directly to override the default diagnostic.
         *
         * The @p state pointer is saved and can be retrieved using
         * @ref parseErrorCallbackState() inside the callback. Unless said
         * otherwise for a particular @ref ParseError, you can't call
         * @ref value() or @ref isSet() from the callback as the arguments are
         * not parsed yet.
         *
         * See @ref Utility-Arguments-parse-error-callback for an example and
         * particular @ref ParseError values for detailed behavior of every
         * error.
         */
        Arguments& setParseErrorCallback(ParseErrorCallback callback, void* state = nullptr);

        /**
         * @brief Parse the arguments and exit on failure
         *
         * If the arguments contain `-h` or `--help` option, the function
         * prints full help text and exits the program with `0`. If there is
         * parsing error (e.g. too little or too many arguments, unknown
         * options etc.), the function prints just the usage text and exits the
         * program with `1`.
         * @see @ref tryParse(), @ref usage(), @ref help()
         */
        void parse(int argc, const char** argv);

        /** @overload */
        void parse(int argc, char** argv) {
            parse(argc, const_cast<const char**>(argv));
        }

        /** @overload */
        void parse(int argc, std::nullptr_t argv) {
            parse(argc, static_cast<const char**>(argv));
        }

        /**
         * @brief Try parsing the arguments
         *
         * Unlike @ref parse() the function does not exit on failure, but
         * returns @cpp false @ce instead. If the user requested help, no
         * additional arguments are parsed, only `--help` option is set and
         * @cpp true @ce is returned.
         */
        bool tryParse(int argc, const char** argv);

        /** @overload */
        bool tryParse(int argc, char** argv) {
            return tryParse(argc, const_cast<const char**>(argv));
        }

        /** @overload */
        bool tryParse(int argc, std::nullptr_t argv) {
            return tryParse(argc, static_cast<const char**>(argv));
        }

        /**
         * @brief Usage string
         *
         * Returns usage string which is printed on parsing error.
         * @see @ref setCommand(), @ref help()
         */
        std::string usage() const;

        /**
         * @brief Full help text string
         *
         * Returns full help text which is printed on `-h` or `--help` request.
         * @see @ref setCommand(), @ref setHelp(), @ref usage()
         */
        std::string help() const;

        /**
         * @brief Value of given argument or option
         * @param key       Long argument or option key
         * @param flags     Configuration value flags
         *
         * Expects that the key exists and @ref parse() was successful. Only
         * for non-array arguments and non-array non-boolean options, use
         * @ref arrayValue() or @ref isSet() for those instead. If @p T is not
         * @ref std::string, uses @ref ConfigurationValue::fromString() to
         * convert the value to given type.
         */
        template<class T = std::string> T value(const std::string& key, ConfigurationValueFlags flags = {}) const;

        /**
         * @brief Count of parsed values in given array argument or option
         * @param key       Array argument or option key
         * @m_since{2020,06}
         *
         * Expects that the key exists, @ref parse() was successful and
         * @p key is an array argument or option.
         * @see @ref addArrayArgument(), @ref addArrayOption()
         */
        std::size_t arrayValueCount(const std::string& key) const;

        /**
         * @brief Value of given array argument or option
         * @param key       Array argument or option key
         * @param id        Array value index
         * @param flags     Configuration value flags
         * @m_since{2020,06}
         *
         * Expects that the key exists, @ref parse() was successful and
         * @p id is less than @ref arrayValueCount(). Only for array arguments
         * and options, use @ref value() or @ref isSet() for those instead. If
         * @p T is not @ref std::string, uses
         * @ref ConfigurationValue::fromString() to convert the value to given
         * type.
         * @see @ref addArrayArgument(), @ref addArrayOption()
         */
        template<class T = std::string> T arrayValue(const std::string& key, const std::size_t id, ConfigurationValueFlags flags = {}) const;

        /**
         * @brief Whether boolean option is set
         * @param key   Long option key
         *
         * Expects that the option exists, was added using
         * @ref addBooleanOption() and @ref parse() was successful. The help
         * option (`-h`, `--help`) is added implicitly.
         * @see @ref value(), @ref arrayValue()
         */
        bool isSet(const std::string& key) const;

    private:
        enum class Type: std::uint8_t;
        enum class InternalFlag: std::uint8_t;
        typedef Containers::EnumSet<InternalFlag> InternalFlags;
        CORRADE_ENUMSET_FRIEND_OPERATORS(InternalFlags)

        struct CORRADE_UTILITY_LOCAL Entry;

        CORRADE_UTILITY_LOCAL void addOptionInternal(char shortKey, std::string key, std::string helpKey, std::string defaultValue, Type type, std::size_t id, const char* assertPrefix);
        bool CORRADE_UTILITY_LOCAL skippedPrefix(const std::string& key) const;
        bool CORRADE_UTILITY_LOCAL verifyKey(const std::string& key) const;
        bool CORRADE_UTILITY_LOCAL verifyKey(char shortKey) const;
        CORRADE_UTILITY_LOCAL Entry* find(const std::string& key);
        CORRADE_UTILITY_LOCAL const Entry* find(const std::string& key) const;
        CORRADE_UTILITY_LOCAL const Entry* find(char shortKey) const;

        std::string CORRADE_UTILITY_LOCAL keyName(const Entry& entry) const;

        const std::string& valueInternal(const std::string& key) const;
        const std::string& arrayValueInternal(const std::string& key, std::size_t id) const;

        InternalFlags _flags;
        /* not std::size_t so it fits into the padding after flags */
        std::uint16_t _finalOptionalArgument{}, _arrayArgument{};
        std::string _prefix;
        std::string _command;
        std::string _help;
        Containers::Array<Entry> _entries;
        Containers::Array<std::string> _values;
        /* Three nested allocations. Feels kinda noobish, eh? */
        Containers::Array<Containers::Array<std::string>> _arrayValues;
        Containers::Array<std::pair<std::string, std::string>> _skippedPrefixes;
        Containers::Array<bool> _booleans;
        ParseErrorCallback _parseErrorCallback;
        void* _parseErrorCallbackState;
};

/**
@debugoperatorclassenum{Arguments,Arguments::ParseError}
@m_since{2020,06}
*/
CORRADE_UTILITY_EXPORT Debug& operator<<(Debug& debug, Arguments::ParseError value);

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> inline std::string Arguments::value(const std::string& key, ConfigurationValueFlags) const {
    return valueInternal(key);
}
#endif

template<class T> T Arguments::value(const std::string& key, ConfigurationValueFlags flags) const {
    const std::string& value = valueInternal(key);
    return value.empty() ? T() : ConfigurationValue<T>::fromString(value, flags);
}

template<class T> T Arguments::arrayValue(const std::string& key, std::size_t id, ConfigurationValueFlags flags) const {
    const std::string& value = arrayValueInternal(key, id);
    return value.empty() ? T() : ConfigurationValue<T>::fromString(value, flags);
}

}}

#endif
