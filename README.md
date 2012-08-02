*corrade* (v.) - "To scrape together, to gather together from various sources"

Corrade is multiplatform plugin management and utility library written in
C++11. Features:

 * Plugin management library with support for both static and dynamically
   loaded plugins, dependency handling and more
 * Unit test framework inspired with QtTest, but without any need to process
   files with some sort of meta compiler
 * Configuration parser and writer with syntax similar to INI files, with
   support for custom types and hierarchic groups
 * Resource manager for resources compiled directly into executable
 * Easy-to-use classes for debug and error output with support for output
   redirection and printing of custom types
 * Filesystem utilities, translation support, endianness handling class,
   implementations of hashing algorithms and more

INSTALLATION
============

You can either use packaging scripts, which are stored in package/
subdirectory, or compile and install everything manually. Note that Doxygen
documentation contains more comprehensive guide for building, packaging and
crosscompiling.

Minimal dependencies
--------------------

 * C++ compiler with good C++11 support. Currently there are two compilers
   which are tested to support everything needed: **GCC** >= 4.6 and **Clang**
   >= 3.1.
 * **CMake** >= 2.6

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
`-DBUILD_TESTS=True` to CMake. Unit tests use QtTest framework (you need at
least **Qt 4.6**) and can be run using

    ctest -V

in build directory. Everything should pass ;-)

Building documentation
----------------------

The documentation is written in **Doxygen** (preferrably 1.8 with Markdown
support, but older versions should do good job too) and additionally uses
**Graphviz** for class diagrams. The documentation can be build by running

    doxygen

in root directory (i.e. where `Doxyfile` is). Resulting HTML documentation
will be in `build/doc/` directory.

CONTACT
=======

Want to learn more about the library? Found a bug or want to tell me an
awesome idea? Feel free to visit my website or contact me at:

 * Website - http://mosra.cz/blog/corrade.php
 * GitHub - http://github.com/mosra/corrade
 * E-mail - mosra@centrum.cz
 * Jabber - mosra@jabbim.cz
