#ifndef Corrade_Utility_Arguments_h
#define Corrade_Utility_Arguments_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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
#include <vector>

#include "Corrade/Utility/ConfigurationValue.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief Command-line argument parser

Supports positional arguments (arguments without name), short and long options
(e.g. `-o file` or `--output file`) and named arguments (i.e. non-optional
options) along with boolean options (e.g. `--verbose`). Positional and named
arguments can be given in any order, it is possible to separate positional
arguments from option list with `--`.

The parsing is semi-autonomous, which means that the parser will exit with
failure or print help text (and exit) on its own: If `-h` or `--help` is given
as first argument (the remaining ones are then ignored), the parser prints full
help text to the output and exits. If parse error occurs (missing/unknown
argument etc.), the parser prints short usage information and exits.

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
*/
class CORRADE_UTILITY_EXPORT Arguments {
    public:
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

        explicit Arguments();

        /**
         * @brief Construct prefixed arguments
         *
         * Prefixed arguments are useful for example when you have some options
         * related to the application and some to the underlying library and
         * you want to handle them in separate steps. Prefixed version can have
         * only named arguments and long options.
         *
         * See class documentation for example.
         * @see @ref addSkippedPrefix()
         */
        explicit Arguments(const std::string& prefix);

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
         * If the class was instantiated with @ref Arguments(const std::string&),
         * returns the specified prefix. Otherwise returns empty string.
         */
        std::string prefix() const;

        /**
         * @brief Whether the arguments were successfully parsed
         *
         * Returns @cpp true @ce if @ref parse() was successfully called,
         * @cpp false @ce otherwise.
         */
        bool isParsed() const { return _isParsed; }

        /**
         * @brief Add mandatory argument
         *
         * After calling @cpp addArgument("argument") @ce the argument will be
         * displayed in argument list like the following. Call @ref setHelp()
         * to change the displayed key:
         *
         * @code{.shell-session}
         * Usage:
         *   ./app argument
         *
         * Arguments:
         *   argument          help text
         * @endcode
         *
         * If no help text is set, the argument is not displayed in argument
         * list. Call @ref setHelp() to set it. Argument value can be retrieved
         * using @ref value().
         *
         * Only non-boolean options are allowed in the prefixed version, use
         * @ref addOption() instead.
         */
        Arguments& addArgument(std::string key);

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
         * If no help text is set, the argument is not displayed in argument
         * list. Call @ref setHelp() to set it. Argument value can be retrieved
         * using @ref value().
         *
         * Only non-boolean options are allowed in the prefixed version, use
         * @ref addOption() instead.
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
         * Only non-boolean options are allowed in the prefixed version, use
         * @ref addOption() instead.
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
         * Option value can be retrieved using @ref value().
         *
         * Short key is not allowed in the prefixed version, use
         * @ref addOption(std::string, std::string) instead.
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
         */
        Arguments& addOption(std::string key, std::string defaultValue = std::string()) {
            return addOption('\0', std::move(key), std::move(defaultValue));
        }

        /**
         * @brief Add boolean option with both short and long key alternative
         *
         * If the option is present, the option has `true` value, otherwise it
         * has `false` value. Unlike above functions, the usage text does not
         * display option value and you need to set help text with
         * @ref setHelp() to make it appear in option list:
         *
         * @code{.shell-session}
         * Usage:
         *   ./app [-o|-option]
         *
         * Arguments:
         *   -o, --option      help text
         * @endcode
         *
         * Option presence can be queried with @ref isSet(). Setting displayed
         * key name in @ref setHelp() is not possible with boolean options.
         * Option for getting help (`-h`, `--help`) is added automatically.
         *
         * Only non-boolean options are allowed in the prefixed version, use
         * @ref addOption() instead.
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
         *
         * Only non-boolean options are allowed in the prefixed version, use
         * @ref addOption() instead.
         */
        Arguments& addBooleanOption(std::string key) {
            return addBooleanOption('\0', std::move(key));
        }

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
        Arguments& setHelp(std::string help);

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
         * Expects that the key exists and @ref parse() was successful. Use
         * @ref isSet() for boolean options. If @p T is not @ref std::string,
         * uses @ref ConfigurationValue::fromString() to convert the value to
         * given type.
         */
        template<class T = std::string> T value(const std::string& key, ConfigurationValueFlags flags = {}) const;

        /**
         * @brief Whether boolean option is set
         * @param key   Long option key
         *
         * Expects that the option exists, is boolean and @ref parse() was
         * successful. Help option (`-h`, `--help`) is present by default.
         * @see @ref value()
         */
        bool isSet(const std::string& key) const;

    private:
        struct CORRADE_UTILITY_LOCAL Entry;

        bool CORRADE_UTILITY_LOCAL skippedPrefix(const std::string& key) const;
        bool CORRADE_UTILITY_LOCAL verifyKey(const std::string& key) const;
        bool CORRADE_UTILITY_LOCAL verifyKey(char shortKey) const;
        std::vector<Entry>::iterator CORRADE_UTILITY_LOCAL find(const std::string& key);
        std::vector<Entry>::const_iterator CORRADE_UTILITY_LOCAL find(const std::string& key) const;
        std::vector<Entry>::iterator CORRADE_UTILITY_LOCAL find(char shortKey);
        std::vector<Entry>::iterator CORRADE_UTILITY_LOCAL findNextArgument(std::vector<Entry>::iterator start);

        std::string CORRADE_UTILITY_LOCAL keyName(const Entry& entry) const;

        std::string valueInternal(const std::string& key) const;

        bool _isParsed{false};
        std::string _prefix;
        std::string _command;
        std::string _help;
        std::vector<Entry> _entries;
        std::vector<std::string> _values;
        std::vector<std::pair<std::string, std::string>> _skippedPrefixes;
        std::vector<bool> _booleans;
};

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> inline std::string Arguments::value(const std::string& key, ConfigurationValueFlags) const {
    return valueInternal(key);
}
#endif

template<class T> T Arguments::value(const std::string& key, ConfigurationValueFlags flags) const {
    std::string value = valueInternal(key);
    return value.empty() ? T() : ConfigurationValue<T>::fromString(value, flags);
}

}}

#endif
