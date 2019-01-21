#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>
#
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.
#

import os
import shutil
import unittest

from acme import simplify_expression, acme

class ParseExpression(unittest.TestCase):
    def test_basic(self):
        self.assertEqual(simplify_expression('if', '0'),
            (False, 'if', '0'))
        self.assertEqual(simplify_expression('if', '1'),
            (True, 'if', '1'))
        self.assertEqual(simplify_expression('if', 'SOMETHING'),
            (None, 'if', 'SOMETHING'))

    def test_normalization(self):
        self.assertEqual(simplify_expression('ifdef', 'FOO'),
            (None, 'ifdef', 'FOO'))
        self.assertEqual(simplify_expression('ifndef', 'FOO'),
            (None, 'ifndef', 'FOO'))
        self.assertEqual(simplify_expression('if', 'defined(FOO)'),
            (None, 'ifdef', 'FOO'))
        self.assertEqual(simplify_expression('elif', 'defined(FOO)'),
            (None, 'elif', 'defined(FOO)'))

    def test_basic_simplify(self):
        self.assertEqual(simplify_expression('if', '0 || 0'),
            (False, 'if', '0'))
        self.assertEqual(simplify_expression('if', '1 && 0'),
            (False, 'if', '0'))
        self.assertEqual(simplify_expression('if', '1 && 1'),
            (True, 'if', '1'))
        self.assertEqual(simplify_expression('if', '0 || 1'),
            (True, 'if', '1'))
        self.assertEqual(simplify_expression('if', '(1)'),
            (True, 'if', '1'))
        self.assertEqual(simplify_expression('if', '(!defined(FOO))'),
            (None, 'ifndef', 'FOO'))
        self.assertEqual(simplify_expression('elif', '!(!!0)'),
            (True, 'elif', '1'))

    def test_simplify(self):
        self.assertEqual(simplify_expression('if', '0 || (!defined(FOO) && 1)'),
            (None, 'ifndef', 'FOO'))
        self.assertEqual(simplify_expression('elif', '!(0 || defined(A)) && (defined(B) && 1)'),
            (None, 'elif', '!defined(A) && defined(B)'))
        self.assertEqual(simplify_expression('elif', '!(0 && defined(A)) && (defined(B) || 1)'),
            (True, 'elif', '1'))

    def test_forced_defines(self):
        self.assertEqual(simplify_expression('elif', '!(0 || defined(A)) && (defined(B) && 1)', {'A': False}),
            (None, 'elif', 'defined(B)'))
        self.assertEqual(simplify_expression('elif', '!(0 || defined(A)) && (defined(B) && 1)', {'B': True}),
            (None, 'elif', '!defined(A)'))
        self.assertEqual(simplify_expression('elif', 'defined(CORRADE_NO_DEBUG) || defined(NDEBUG)', {'DOXYGEN_GENERATING_OUTPUT': False, 'CORRADE_NO_DEBUG': True}),
            (True, 'elif', '1'))

class ParseFile(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        unittest.TestCase.__init__(self, *args, **kwargs)
        self.path = os.path.dirname(os.path.realpath(__file__))

        # Display ALL THE DIFFS
        self.maxDiff = None

    def setUp(self):
        if os.path.exists(os.path.join(self.path, 'output')): shutil.rmtree(os.path.join(self.path, 'output'))

    def run_acme(self, path):
        test_dir = os.path.join(self.path, path)
        acme(os.path.join(test_dir, 'input.h'), os.path.join(test_dir, 'actual.h'))

        with open(os.path.join(test_dir, 'expected.h')) as f:
            expected_contents = f.read()
        with open(os.path.join(test_dir, 'actual.h')) as f:
            actual_contents = f.read()
        return actual_contents, expected_contents

    def test_comments(self):
        self.assertEqual(*self.run_acme('comments'))

    def test_includes(self):
        self.assertEqual(*self.run_acme('includes'))

    def test_preprocessor(self):
        self.assertEqual(*self.run_acme('preprocessor'))

    def test_pragmas(self):
        self.assertEqual(*self.run_acme('pragmas'))
