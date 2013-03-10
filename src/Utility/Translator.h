#ifndef Corrade_Utility_Translator_h
#define Corrade_Utility_Translator_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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
 * @brief Class Corrade::Utility::Translator
 */

#include <string>
#include <map>
#include <set>

#include "Configuration.h"

namespace Corrade { namespace Utility {

/**
@brief Translation

Allows application translation via configuration files. Features:
 - switching application language on-the-fly (without restart) centrally for
   all Translator instances
 - fallback language for not-yet-translated strings
 - translation strings provided via pointers

@section TranslatorUsage Simple usage
There are many possibilities for loading translations - they can be loaded
in constructor from file or from existing configuration group, or can be
loaded dynamically via setPrimary() / setFallback().
@code
# en_US.conf
welcome=Hello world!
exit=Exit application
about=View About dialog
@endcode
@code
# cs_CZ.conf
welcome=Ahoj světe!
exit=Ukončit aplikaci
@endcode
@code
// Initialize translator
Translator tr("en_US.conf");

// Get pointers to translation strings
string* welcome = tr.get("welcome");
string* exit = tr.get("exit");
string* about = tr.get("about");

std::cout << *welcome << endl;  // Hello world!
std::cout << *exit << endl;     // Exit application
std::cout << *about << endl;    // View About dialog

// Switch application language, add fallback for untranslated strings
tr.setPrimary("cs_CZ.conf");
tr.setFallback("en_US.conf");

std::cout << *welcome << endl;  // Ahoj světe!
std::cout << *exit << endl;     // Ukončit aplikaci
std::cout << *about << endl;    // View About dialog (untranslated,
                                // falls back to en_US.conf)
@endcode
@section TranslatorDynamicUsage Using dynamic languages
All Translator instances are globally registered, so they can be dynamically
updated after calling setLocale(). Using the same translation files as in
previous example, it can be done this way:
@code
// Initialize translator with dynamic filename. '#' character will be replaced
// with current locale name. Also setting fallback, if given localized file
// cannot be found
Translator tr("#.conf", "en_US.conf");

string* welcome = tr.get("welcome");

// Set locale to cs_CZ
Translator::setLocale("cs_CZ");

std::cout << *welcome << endl;  // Ahoj světe!
@endcode
@subsection TranslatorGroups Using configuration groups instead of files
Sometimes you have all translations in one configuration file and you want to
feed the translator with particular groups. Using dynamic languages it can
be done this way:
@code
# translations.conf
[en_US]
welcome=Hello world!
exit=Exit application
about=View About dialog

[cs_CZ]
welcome=Ahoj světe!
exit=Ukončit aplikaci
@endcode
@code
Configuration conf("translations.conf", Configuration::ReadOnly);
Translator tr;

// Set dynamic to true, so it will use subgroup named the same as current locale
tr.setPrimary(&conf, true);
tr.setFallback(conf->group("en_US"));

string* welcome = tr.get("welcome");

Translator::setLocale("cs_CZ");
std::cout << *welcome << endl;  // Ahoj světe!
@endcode
 */
class CORRADE_UTILITY_EXPORT Translator {
    public:
        /**
         * @brief Set current locale
         * @param locale        Locale name
         *
         * Sets locale. All dynamically set languages (see
         * setPrimary(const std::string&) and
         * setPrimary(const ConfigurationGroup*, bool) ) are updated with new
         * locale, non-dynamic languages are untouched.
         */
        static void setLocale(const std::string& locale);

        /**
         * @brief Get current locale
         * @return Current locale
         * @todo Set to system locale on initialization
         */
        inline static std::string locale() { return *_locale(); }

        /**
         * @brief Construct empty translator
         */
        inline explicit Translator(): primaryDynamicGroup(nullptr), primaryFile(nullptr), fallbackFile(nullptr), primary(nullptr), fallback(nullptr) {
            instances()->insert(this);
        }

        /**
         * @brief Construct from file
         * @param _primary      Primary language file. See also
         *      setPrimary(const std::string&).
         * @param _fallback     Fallback language file
         */
        inline explicit Translator(const std::string& _primary, const std::string& _fallback = std::string()): primaryDynamicGroup(nullptr), primaryFile(nullptr), fallbackFile(nullptr), primary(nullptr), fallback(nullptr) {
            setFallback(_fallback);
            setPrimary(_primary);

            instances()->insert(this);
        }

        /**
         * @brief Construct from existing configuration
         * @param _primary      Primary language configuration group
         * @param _fallback     Fallback language configuration group
         * @param dynamic       Whether treat primary group as dynamic. See
         *      setPrimary(const ConfigurationGroup*, bool).
         */
        inline explicit Translator(const ConfigurationGroup* _primary, const ConfigurationGroup* _fallback = nullptr, bool dynamic = false): primaryDynamicGroup(nullptr), primaryFile(nullptr), fallbackFile(nullptr), primary(nullptr), fallback(nullptr) {
            setFallback(_fallback);
            setPrimary(_primary, dynamic);

            instances()->insert(this);
        }

        /** @brief Destructor */
        ~Translator();

        /**
         * @brief Load primary translation from file
         * @param file          Configuration file. If the filename contains
         *      `#` character, it will be replaced with current locale name
         *      and the translation automatically reloaded after every
         *      setLocale() call.
         *
         * Loads primary translation from file. All translations previously
         * fetched with get() are updated with new translations.
         * @see setFallback(const std::string&), setPrimary(const ConfigurationGroup*, bool)
         */
        void setPrimary(const std::string& file);

        /**
         * @brief Load fallback translation from file
         * @param file          Configuration file
         *
         * Loads fallback translation from file.
         * @see setPrimary(const std::string&), setFallback(const ConfigurationGroup*)
         */
        void setFallback(const std::string& file);

        /**
         * @brief Load primary translation from existing configuration
         * @param group         Configuration group
         * @param dynamic       If set to true, primary language will be fetched
         *      from `group` subgroup with current locale name and
         *      automatically reloaded after every setLocale() call.
         *
         * Uses existing configuration group for primary translation. All
         * translations previously fetched with get() are updated with new
         * translations.
         * @attention Caller must ensure that the configuration groups aren't
         *      deleted during usage in Translator, also the Translator doesn't
         *      delete them on destruction, this is up to caller.
         * @see setFallback(const ConfigurationGroup*)
         */
        void setPrimary(const ConfigurationGroup* group, bool dynamic = false);

        /**
         * @brief Load fallback translation from existing configuration
         * @param group         Configuration group
         *
         * Uses existing configuration group for fallback translation.
         * @see setPrimary(const ConfigurationGroup*)
         */
        void setFallback(const ConfigurationGroup* group);

        /**
         * @brief Get localized string
         * @param key           Key
         */
        const std::string* get(const std::string& key);

    private:
        CORRADE_UTILITY_EXPORT static std::string* _locale();
        CORRADE_UTILITY_EXPORT static std::set<Translator*>* instances();

        std::string primaryDynamicFilename;
        const ConfigurationGroup* primaryDynamicGroup;

        Configuration *primaryFile,
            *fallbackFile;

        const ConfigurationGroup *primary,
            *fallback;

        std::map<std::string, std::string*> localizations;

        CORRADE_UTILITY_LOCAL bool get(const std::string& key, std::string* text, int level) const;

        CORRADE_UTILITY_LOCAL std::string replaceLocale(const std::string& filename) const;
};

}}

#endif
