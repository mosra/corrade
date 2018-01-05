> *corrade* (v.) — “To scrape together, to gather together from various sources”

Corrade is a multiplatform utility library written in C++11/C++14. It's used as
a base for the [Magnum graphics engine](http://magnum.graphics/), among other
things.

[![Join the chat at https://gitter.im/mosra/magnum](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/mosra/magnum?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Build Status](https://travis-ci.org/mosra/corrade.svg?branch=master)](https://travis-ci.org/mosra/corrade)
[![Build Status](https://ci.appveyor.com/api/projects/status/afjjlsgtk6jjxulp/branch/master?svg=true)](https://ci.appveyor.com/project/mosra/corrade/branch/master)
[![Coverage Status](https://coveralls.io/repos/github/mosra/corrade/badge.svg?branch=master)](https://coveralls.io/github/mosra/corrade?branch=master)
[![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

-   Project homepage — http://magnum.graphics/corrade/
-   Documentation — http://doc.magnum.graphics/corrade/
-   GitHub project page — https://github.com/mosra/corrade

SUPPORTED PLATFORMS
===================

-   **Linux** and embedded Linux
-   **Windows**, **Windows RT** (Store/Phone)
-   **macOS**, **iOS**
-   **Android**
-   **Web** ([asm.js](http://asmjs.org/) or [WebAssembly](http://webassembly.org/)),
    through [Emscripten](http://kripken.github.io/emscripten-site/)

See the Magnum Project [Build Status page](http://magnum.graphics/build-status/)
for detailed per-platform build status.

FEATURES
========

-   Low-level utilities to bridge platform differences when accessing OS
    functionality, filesystem, console and environment
-   Lightweight container implementations, complementing STL features with
    focus on compilation speed, ease of use and performance
-   Test framework emphasizing flexibility, extensibility, minimal use of
    macros and clarity of diagnostic output
-   Plugin management library with static and dynamic plugins, dependency
    handling and hot code reload
-   Signal/slot connection library with full type safety

Check also the Magnum Project [Feature Overview pages](http://magnum.graphics/features/)
for further information.

BUILDING CORRADE
================

You can either use packaging scripts, which are stored in the
[package/](https://github.com/mosra/corrade/tree/master/package)
subdirectory, or compile and install everything manually. A short guide is
below, for complete building documentation covering all platforms head over to
the [Corrade documentation](http://doc.magnum.graphics/corrade/building-corrade.html).

Minimal dependencies
--------------------

-   C++ compiler with good C++11 support. Compilers which are tested to have
    everything needed are **GCC** >= 4.7, **Clang** >= 3.1 and **MSVC** >= 2015.
    For Windows you can also use **MinGW-w64**.
-   **CMake** >= 2.8.12

Compilation, installation
-------------------------

The library can be built and installed using these commands:

```sh
git clone git://github.com/mosra/corrade && cd corrade
mkdir -p build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make -j
make install # sudo may be required
```

Building and running unit tests
-------------------------------

If you want to build also unit tests (which are not built by default), pass
`-DBUILD_TESTS=ON` to CMake. Unit tests use Corrade's own TestSuite framework
and can be run using

```sh
ctest --output-on-failure
````

in build directory. Everything should pass ;)

Building documentation
----------------------

The documentation is written using [Doxygen](https://doxygen.org). It can be
built by running

```sh
doxygen
```

in the root directory (i.e. where `Doxyfile` is). Resulting HTML documentation
will be in the `build/doc/` directory. You may need to create the `build/`
directory if it doesn't exist yet. Snapshot of the documentation is
[also available for online viewing](http://doc.magnum.graphics/corrade/).

GETTING STARTED
===============

The best way to get started is to read some [examples and tutorials](http://doc.magnum.graphics/corrade/corrade-example-index.html). Sources for them are contained in the
[src/examples](https://github.com/mosra/corrade/tree/master/src/examples)
directory and are built separately, requiring Corrade to be installed:

```sh
mkdir -p build-examples
cd build-examples
cmake ../src/examples
make -j
```

CONTACT & SUPPORT
=================

If you want to contribute to Corrade, if you spotted a bug, need a feature or
have an awesome idea, you can get a copy of the sources from GitHub and start
right away! There is the already mentioned guide about
[how to download and build Corrade](http://doc.magnum.graphics/corrade/building-corrade.html)
and also a guide about [coding style and best practices](http://doc.magnum.graphics/corrade/corrade-coding-style.html)
which you should follow to keep the library as consistent and maintainable as
possible.

-   Project homepage — http://magnum.graphics/corrade/
-   Documentation — http://doc.magnum.graphics/corrade/
-   GitHub project page — https://github.com/mosra/corrade
-   Gitter community chat — https://gitter.im/mosra/magnum
-   IRC — join the `#magnum-engine` channel on freenode
-   Google Groups mailing list — magnum-engine@googlegroups.com ([archive](https://groups.google.com/forum/#!forum/magnum-engine))
-   Author's personal Twitter — https://twitter.com/czmosra
-   Author's personal e-mail — mosra@centrum.cz

See also the Magnum Project [Contact & Support page](http://magnum.graphics/contact/)
for further information.

CREDITS
=======

See the [CREDITS.md](CREDITS.md) file for details. Big thanks to everyone
involved!

LICENSE
=======

Corrade itself and its documentation is licensed under the MIT/Expat license,
see the [COPYING](COPYING) file for details. All example code in `src/examples`
is put into public domain (or UNLICENSE) to free you from any legal obstacles
when reusing the code in your apps. See the [COPYING-examples](COPYING-examples)
file for details.
