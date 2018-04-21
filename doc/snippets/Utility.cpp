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

#include <cstdlib>
#include <Corrade/Utility/Configuration.h>
#include <Corrade/Utility/Macros.h>

using namespace Corrade;

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
