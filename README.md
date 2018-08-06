> *corrade* (v.) — “To scrape together, to gather together from various sources”

Corrade is a multiplatform utility library written in C++11/C++14. It's used as
a base for the [Magnum graphics engine](http://magnum.graphics/), among other
things.

[![Join the chat at https://gitter.im/mosra/magnum](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/mosra/magnum?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Build Status](https://travis-ci.org/mosra/corrade.svg?branch=master)](https://travis-ci.org/mosra/corrade)
[![Build Status](https://ci.appveyor.com/api/projects/status/afjjlsgtk6jjxulp/branch/master?svg=true)](https://ci.appveyor.com/project/mosra/corrade/branch/master)
[![Coverage Status](https://codecov.io/gh/mosra/corrade/branch/master/graph/badge.svg)](https://codecov.io/gh/mosra/corrade)
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

WHAT'S NEW?
===========

Curious about what was added or improved recently? Check out the
[Changelog](http://doc.magnum.graphics/corrade/corrade-changelog.html#corrade-changelog-latest)
page in the documentation. Check also the Magnum Project
[Changelog](http://doc.magnum.graphics/magnum/changelog.html#changelog-latest).

GETTING STARTED
===============

Download, build and install Corrade as explained in
[the building documentation](http://doc.magnum.graphics/corrade/building-corrade.html)
— we provide packages for many platforms, including Windows, Linux and macOS.
After that, the best way to get started is to read some
[examples and tutorials](http://doc.magnum.graphics/corrade/corrade-example-index.html).

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
-   E-mail — info@magnum.graphics
-   IRC — join the `#magnum-engine` channel on freenode
-   Google Groups mailing list — magnum-engine@googlegroups.com ([archive](https://groups.google.com/forum/#!forum/magnum-engine))
-   Author's personal Twitter — https://twitter.com/czmosra

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
