*corrade* (v.) - "To scrape together, to gather together from various sources"

Corrade is multiplatform utility library written in C++11/C++14.

[![Join the chat at https://gitter.im/mosra/magnum](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/mosra/magnum?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

SUPPORTED PLATFORMS
===================

*   **Linux** and embedded Linux [![Build Status](https://travis-ci.org/mosra/corrade.svg?branch=master)](https://travis-ci.org/mosra/corrade) [![Coverage Status](https://coveralls.io/repos/github/mosra/corrade/badge.svg?branch=master)](https://coveralls.io/github/mosra/corrade?branch=master)
*   **Windows** [![Build Status](https://ci.appveyor.com/api/projects/status/afjjlsgtk6jjxulp/branch/master?svg=true)](https://ci.appveyor.com/project/mosra/corrade/branch/master)
*   **macOS** [![Build Status](https://travis-ci.org/mosra/corrade.svg?branch=master)](https://travis-ci.org/mosra/corrade)
*   **iOS** [![Build Status](https://travis-ci.org/mosra/corrade.svg?branch=master)](https://travis-ci.org/mosra/corrade)
*   **Android** 1.5 (API Level 3) and higher
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
*   Various utilities to ease multiplatform development.

INSTALLATION
============

You can either use packaging scripts, which are stored in `package/`
subdirectory, or compile and install everything manually. Note that Doxygen
documentation contains more comprehensive guide for building, packaging and
crosscompiling.

Minimal dependencies
--------------------

*   C++ compiler with good C++11 support. Compilers which are tested to have
    everything needed are **GCC** >= 4.7, **Clang** >= 3.1 and **MSVC** >= 2015.
    On Windows you can also use **MinGW-w64**.
*   **CMake** >= 2.8.12

Note that full feature set is available only on GCC >= 4.8.1 and Clang >= 3.1
and compatibility mode with reduced feature set must be enabled for other
compilers.

Compilation, installation
-------------------------

The library can be built and installed using these four commands:

    mkdir -p build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=/usr .. && make
    make install

Building and running unit tests
-------------------------------

If you want to build also unit tests (which are not built by default), pass
`-DBUILD_TESTS=ON` to CMake. Unit tests use Corrade's own TestSuite framework
and can be run using

    ctest --output-on-failure

in build directory. Everything should pass ;-)

Building documentation
----------------------

The documentation is written in **Doxygen** (preferrably 1.8 with Markdown
support, but older versions should do good job too) and additionally uses
**Graphviz** for class diagrams. The documentation can be build by running

    doxygen

in root directory (i.e. where `Doxyfile` is). Resulting HTML documentation
will be in `build/doc/` directory.

Building examples
-----------------

The library comes with handful of examples, contained in `src/examples/`
directory. Each example is thoroughly explained in documentation. The examples
require Corrade to be installed and they are built separately:

    mkdir -p build-examples
    cd build-examples
    cmake ../src/examples
    make

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

See [CREDITS.md](CREDITS.md) file for details. Big thanks to everyone involved!

LICENSE
=======

Corrade is licensed under MIT/Expat license, see [COPYING](COPYING) file for
details.
