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

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <Corrade/Utility/Arguments.h>
#include <Corrade/Utility/Assert.h>
#include <Corrade/Utility/Configuration.h>
#include <Corrade/Utility/Format.h>
#include <Corrade/Utility/Macros.h>

using namespace Corrade;

class Vec {
typedef char T;
std::size_t size() const;
T data[1];
/* [CORRADE_ASSERT] */
T operator[](std::size_t pos) const {
    CORRADE_ASSERT(pos < size(), "Array::operator[](): index out of range", {});
    return data[pos];
}
/* [CORRADE_ASSERT] */

std::vector<T> sources;
/* [CORRADE_ASSERT-void] */
void compile() {
    CORRADE_ASSERT(!sources.empty(), "Shader::compile(): no sources added", );

    // ...
}
/* [CORRADE_ASSERT-void] */

T set(std::size_t pos) {
/* [CORRADE_ASSERT-stream] */
CORRADE_ASSERT(pos < size(), "Array::operator[](): accessing element"
    << pos << "in an array of size" << size(), {});
/* [CORRADE_ASSERT-stream] */

/* [CORRADE_INTERNAL_ASSERT] */
CORRADE_INTERNAL_ASSERT(pos < size());
/* [CORRADE_INTERNAL_ASSERT] */
return {};
}

bool initialize(char = 0);
void foo(char userParam) {
/* [CORRADE_ASSERT-output] */
CORRADE_ASSERT(initialize(userParam),
    "Initialization failed: wrong parameter" << userParam, );
/* [CORRADE_ASSERT-output] */

/* [CORRADE_ASSERT_OUTPUT] */
CORRADE_ASSERT_OUTPUT(initialize(userParam),
    "Initialization failed: wrong parameter" << userParam, );
/* [CORRADE_ASSERT_OUTPUT] */

/* [CORRADE_INTERNAL_ASSERT-output] */
CORRADE_INTERNAL_ASSERT(initialize());
/* [CORRADE_INTERNAL_ASSERT-output] */

/* [CORRADE_INTERNAL_ASSERT_OUTPUT] */
CORRADE_INTERNAL_ASSERT_OUTPUT(initialize());
/* [CORRADE_INTERNAL_ASSERT_OUTPUT] */
}

enum class Flag { A, B };
bool foo(Flag flag) {
bool foo{}, bar{};
/* [CORRADE_ASSERT_UNREACHABLE] */
switch(flag) {
    case Flag::A: return foo;
    case Flag::B: return bar;
}

CORRADE_ASSERT_UNREACHABLE();
/* [CORRADE_ASSERT_UNREACHABLE] */
}
};

/* [CORRADE_CONSTEXPR_ASSERT] */
constexpr int divide(int a, int b) {
    return CORRADE_CONSTEXPR_ASSERT(b, "divide(): can't divide by zero"), a/b;
}
/* [CORRADE_CONSTEXPR_ASSERT] */

namespace Internal {
/* [CORRADE_INTERNAL_CONSTEXPR_ASSERT] */
constexpr int divide(int a, int b) {
    return CORRADE_INTERNAL_CONSTEXPR_ASSERT(b), a/b;
}
/* [CORRADE_INTERNAL_CONSTEXPR_ASSERT] */
}

class Buzz {
/* [Arguments-usage] */
int main(int argc, char** argv) {
    Utility::Arguments args;
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
/* [Arguments-usage] */

void another(int argc, char** argv) {
/* [Arguments-delegating] */
{
    /* The underlying library */
    Utility::Arguments args{"formatter"};
    args.addOption("width", "80").setHelp("width", "number of columns")
        .addOption("color", "auto").setHelp("color", "output color")
        .parse(argc, argv);
}

/* The application */
Utility::Arguments args;
args.addArgument("text").setHelp("text", "the text to print")
    .addNamedArgument('n', "repeat").setHelp("repeat", "repeat count")
    .addSkippedPrefix("formatter", "formatter options")
    .setHelp("Repeats the text given number of times.")
    .parse(argc, argv);
/* [Arguments-delegating] */

{
/* [Arguments-delegating-bool] */
Utility::Arguments args{"formatter"};
args.addOption("unicode", "false");

// ...

bool handleUnicode = args.value<bool>("unicode");
/* [Arguments-delegating-bool] */
static_cast<void>(handleUnicode);
}
}
};

int main() {
{
/* [Configuration-usage] */
Utility::Configuration conf{"my.conf"};

/* Set value of third occurence of the key from some deep group */
conf.addGroup("foo")->addGroup("bar")->setValue("myKey", "myValue");

/* Get a value back */
std::string value = conf.group("foo")->group("bar")->value("myKey");

/* Remove all groups named "bar" from root */
conf.removeAllGroups("bar");

/* Add three new integral values */
conf.addValue("a", 1);
conf.addValue("a", 2);
conf.addValue("a", 3);

conf.save();
/* [Configuration-usage] */
}

{
/* [CORRADE_IGNORE_DEPRECATED] */
CORRADE_DEPRECATED("use bar() instead") void foo(int);

CORRADE_IGNORE_DEPRECATED_PUSH
foo(42);
CORRADE_IGNORE_DEPRECATED_POP
/* [CORRADE_IGNORE_DEPRECATED] */
}

{
/* [formatString] */
std::string s = Utility::formatString("{} version {}.{}.{}, {} MB",
    "vulkan.hpp", 1, 1, 76, 1.79);
// vulkan.hpp version 1.1.76, 1.79 MB
/* [formatString] */
static_cast<void>(s);
}

{
/* [formatString-numbered] */
std::string s = Utility::formatString("<{0}><{1}>Good {}, {}!</{1}></{0}>",
    "p", "strong", "afternoon", "ma'am!");
// <p><strong>Good afternoon, ma'am!</strong></p>
/* [formatString-numbered] */
static_cast<void>(s);
}

{
/* [formatString-escape] */
std::string s = Utility::formatString("union {{ {} a; char data[{}]; }} caster;",
    "float", sizeof(float));
// union { float a; char data[4]; } caster;
/* [formatString-escape] */
static_cast<void>(s);
}

{
/* [formatString-type-precision] */
std::string s = Utility::formatString("path {{ fill: #{:6x}; stroke: #{:6x}; }}",
    0x33ff00, 0x00aa55);
// path { fill: #33ff00; stroke: #00aa55; }
/* [formatString-type-precision] */
}

{
void addShaderSource(Containers::ArrayView<char>);
/* [formatInto-buffer] */
char shaderVersion[128]; // large enough
std::size_t size = Utility::formatInto(shaderVersion, "#version {}\n", 430);
addShaderSource({shaderVersion, size});
/* [formatInto-buffer] */
}

{
/* [formatInto-string] */
std::vector<float> positions{-0.5f, -0.5f, 0.0f,
                              0.5f, -0.5f, 0.0f,
                              0.0f,  0.5f, 0.0f};
std::string out;
for(std::size_t i = 0; i < positions.size(); i += 3)
    Utility::formatInto(out, out.size(), "{}[{0}, {1}, {2}]",
        out.empty() ? "" : ", ",
        positions[i*3 + 0], positions[i*3 + 1], positions[i*3 + 2]);

// [-0.5, -0.5, 0], [0.5, -0.5, 0], [0.0, 0.5, 0.0]
/* [formatInto-string] */
}

{
/* [formatInto-stdout] */
Utility::formatInto(stdout, "Hello, {}!", "world");
/* [formatInto-stdout] */
}

}

namespace A {

template<int> class Output;
/* [CORRADE_DEPRECATED] */
class CORRADE_DEPRECATED("use Bar instead") Foo;
CORRADE_DEPRECATED("use bar() instead") void foo();
typedef CORRADE_DEPRECATED("use Fizz instead") Output<5> Buzz;
/* [CORRADE_DEPRECATED] */

}

namespace B {

template<class> class Bar;
/* [CORRADE_DEPRECATED_ALIAS] */
template<class T> using Foo CORRADE_DEPRECATED_ALIAS("use Bar instead") = Bar<T>;
/* [CORRADE_DEPRECATED_ALIAS] */

}

namespace Bar {}
/* [CORRADE_DEPRECATED_NAMESPACE] */
namespace CORRADE_DEPRECATED_NAMESPACE("use Bar instead") Foo {
    using namespace Bar;
}
/* [CORRADE_DEPRECATED_NAMESPACE] */

namespace C {

/* [CORRADE_DEPRECATED_ENUM] */
enum class CORRADE_DEPRECATED_ENUM("use Bar instead") Foo {};

enum class Bar {
    Fizz = 0,
    Buzz = 1,
    Baz CORRADE_DEPRECATED_ENUM("use Bar::Buzz instead") = 1
};
/* [CORRADE_DEPRECATED_ENUM] */

}

int foo(int, int);
/* [CORRADE_UNUSED] */
int foo(int a, CORRADE_UNUSED int b) {
    return a;
}
/* [CORRADE_UNUSED] */

/* [CORRADE_ALIGNAS] */
CORRADE_ALIGNAS(4) char data[16]; // so it can be read as 32-bit integers
/* [CORRADE_ALIGNAS] */

void exit42();
/* [CORRADE_NORETURN] */
CORRADE_NORETURN void exit42() { std::exit(42); }
/* [CORRADE_NORETURN] */
