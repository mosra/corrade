> *corrade* (v.) — “To scrape together, to gather together from various sources”

Corrade is a multiplatform utility library written in C++11. It's used as a
base for the [Magnum graphics engine](https://magnum.graphics/), among other
things.

[![Join the chat at https://gitter.im/mosra/magnum](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/mosra/magnum?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Build Status](https://circleci.com/gh/mosra/corrade.svg?style=shield)](https://circleci.com/gh/mosra/corrade)
[![Build Status](https://ci.appveyor.com/api/projects/status/afjjlsgtk6jjxulp/branch/master?svg=true)](https://ci.appveyor.com/project/mosra/corrade/branch/master)
[![Coverage Status](https://codecov.io/gh/mosra/corrade/branch/master/graph/badge.svg)](https://codecov.io/gh/mosra/corrade)
[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)

-   Project homepage — https://magnum.graphics/corrade/
-   Documentation — https://doc.magnum.graphics/corrade/
-   GitHub project page — https://github.com/mosra/corrade

SUPPORTED PLATFORMS
===================

-   **Linux** and embedded Linux
-   **Windows** with MSVC, clang-cl and MinGW, **Windows RT** (Store/Phone)
-   **macOS**, **iOS**
-   **Android**
-   **Web** ([asm.js](http://asmjs.org/) or [WebAssembly](http://webassembly.org/)),
    through [Emscripten](http://kripken.github.io/emscripten-site/)

See the Magnum Project [Build Status page](https://magnum.graphics/build-status/)
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

Check also the Magnum Project [Feature Overview pages](https://magnum.graphics/features/)
for further information.

WHAT'S NEW?
===========

Curious about what was added or improved recently? Check out the
[Changelog](https://doc.magnum.graphics/corrade/corrade-changelog.html#corrade-changelog-latest)
page in the documentation. Check also the Magnum Project
[Changelog](https://doc.magnum.graphics/magnum/changelog.html#changelog-latest).

GETTING STARTED
===============

Download, build and install Corrade as explained in
[the building documentation](https://doc.magnum.graphics/corrade/building-corrade.html)
— we provide packages for many platforms, including Windows, Linux and macOS.
After that, the best way to get started is to read some
[examples and tutorials](https://doc.magnum.graphics/corrade/corrade-example-index.html).

Apart from that, various Corrade functionality is available through
[single-header libraries](https://doc.magnum.graphics/corrade/corrade-singles.html).
Just download a file, `#include` it in your project and you're ready to go! No
buildsystem wrangling needed.

CONTACT & SUPPORT
=================

If you want to contribute to Corrade, if you spotted a bug, need a feature or
have an awesome idea, you can get a copy of the sources from GitHub and start
right away! There is the already mentioned guide about
[how to download and build Corrade](https://doc.magnum.graphics/corrade/building-corrade.html)
and also a guide about [coding style and best practices](https://doc.magnum.graphics/corrade/corrade-coding-style.html)
which you should follow to keep the library as consistent and maintainable as
possible.

-   Project homepage — https://magnum.graphics/corrade/
-   Documentation — https://doc.magnum.graphics/corrade/
-   GitHub — https://github.com/mosra/corrade and the
    [#magnum](https://github.com/topics/magnum) topic
-   GitLab — https://gitlab.com/mosra/corrade
-   Gitter community chat — https://gitter.im/mosra/magnum
-   E-mail — info@magnum.graphics
-   Google Groups mailing list — magnum-engine@googlegroups.com
    ([archive](https://groups.google.com/forum/#!forum/magnum-engine))
-   Bluesky — https://bsky.app/profile/mosra.cz

See also the Magnum Project [Contact & Support page](https://magnum.graphics/contact/)
for further information.

CREDITS
=======

[Detailed contributor list](https://doc.magnum.graphics/corrade/corrade-credits-contributors.html)
is maintained in the documentation. Big thanks to everyone involved!

There's also a list of [third party components](https://doc.magnum.graphics/corrade/corrade-credits-third-party.html)
that affect public use of the project.

LICENSE
=======

Corrade itself and its documentation is licensed under the MIT/Expat license,
see the [COPYING](COPYING) file for details. All example code in `src/examples`
is put into public domain (or UNLICENSE) to free you from any legal obstacles
when reusing the code in your apps. See the [COPYING-examples](COPYING-examples)
file for details.
