/*
    This file is part of a single header. library

    Copyright © 1997, 2006 John Doe <john@doe.com>
    Copyright © 2007, 2008, 2009, 2010, 2011,
                2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019
                Some Long-Time Contributor <me@me.team>
    Copyright © 2020 Some Guy <oh@well>

    SOME MORE TEXT
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
