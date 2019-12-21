#!/usr/bin/env python3
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
