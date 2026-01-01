/*
    This file is part of Corrade.

    Original authors — credit is appreciated but not required:

        2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
        2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026
            — Vladimír Vondruš <mosra@centrum.cz>

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or distribute
    this software, either in source code form or as a compiled binary, for any
    purpose, commercial or non-commercial, and by any means.

    In jurisdictions that recognize copyright laws, the author or authors of
    this software dedicate any and all copyright interest in the software to
    the public domain. We make this dedication for the benefit of the public
    at large and to the detriment of our heirs and successors. We intend this
    dedication to be an overt act of relinquishment in perpetuity of all
    present and future rights to this software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <Corrade/Containers/Optional.h>
#include <Corrade/Utility/Arguments.h>
#include <Corrade/Utility/Json.h>
#include <Corrade/Utility/JsonWriter.h>

using namespace Corrade;

int main(int argc, char** argv) {
    Utility::Arguments args;
    args.addArgument("file")
            .setHelp("file", "Input JSON file to format")
        .addFinalOptionalArgument("output")
            .setHelp("output", "JSON file to write to instead of standard output")
        .addOption("indent", "2")
            .setHelp("indent", "How many spaces to indent the output with")
        .addBooleanOption("compact")
            .setHelp("compact", "Don't wrap and indent the output")
        .setGlobalHelp("JSON formatter.")
        .parse(argc, argv);

    /* Just tokenize the file without parsing anything */
    const Containers::Optional<Utility::Json> json =
        Utility::Json::fromFile(args.value("file"));
    if(!json)
        /* Utility::Json prints a message already, no need to repeat */
        return 1;

    /* Set up the writer with desired formatting */
    Utility::JsonWriter::Options options;
    if(!args.isSet("compact"))
        options |= Utility::JsonWriter::Option::Wrap|
                   Utility::JsonWriter::Option::TypographicalSpace;
    Utility::JsonWriter writer{options, args.value<unsigned>("indent")};

    /* Pass it the whole input */
    writer.writeJson(json->root());

    /* Write to a file if specified or to standard output */
    if(const Containers::StringView output = args.value("output")) {
        if(!writer.toFile(output))
            /* Again Utility::JsonWriter prints a message already */
            return 1;
    } else Utility::Debug{} << writer.toString();
}
