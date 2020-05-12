/*
    This file is part of a single header. library

    {{copyright}}

    SOME MORE TEXT
*/

/*
    Copyright © 1997, 2006 John Doe <john@doe.com>

    the whole contents of this block will get removed

    Copyright © 2007, 2008, 2009, 2010, 2011,
                2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

    THIS IS A LOUD LINE

    Copyright © 2020 Some Guy <oh@well>

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019
                Some Long-Time Contributor <me@me.team>

    Copyright is fine, but don't go *too* extreme with that, ok? :>
*/

/** @file
 * @brief A file comment
 */

// some comment
void foo();

/* a
     block
   comment */

/** @brief a single-line block comment */
extern int a;

struct Foo {
    /* The following line should be kept */
    /*implicit*/ Foo() = default;
};
