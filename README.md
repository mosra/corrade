*corrade* (v.) - "To scrape together, to gather together from various sources"

Corrade is multiplatform plugin management and utility library, written in pure
C++ with no external dependencies. Features:

 * Plugin management library with dependency handling
 * INI-style configuration files parser and writer
 * Resource manager, filesystem utilites and more

INSTALLATION
============

You can either use packaging scripts, which are stored in package/ subdirectory,
or compile and install everything manually.

Dependencies
------------

 * CMake    - for building
 * Qt       - optionally, for unit tests

Compilation, installation
-------------------------

    mkdir -p build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=/usr .. && make
    make install

If you want to build also unit tests (which are not built by default),
pass -DBUILD_TESTS=True to CMake. Unit tests use QtTest framework.

CONTACT
=======

Want to learn more about the library? Found a bug or want to tell me an
awesome idea? Feel free to visit my website or contact me at:

 * Website - http://mosra.cz/blog/
 * GitHub - http://github.com/mosra
 * E-mail - mosra@centrum.cz
 * Jabber - mosra@jabbim.cz
