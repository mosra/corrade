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

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "Corrade/Containers/Array.h"
#include "Corrade/Utility/Arguments.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/Configuration.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/Endianness.h"
#if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)) || defined(CORRADE_TARGET_EMSCRIPTEN)
#include "Corrade/Utility/FileWatcher.h"
#endif
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/FormatStl.h"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Sha1.h"

/* [Tweakable-disable-header] */
#define CORRADE_TWEAKABLE
#include "Corrade/Utility/Tweakable.h"
/* [Tweakable-disable-header] */

/* [ConfigurationValue] */
#include <Corrade/Utility/ConfigurationGroup.h>

struct Foo {
    int a, b;
};

namespace Corrade { namespace Utility {

template<> struct ConfigurationValue<Foo> {
    static std::string toString(const Foo& value, ConfigurationValueFlags flags) {
        return
            ConfigurationValue<int>::toString(value.a, flags) + ' ' +
            ConfigurationValue<int>::toString(value.b, flags);
    }

    static Foo fromString(const std::string& stringValue, ConfigurationValueFlags flags) {
        std::istringstream i{stringValue};
        std::string a, b;

        Foo foo;
        (i >> a) && (foo.a = ConfigurationValue<int>::fromString(a, flags));
        (i >> b) && (foo.b = ConfigurationValue<int>::fromString(b, flags));
        return foo;
    }
};

}}
/* [ConfigurationValue] */

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
#ifdef CORRADE_NO_ASSERT
static_cast<void>(pos);
#endif
return {};
}

bool initialize(char = 0);
void consume(Containers::Array<char>&&);
void foo(char userParam) {
/* [CORRADE_ASSERT-output] */
CORRADE_ASSERT(initialize(userParam),
    "Initialization failed: wrong parameter" << userParam, ); // wrong!
/* [CORRADE_ASSERT-output] */

/* [CORRADE_ASSERT_OUTPUT] */
CORRADE_ASSERT_OUTPUT(initialize(userParam),
    "Initialization failed: wrong parameter" << userParam, );
/* [CORRADE_ASSERT_OUTPUT] */

/* [CORRADE_INTERNAL_ASSERT-output] */
CORRADE_INTERNAL_ASSERT(initialize()); // wrong!
/* [CORRADE_INTERNAL_ASSERT-output] */

/* [CORRADE_INTERNAL_ASSERT_OUTPUT] */
CORRADE_INTERNAL_ASSERT_OUTPUT(initialize());
/* [CORRADE_INTERNAL_ASSERT_OUTPUT] */

{
/* [CORRADE_INTERNAL_ASSERT_EXPRESSION-without] */
Containers::Array<char> data;
CORRADE_INTERNAL_ASSERT_OUTPUT(data = Utility::Directory::read("file.dat"));
consume(std::move(data));
/* [CORRADE_INTERNAL_ASSERT_EXPRESSION-without] */
}

{
/* [CORRADE_INTERNAL_ASSERT_EXPRESSION] */
consume(CORRADE_INTERNAL_ASSERT_EXPRESSION(Utility::Directory::read("file.dat")));
/* [CORRADE_INTERNAL_ASSERT_EXPRESSION] */
}

{
int *src{}, *dst{}, *end{};
/* [CORRADE_ASSUME] */
CORRADE_ASSUME(src != dst);
for(; src != end; ++src, ++dst) *dst += *src;
/* [CORRADE_ASSUME] */
}
}

#ifdef CORRADE_TARGET_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif
enum class Status { Great, NotGreat };
/* [CORRADE_ASSERT-unreachable] */
std::string statusString(Status status) {
    switch(status) {
        case Status::Great: return "great";
        case Status::NotGreat: return "not great";
    }
    CORRADE_ASSERT(false, "status is neither great nor non-great", {}); // wrong!
}
/* [CORRADE_ASSERT-unreachable] */

enum class Type { UnsignedInt, UnsignedShort, UnsignedByte };
/* [CORRADE_INTERNAL_ASSERT-unreachable] */
std::size_t elementCount(std::size_t size, Type type) {
    switch(type) {
        case Type::UnsignedInt: return size/4;
        case Type::UnsignedShort: return size/2;
        case Type::UnsignedByte: return size/1;
    }

    CORRADE_INTERNAL_ASSERT(false); // wrong!
}
/* [CORRADE_INTERNAL_ASSERT-unreachable] */
#ifdef CORRADE_TARGET_GCC
#pragma GCC diagnostic push
#endif
};

struct Vec2 {
enum class Status { Great, NotGreat };
/* [CORRADE_ASSERT_UNREACHABLE] */
std::string statusString(Status status) {
    switch(status) {
        case Status::Great: return "great";
        case Status::NotGreat: return "not great";
    }
    CORRADE_ASSERT_UNREACHABLE("status is neither great nor non-great", {});
}
/* [CORRADE_ASSERT_UNREACHABLE] */

enum class Type { UnsignedInt, UnsignedShort, UnsignedByte };
/* [CORRADE_INTERNAL_ASSERT_UNREACHABLE] */
std::size_t elementCount(std::size_t size, Type type) {
    switch(type) {
        case Type::UnsignedInt: return size/4;
        case Type::UnsignedShort: return size/2;
        case Type::UnsignedByte: return size/1;
    }
    CORRADE_INTERNAL_ASSERT_UNREACHABLE();
}
/* [CORRADE_INTERNAL_ASSERT_UNREACHABLE] */
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

/* [CORRADE_HAS_TYPE-type] */
CORRADE_HAS_TYPE(HasKeyType, typename T::key_type);

static_assert(HasKeyType<std::map<int, int>>::value, "");
static_assert(!HasKeyType<std::vector<int>>::value, "");
/* [CORRADE_HAS_TYPE-type] */

/* [CORRADE_HAS_TYPE-function] */
CORRADE_HAS_TYPE(HasSize, decltype(std::declval<T>().size()));

static_assert(HasSize<std::vector<int>>::value, "");
static_assert(!HasSize<std::tuple<int, int>>::value, "");
/* [CORRADE_HAS_TYPE-function] */

class Buzz {
/* [Arguments-usage] */
int main(int argc, char** argv) {
    Utility::Arguments args;
    args.addArgument("text").setHelp("text", "the text to print")
        .addNamedArgument('n', "repeat").setHelp("repeat", "repeat count")
        .addBooleanOption('v', "verbose").setHelp("verbose", "log verbosely")
        .addOption("log", "log.txt").setHelp("log", "save verbose log to given file")
        .setGlobalHelp("Repeats the text given number of times.")
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
{
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
    .setGlobalHelp("Repeats the text given number of times.")
    .parse(argc, argv);
/* [Arguments-delegating] */
}

{
/* [Arguments-delegating-bool] */
Utility::Arguments args{"formatter"};
args.addOption("unicode", "false");

// ...

bool handleUnicode = args.value<bool>("unicode");
/* [Arguments-delegating-bool] */
static_cast<void>(handleUnicode);
}

{
/* [Arguments-delegating-ignore-unknown] */
/* The first instance handles all arguments */
Utility::Arguments args{"formatter"};
args.addOption("width", "80").setHelp("width", "number of columns")
    .addOption("color", "auto").setHelp("color", "output color")
    .addOption("log", "default").setHelp("log", "default|verbose|quiet")
    .parse(argc, argv);

{
    /* A subsystem cares only about the log option, ignoring the rest. It also
       doesn't need to provide help because that gets handled above already. */
    Utility::Arguments arg1{"formatter",
        Utility::Arguments::Flag::IgnoreUnknownOptions};
    arg1.addOption("log", "default")
        .parse(argc, argv);
}
/* [Arguments-delegating-ignore-unknown] */
}

{
/* [Arguments-parse-error-callback] */
Utility::Arguments args;
args.addOption("input")
    .addOption("output")
    .addBooleanOption("info")
        .setHelp("info", "print info about the input file and exit")
    .setParseErrorCallback([](const Utility::Arguments& args,
                              Utility::Arguments::ParseError error,
                              const std::string& key) {
        /* If --info is passed, we don't need the output argument */
        if(error == Utility::Arguments::ParseError::MissingArgument &&
           key == "output" &&
           args.isSet("info")) return true;

        /* Handle all other errors as usual */
        return false;
    })
    .parse(argc, argv);
/* [Arguments-parse-error-callback] */
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
int pwd{};
bool bar{};
/* [Debug-usage] */
// Common usage
Utility::Debug{} << "string" << 34 << 275.0f;

// Redirect debug output to string
std::ostringstream o;
Utility::Debug{&o} << "the meaning of life, universe and everything is" << 42;

// Mute debug output
Utility::Debug{nullptr} << "no one should see my ebanking password" << pwd;

// Conditional debug output (avoid inserting newline where it's not desired)
Utility::Debug d;
d << "Cannot foo";
if(bar)
    d << "because of bar.";
else
    d << "because of everything else.";
// (newline character will be written to output on object destruction)
/* [Debug-usage] */
}

{
/* [Debug-scoped-output] */
std::ostringstream debugOut, errorOut;

Utility::Error{} << "this is printed into std::cerr";

Utility::Error redirectError{&errorOut};

{
    Utility::Debug redirectDebug{&debugOut};

    Utility::Debug{} << "this is printed into debugOut";
    Utility::Error{} << "this is printed into errorOut";
    Utility::Debug{} << "this is also printed into debugOut";
}

Utility::Debug{} << "this is printed into std::cout again";
Utility::Error{} << "this is still printed into errorOut";
/* [Debug-scoped-output] */
}

{
/* [Debug-modifiers-whitespace] */
// Prints "Value: 16, 24"
Utility::Debug{} << "Value:" << 16 << Utility::Debug::nospace << "," << 24;

// Prints "Value\n16"
Utility::Debug{} << "Value:" << Utility::Debug::newline << 16;

// Doesn't output newline at the end
Utility::Debug{Utility::Debug::Flag::NoNewlineAtTheEnd} << "Hello!";
/* [Debug-modifiers-whitespace] */
}

{
/* [Debug-modifiers-colors] */
Utility::Debug{}
    << Utility::Debug::boldColor(Utility::Debug::Color::Green) << "Success!"
    << Utility::Debug::resetColor << "Everything is fine.";
/* [Debug-modifiers-colors] */
}

{
/* [Debug-modifiers-colors-disable] */
Utility::Debug::Flags flags = Utility::Debug::isTty() ?
    Utility::Debug::Flags{} : Utility::Debug::Flag::DisableColors;
Utility::Debug{flags}
    << Utility::Debug::boldColor(Utility::Debug::Color::Green) << "Success!";
/* [Debug-modifiers-colors-disable] */
}

{
bool errorHappened{};
/* [Debug-modifiers-colors-scoped] */
Utility::Debug{} << "this has default color";

{
    Utility::Debug d;
    if(errorHappened) d << Utility::Debug::color(Utility::Debug::Color::Red);

    Utility::Debug{} << "if an error happened, this will be printed red";
    Utility::Debug{} << "this also"
        << Utility::Debug::boldColor(Utility::Debug::Color::Blue)
        << "and this blue";
}

Utility::Debug{} << "this has default color again";
/* [Debug-modifiers-colors-scoped] */
}

{
/* [Debug-source-location] */
float a = 336;

!Utility::Debug{} << "the result is" << (a /= 8);
!Utility::Debug{} << "but here it's" << (a /= 8);

!Utility::Debug{};

Utility::Debug{} << "and finally, " << (a *= 8);
/* [Debug-source-location] */
}

{
/* [Debug-nospace] */
Utility::Debug{} << "Value:" << 16 << Utility::Debug::nospace << "," << 24;
/* [Debug-nospace] */
}

{
/* [Debug-newline] */
Utility::Debug{} << "Value:" << Utility::Debug::newline << 16;
Utility::Debug{} << "Value:" << Utility::Debug::nospace << "\n"
    << Utility::Debug::nospace << 16;
/* [Debug-newline] */
}

{
/* [Debug-space] */
Utility::Debug{} << "Value:";

Utility::Debug{} << "" << 16;
Utility::Debug{} << Utility::Debug::space << 16;
/* [Debug-space] */
}

{
/* [Debug-color] */
unsigned char data[] { 0, 32, 64, 96, 128, 160, 192, 224, 255 };
Utility::Debug{} << "41 shades of grey missing:"
    << Utility::Debug::packed << Utility::Debug::color
    << Containers::arrayView(data);
/* [Debug-color] */
}

{
struct {
    bool broken() { return true; }
} stuff;
/* [Fatal-Error] */
if(stuff.broken()) {
    Utility::Error{} << "Everything's broken, exiting.";
    std::exit(42);
}
/* [Fatal-Error] */

/* [Fatal-Fatal] */
if(stuff.broken())
    Utility::Fatal{42} << "Everything's broken, exiting.";
/* [Fatal-Fatal] */
}

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
{
std::string from, to;
/* [Directory-copy-mmap] */
Utility::Directory::write(to, Utility::Directory::mapRead(from));
/* [Directory-copy-mmap] */
}
#endif

/* FFS, GCC. According to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53431,
   reported back in 2012, for C++ GCC does lexing before parsing #pragmas and
   thus the -Wmultichar warning *can't* be ignored with a pragma. Because I
   don't want to compile the whole file with -Wno-multichar, I'm disabling this
   snippet for GCC. Note to self: check this again in 2025. */
#if !defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_CLANG)
{
#ifdef CORRADE_TARGET_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfour-char-constants"
#elif defined(CORRADE_TARGET_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultichar"
#endif
/* [Endianness-fourCC] */
std::uint32_t a = 'WAVE';
std::uint32_t b = Utility::Endianness::fourCC('W', 'A', 'V', 'E');
/* [Endianness-fourCC] */
static_cast<void>(a);
static_cast<void>(b);
#if defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_CLANG)
#pragma GCC diagnostic pop
#endif
}
#endif

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
std::string s = Utility::formatString("path {{ fill: #{:.6x}; stroke: #{:.6x}; }}",
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

#if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)) || defined(CORRADE_TARGET_EMSCRIPTEN)
{
/* [FileWatcher] */
Utility::FileWatcher watcher{"settings.conf"};

// in the main application loop
if(watcher.hasChanged()) {
    // reload the settings
}
/* [FileWatcher] */
}
#endif

{
int a = 2;
int d[5]{};
int e[5]{};
int *b = d, *c = e;
/* [CORRADE_FALLTHROUGH] */
switch(a) {
    case 2:
        *b++ = *c++;
        CORRADE_FALLTHROUGH
    case 1:
        *b++ = *c++;
};
/* [CORRADE_FALLTHROUGH] */
}

{
/* [CORRADE_LINE_STRING] */
const char* shader = "#line " CORRADE_LINE_STRING "\n" R"GLSL(
    in vec3 position;

    void main() {
        THIS_IS_AN_ERROR();
    }
)GLSL";
/* [CORRADE_LINE_STRING] */
static_cast<void>(shader);
}

{
/* [Tweakable-define] */
#define _ CORRADE_TWEAKABLE
/* [Tweakable-define] */
}

{
struct App {
float dt{}, fallVelocity{};
struct {
    float x{}, y{};
} position;
/* [Tweakable-wrap-update] */
Utility::Tweakable tweakable;

void init() {
    tweakable.enable();

    // …
}

void mainLoop() {
    fallVelocity += _(9.81f)*dt;
    position.x += _(2.2f)*dt;
    position.y += fallVelocity*dt;

    // …

    tweakable.update();
}
/* [Tweakable-wrap-update] */
};
}

{
struct App {
/* [Tweakable-scope] */
explicit App() {
    // …

    tweakable.scope([](App& app) {
        app.dt = _(0.01666667f); // 60 FPS
        app.fallVelocity = _(0.0f);
        app.position = {_(5.0f), _(150.0f)};
    }, *this);
}

void mainLoop() {
    fallVelocity += _(9.81f)*dt;
    // …
/* [Tweakable-scope] */
}

#undef _
/* [Tweakable-disable] */
#define _
/* [Tweakable-disable] */

Utility::Tweakable tweakable;
float dt, fallVelocity;
struct {
    float x, y;
} position;
};
}

{
/* [Sha1-usage] */
Utility::Sha1 sha1;

/* Add 7 bytes of string data */
sha1 << std::string{"corrade"};

/* Add four bytes of binary data */
const char data[4] = { '\x35', '\xf6', '\x00', '\xab' };
sha1 << Containers::arrayView(data);

/* Print the digest as a hex string */
Utility::Debug{} << sha1.digest().hexString();

/* Shorthand variant, treating the argument as a string */
Utility::Debug{} << Utility::Sha1::digest("corrade");
/* [Sha1-usage] */
}
}

typedef std::pair<int, int> T;
/* [TweakableParser] */
namespace Corrade { namespace Utility { // namespace is important

template<> struct TweakableParser<T> {
    static std::pair<TweakableState, T> parse(Containers::ArrayView<const char> value);
};

}}
/* [TweakableParser] */

namespace A {

struct Fizz;
/* [CORRADE_DEPRECATED] */
class CORRADE_DEPRECATED("use Bar instead") Foo;
CORRADE_DEPRECATED("use bar() instead") void foo();
typedef CORRADE_DEPRECATED("use Fizz instead") Fizz Buzz;
CORRADE_DEPRECATED("use Value instead") constexpr int Vauel = 3;
/* [CORRADE_DEPRECATED] */
CORRADE_IGNORE_DEPRECATED_PUSH
inline void soVauelIsNotUnused() { static_cast<void>(Vauel); } /* eugh */
CORRADE_IGNORE_DEPRECATED_POP

}

namespace B {

template<class> class Bar;
/* [CORRADE_DEPRECATED_ALIAS] */
template<class T> using Foo CORRADE_DEPRECATED_ALIAS("use Bar instead") = Bar<T>;
/* [CORRADE_DEPRECATED_ALIAS] */

}

namespace Another {
namespace Bar {}
/* [CORRADE_DEPRECATED_NAMESPACE] */
namespace CORRADE_DEPRECATED_NAMESPACE("use Bar instead") Foo {
    using namespace Bar;
}
/* [CORRADE_DEPRECATED_NAMESPACE] */
}

namespace C {

/* [CORRADE_DEPRECATED_ENUM] */
enum class CORRADE_DEPRECATED_ENUM("use Bar instead") Foo {};

enum class Bar {
    Fizz = 0,
    Buzz = 1,
    Baz CORRADE_DEPRECATED_ENUM("use Bar::Buzz instead") = 1
};
/* [CORRADE_DEPRECATED_ENUM] */

int foo(int, int);
/* [CORRADE_UNUSED] */
int foo(int a, CORRADE_UNUSED int b) {
    return a;
}
/* [CORRADE_UNUSED] */

/* [CORRADE_ALIGNAS] */
CORRADE_ALIGNAS(4) char data[16]; // so it can be read as 32-bit integers
/* [CORRADE_ALIGNAS] */

CORRADE_NORETURN void exit42();
/* [CORRADE_NORETURN] */
CORRADE_NORETURN void exit42() { std::exit(42); }
/* [CORRADE_NORETURN] */

/* [CORRADE_ALWAYS_INLINE] */
CORRADE_ALWAYS_INLINE int addOne(int a);
/* [CORRADE_ALWAYS_INLINE] */

int counter = 0;
/* [CORRADE_NEVER_INLINE] */
CORRADE_NEVER_INLINE void testFunctionCallOverhead();
/* [CORRADE_NEVER_INLINE] */

/* [CORRADE_VISIBILITY_EXPORT] */
void privateFunction(); /* can't be used outside of the shared library */

CORRADE_VISIBILITY_EXPORT void exportedFunction();

class CORRADE_VISIBILITY_EXPORT ExportedClass {
    public:
        void foo(); /* Non-inline members get implicitly exported as well */

    private:
        /* Used only privately, thus doesn't need to be exported */
        CORRADE_VISIBILITY_LOCAL void privateFoo();
};
/* [CORRADE_VISIBILITY_EXPORT] */
}

namespace D {
/* [CORRADE_VISIBILITY_EXPORT-dllexport] */
#ifdef MyLibrary_EXPORTS
    #define MYLIBRARY_EXPORT CORRADE_VISIBILITY_EXPORT
#else
    #define MYLIBRARY_EXPORT CORRADE_VISIBILITY_IMPORT
#endif

class MYLIBRARY_EXPORT ExportedClass {
    public:
        // …
};
/* [CORRADE_VISIBILITY_EXPORT-dllexport] */
}

namespace E {
extern int stuff;
/* [CORRADE_VISIBILITY_INLINE_MEMBER_EXPORT] */
class MYLIBRARY_EXPORT ExportedClass {
    public:
        // …

        CORRADE_VISIBILITY_INLINE_MEMBER_EXPORT int inlineFoo() {
            return stuff + 3;
        }
};
/* [CORRADE_VISIBILITY_INLINE_MEMBER_EXPORT] */
}
