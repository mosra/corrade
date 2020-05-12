#!/usr/bin/env python3

#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>
#   Copyright © 2019 Jonathan Hale <squareys@googlemail.com>
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

import hashlib

# Input data for every test case in Sha1Test.cpp
inputs = {
    "emptyString": [""],
    "exact64bytes": [
        "123456789a123456789b123456789c123456789d123456789e123456789f1234"],
    "exactOneBlockPadding": [
        "123456789a123456789b123456789c123456789d123456789e12345"],
    "twoBlockPadding": [
        "123456789a123456789b123456789c123456789d123456789e123456"],
    "zeroInLeftover": [
        "123456789a123456789b123456789c123456789d123456789e123456789f12341\000134",
        "\0001"],
    "noStringDelimiter" : [b"12345"]
}

# Print byte array data and length
debug = False

# Compute and print sha1 of test inputs
for name in inputs:
    data = inputs[name]
    m = hashlib.sha1()
    for d in data:
        bytesArray = d if type(d) == bytes else d.encode()
        m.update(bytesArray)
        if debug:
            print("data: {}\nlength:{}\n".format(bytesArray, len(bytesArray)))
    print(name + ' = "' + m.hexdigest() + '"')
