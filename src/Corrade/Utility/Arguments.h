#ifndef Corrade_Utility_Arguments_h
#define Corrade_Utility_Arguments_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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

## Example usage

Contrived example of command-line utility which prints given text given number
of times, optionally redirecting the output to a file:
@code
int main(int argc, char** argv) {
    Arguments args;
    args.addArgument("text").setHelp("text", "the text to print")
        .addNamedArgument('n', "repeat").setHelp("repeat", "repeat count")
        .addBooleanOption('v', "verbose").setHelp("verbose", "log verbosely")
        .addOption("log", "log.txt").setHelp("log", "save verbose log to given file")
        .setHelp("Repeats the text given number of times.")
        .parse(argc, argv);

    std::ofstream logOutput(args.value("log"));
    for(int i = 0; i < args.value<int>("repeat"); ++i) {
        if(args.isSet("verbose")) {
            logOutput << "Printing instance " << i << " of text " << args.value("text");
        }

        std::cout << args.value("text");
    }

    return 0;
}
@endcode

Upon requesting help, the utility prints the following:

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

*/
class CORRADE_UTILITY_EXPORT Arguments {
    public:
        explicit Arguments();

        ~Arguments();

        /**
         * @brief Add mandatory argument
         *
         * After calling `addArgument("argument")` the argument will be
         * displayed in argument list like the following. Call
         * @ref setHelpKey() to change the displayed key:
         *
         *      Usage:
         *        ./app argument
         *
         *      Arguments:
         *        argument          help text
         *
         * If no help text is set, the argument is not displayed in argument
         * list. Call @ref setHelp() to set it. Argument value can be retrieved
         * using @ref value().
         */
        Arguments& addArgument(std::string key);

        /**
         * @brief Add named mandatory argument with both short and long key alternative
         *
         * After calling <tt>addNamedArgument('a', "argument")</tt> the
         * argument will be displayed in help text like the following. Argument
         * value is just uppercased key value, call @ref setHelpKey() to change
         * it:
         *
         *      Usage:
         *        ./app -a|--argument ARGUMENT
         *
         *      Arguments:
         *        --a, --argument   help text
         *
         * If no help text is set, the argument is not displayed in argument
         * list. Call @ref setHelp() to set it. Argument value can be retrieved
         * using @ref value().
         */
        Arguments& addNamedArgument(char shortKey, std::string key);

        /**
         * @brief Add named mandatory argument with long key only
         *
         * Similar to the above, the only difference is that the usage and help
         * text does not mention the short option:
         *
         *      Usage:
         *        ./app --argument ARGUMENT
         *
         *      Arguments:
         *        --argument        help text
         */
        Arguments& addNamedArgument(std::string key) {
            return addNamedArgument('\0', std::move(key));
        }

        /**
         * @brief Add option with both short and long key alternative
         *
         * After calling <tt>addOption('o', "option")</tt> the option will be
         * displayed in help text like the following. Option value is just
         * uppercased key value, call @ref setHelpKey() to change it:
         *
         *      Usage:
         *        ./app [-o|--option OPTION]
         *
         * Default value, if nonempty, is displayed in option list like the
         * following, call @ref setHelp() to add descriptional help text. If
         * default value is empty and no help text is set, the option is not
         * displayed in the list at all.
         *
         *      Arguments:
         *        -o, --option      help text
         *                          (default: defaultValue)
         *
         * Option value can be retrieved using @ref value().
         */
        Arguments& addOption(char shortKey, std::string key, std::string defaultValue = std::string());

        /**
         * @brief Add option with long key only
         *
         * Similar to the above, the only difference is that the usage and help
         * text does not mention the short option:
         *
         *      Usage:
         *        ./app [--option OPTION]
         *
         *      Arguments:
         *        --option          help text
         *                          (default: defaultValue)
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
         *      Usage:
         *        ./app [-o|-option]
         *
         *      Arguments:
         *        -o, --option      help text
         *
         * Option presence can be queried with @ref isSet(), @ref setHelpKey()
         * cannot be used with boolean options. Option for getting help (`-h`,
         * `--help`) is added automatically.
         */
        Arguments& addBooleanOption(char shortKey, std::string key);

        /**
         * @brief Add boolean option with long key only
         *
         * Similar to the above, the only difference is that the usage and help
         * text does not mention the short option:
         *
         *      Usage:
         *        ./app [--option]
         *
         *      Arguments:
         *        --option          help text
         */
        Arguments& addBooleanOption(std::string key) {
            return addBooleanOption('\0', std::move(key));
        }

        /**
         * @brief Set command name
         *
         * If empty, the command name is extracted from arguments passed to
         * @ref parse() on parsing, or set to `./app` if not parsed yet. The
         * command name is then used in @ref usage() and @ref help(). Default
         * is empty.
         * @see @ref setHelp()
         */
        Arguments& setCommand(std::string name);

        /**
         * @brief Set global help text
         *
         * If nonempty, the text is printed between usage text and argument and
         * option list. Default is none.
         * @see @ref setCommand()
         */
        Arguments& setHelp(std::string help);

        /**
         * @brief Set help text for given key
         *
         * Arguments, boolean options and options with empty default values
         * are not displayed in argument and option list unless they have help
         * text set.
         * @see @ref setHelpKey()
         */
        Arguments& setHelp(const std::string& key, std::string help);

        /**
         * @brief Set key name displayed in help text
         *
         * For arguments the key is replaced with @p helpKey, for nonboolean
         * options the uppercased key name is replaced with @p helpKey. For
         * example, calling `setHelpKey("input", "file.bin")` and
         * `setHelpKey("limit", "N")` will transform the following usage text:
         *
         *      ./app --limit LIMIT input
         *
         * to:
         *
         *      ./app --limit N file.bin
         *
         * The displayed keys are changed also in argument and option list.
         * @see @ref setHelp()
         */
        Arguments& setHelpKey(const std::string& key, std::string helpKey);

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

        /**
         * @brief Try parsing the arguments
         *
         * Unlike @ref parse() the function does not exit on failure, but
         * returns `false` instead. If the user requested help, no additional
         * arguments are parsed, only `--help` option is set and `true` is
         * returned.
         */
        bool tryParse(int argc, const char** argv);

        /** @overload */
        bool tryParse(int argc, char** argv) {
            return tryParse(argc, const_cast<const char**>(argv));
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
         * @param key   Long argument or option key
         *
         * Expects that the key exists. Use @ref isSet() for boolean options.
         * If the arguments weren't parsed yet, returns empty string or
         * default-constructed value. If @p T is not `std::string`, uses
         * @ref ConfigurationValue::fromString() to convert the value to given
         * type.
         */
        template<class T = std::string> T value(const std::string& key) const;

        /**
         * @brief Whether boolean option is set
         * @param key   Long option key
         *
         * Expects that the option exists and is boolean. Help option (`-h`,
         * `--help`) is present by default. If the arguments weren't parsed
         * yet, returns false.
         * @see @ref value()
         */
        bool isSet(const std::string& key) const;

    private:
        struct CORRADE_UTILITY_LOCAL Entry;

        bool CORRADE_UTILITY_LOCAL verifyKey(const std::string& key) const;
        bool CORRADE_UTILITY_LOCAL verifyKey(char shortKey) const;
        std::vector<Entry>::iterator CORRADE_UTILITY_LOCAL find(const std::string& key);
        std::vector<Entry>::const_iterator CORRADE_UTILITY_LOCAL find(const std::string& key) const;
        std::vector<Entry>::iterator CORRADE_UTILITY_LOCAL find(char shortKey);
        std::vector<Entry>::const_iterator CORRADE_UTILITY_LOCAL find(char shortKey) const;
        std::vector<Entry>::iterator CORRADE_UTILITY_LOCAL findNextArgument(std::vector<Entry>::iterator start);

        std::string CORRADE_UTILITY_LOCAL keyName(const Entry& entry) const;

        std::string valueInternal(const std::string& key) const;

        std::string _command;
        std::string _help;
        std::vector<Entry> _entries;
        std::vector<std::string> _values;
        std::vector<bool> _booleans;
};

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> inline std::string Arguments::value(const std::string& key) const {
    return valueInternal(key);
}
#endif

template<class T> T Arguments::value(const std::string& key) const {
    std::string value = valueInternal(key);
    return value.empty() ? T() : ConfigurationValue<T>::fromString(value, {});
}

}}

#endif
