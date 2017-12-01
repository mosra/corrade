*corrade* (v.) -- "To scrape together, to gather together from various sources"

Corrade is a multiplatform utility library written in C++11/C++14. It's used as
a base for the [Magnum graphics engine](http://magnum.graphics/) among other
things.

[![Join the chat at https://gitter.im/mosra/magnum](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/mosra/magnum?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

SUPPORTED PLATFORMS
===================

*   **Linux** and embedded Linux [![Build Status](https://travis-ci.org/mosra/corrade.svg?branch=master)](https://travis-ci.org/mosra/corrade) [![Coverage Status](https://coveralls.io/repos/github/mosra/corrade/badge.svg?branch=master)](https://coveralls.io/github/mosra/corrade?branch=master)
*   **Windows** [![Build Status](https://ci.appveyor.com/api/projects/status/afjjlsgtk6jjxulp/branch/master?svg=true)](https://ci.appveyor.com/project/mosra/corrade/branch/master)
*   **macOS** [![Build Status](https://travis-ci.org/mosra/corrade.svg?branch=master)](https://travis-ci.org/mosra/corrade)
*   **iOS** [![Build Status](https://travis-ci.org/mosra/corrade.svg?branch=master)](https://travis-ci.org/mosra/corrade)
*   **Android**
*   **Windows RT** (Store/Phone) [![Build Status](https://ci.appveyor.com/api/projects/status/afjjlsgtk6jjxulp/branch/master?svg=true)](https://ci.appveyor.com/project/mosra/corrade/branch/master)
*   **Web** (asm.js or WebAssembly), through [Emscripten](http://kripken.github.io/emscripten-site/) [![Build Status](https://travis-ci.org/mosra/corrade.svg?branch=master)](https://travis-ci.org/mosra/corrade)

FEATURES
========

*   Actively maintained Doxygen documentation with tutorials and examples.
    Snapshot is available at http://doc.magnum.graphics/corrade/.
*   Signal/slot connection library with compile-time argument checking.
*   Plugin management library with static and dynamic plugins and dependency
    handling.
*   Easy-to-use unit test framework and extensible debugging output.
*   Various utilities to simplify multiplatform development.

INSTALLATION
============

You can either use packaging scripts, which are stored in the `package/`
subdirectory, or compile and install everything manually using the guide below.
Note that the [Corrade documentation](http://doc.magnum.graphics/corrade/)
contains more comprehensive guide for building, packaging and crosscompiling.

Minimal dependencies
--------------------

*   C++ compiler with good C++11 support. Compilers which are tested to have
    everything needed are **GCC** >= 4.7, **Clang** >= 3.1 and **MSVC** >= 2015.
    For Windows you can also use **MinGW-w64**.
*   **CMake** >= 2.8.12

Note that full feature set is available only on GCC >= 4.8.1 and Clang >= 3.1
and compatibility mode with reduced feature set must be enabled for other
compilers.

Compilation, installation
-------------------------

The library can be built and installed using these four commands:

```sh
mkdir -p build
cd build
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

The documentation is written in **Doxygen**. It can be build by running

```sh
doxygen
```

in the root directory (i.e. where `Doxyfile` is). Resulting HTML documentation
will be in the `build/doc/` directory. Snapshot of the documentation is
[also available for online viewing](http://doc.magnum.graphics/corrade/).

Building examples
-----------------

The library comes with a handful of examples, contained in the `src/examples/`
directory. Each example is thoroughly explained in the documentation. The
examples require Corrade to be installed and they are built separately:

```sh
mkdir -p build-examples
cd build-examples
cmake ../src/examples
make -j
```

CONTACT
=======

Want to learn more about the library? Found a bug or want to share an awesome
idea? Feel free to visit the project website or contact the team at:

*   Website -- http://magnum.graphics/corrade/
*   GitHub -- http://github.com/mosra/corrade
*   Gitter -- https://gitter.im/mosra/magnum
*   IRC -- join `#magnum-engine` channel on freenode
*   Google Groups -- https://groups.google.com/forum/#!forum/magnum-engine
*   Twitter -- https://twitter.com/czmosra
*   E-mail -- mosra@centrum.cz
*   Jabber -- mosra@jabbim.cz

CREDITS
=======

See the [CREDITS.md](CREDITS.md) file for details. Big thanks to everyone
involved!

LICENSE
=======

Corrade is licensed under the MIT/Expat license, see the [COPYING](COPYING)
file for details.
