#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020, 2021, 2022
#             Vladimír Vondruš <mosra@centrum.cz>
#   Copyright © 2022 Guillaume Jacquemin <williamjcm@users.noreply.github.com>
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

import math

import gdb, gdb.printing

class ArrayIterator:
    def __init__(self, begin, end):
        self.item = begin
        self.end = end
        self.count = 0

    def __iter__(self):
        return self

    def __next__(self):
        count = self.count
        self.count = self.count + 1
        if self.item == self.end:
            raise StopIteration
        item = self.item.dereference()
        self.item = self.item + 1
        return (f"[{count}]", item)

class ConfigurationIterator:
    def __init__(self, groups: gdb.Value, values: gdb.Value):
        self.groups = groups
        self.values = values
        self.groups_printed = False
        self.values_printed = False

    def __iter__(self):
        return self

    def __next__(self):
        if self.groups_printed and self.values_printed:
            raise StopIteration
        if not self.groups_printed:
            self.groups_printed = True
            return ('[groups]', self.groups)
        if not self.values_printed:
            self.values_printed = True
            return ('[values]', self.values)

class CorradeTypePrinter:
    """Base class for all pretty-printers"""

    def __init__(self, val: gdb.Value):
        self.val = val
        self.enabled: bool = True

    def to_string(self):
        return f"{self.val.type}"

class CorradeArrayTypePrinter(CorradeTypePrinter):
    """Base class for array-like pretty-printers"""

    def children(self):
        raise NotImplementedError("CorradeArrayTypePrinter children()")

    def display_hint(self):
        return 'array'

class CorradeStringTypePrinter(CorradeTypePrinter):
    """Base class for string-like pretty-printers"""

    def display_hint(self):
        return 'string'

class CorradeArray(CorradeArrayTypePrinter):
    """Prints a Containers::Array(View)"""

    def children(self):
        return ArrayIterator(self.val['_data'], self.val['_data'] + self.val['_size'])

    def to_string(self):
        return f"{self.val.type.name.split('<')[0]}<{self.val.type.template_argument(0)}> of size {self.val['_size']}"

class CorradeBitArray(CorradeArrayTypePrinter):
    """Prints a Containers::BitArray or BasicBitArrayView"""

    class BitArrayIterator:
        def __init__(self, data, size_offset: int):
            self.data = data
            self.size_offset = size_offset
            self.count: int = 0

        def __iter__(self):
            return self

        def __next__(self):
            count = self.count
            self.count = self.count + 1
            if count == (self.size_offset >> 3):
                raise StopIteration
            bit: bool = bool(self.data[((self.size_offset & 0x07) + count) >> 3] &
                             (1 << ((self.size_offset + count) & 0x7)))
            return (f"[{count}]", bit)

    def __init__(self, val: gdb.Value):
        super(CorradeBitArray, self).__init__(val)
        self.size_offset = int(val['_sizeOffset'])

    def children(self):
        return self.BitArrayIterator(self.val['_data'], self.size_offset)

    def to_string(self):
        return f"{self.val.type} of size {self.size_offset >> 3}"

class CorradeEnumSet(CorradeTypePrinter):
    """Prints a Containers::EnumSet"""

    class EnumSetIterator:
        def __init__(self, value: gdb.Value, enum_type: gdb.Type):
            self.value = value
            self.type = enum_type
            self.printed = False

        def __iter__(self):
            return self

        def __next__(self):
            if self.printed:
                raise StopIteration
            self.printed = True
            return ('value', self.value.cast(self.type))

    def children(self):
        return self.EnumSetIterator(self.val['_value'], self.val.type.template_argument(0))

class CorradeBigEnumSet(CorradeTypePrinter):
    """Prints a Containers::BigEnumSet"""

    def __init__(self, val: gdb.Value):
        self.size = int(val.type.strip_typedefs().name.split(',')[1].split('>')[0])
        super(CorradeBigEnumSet, self).__init__(val)

    class BigEnumSetIterator:
        def __init__(self, data: gdb.Value, size: int, enum_type: gdb.Type):
            self.data = data
            self.size = size
            self.type = enum_type
            self.index = 0

        def __iter__(self):
            return self

        def __next__(self):
            index = self.index
            self.index = self.index + 1
            if index == self.size * 64:
                raise StopIteration
            return (str(gdb.parse_and_eval(f'static_cast<{self.type.name}>({index})')),
                    bool(int(self.data[math.trunc(index / 64)]) & (1 << index % 64)))

    def children(self):
        return self.BigEnumSetIterator(self.val['_data'], self.size, self.val.type.template_argument(0))

class CorradeLinkedList(CorradeArrayTypePrinter):
    """Prints a Containers::LinkedList"""

    class LinkedListIterator:
        def __init__(self, first: gdb.Value, last: gdb.Value):
            self.item = first
            self.last = last
            self.count = 0

        def __iter__(self):
            return self

        def __next__(self):
            count = self.count
            self.count = self.count + 1
            if int(self.item) == 0x0:
                raise StopIteration
            item = self.item.dereference()
            self.item = self.item['_next']
            return (f"[{count}]", item)

    def children(self):
        return self.LinkedListIterator(self.val['_first'], self.val['_last'])

class CorradeLinkedListItem(CorradeTypePrinter):
    """
    Prints a Containers::LinkedListItem.

    This is mostly to limit noise caused by GDB trying to do recursive printing
    if LinkedList's T has no pretty-printer.
    """

    def to_string(self):
        return f"Corrade::Containers::LinkedListItem<{self.val.type.template_argument(0)}>"

class CorradeOptional(CorradeTypePrinter):
    """Prints a Containers::Optional"""

    class OptionalIterator:
        def __init__(self, val: gdb.Value | None):
            self.val = val

        def __iter__(self):
            return self

        def __next__(self):
            if self.val is None:
                raise StopIteration
            self.val, val = None, self.val
            return ('[value]', val)

    def __init__(self, val: gdb.Value):
        self.set = bool(val['_set'])
        self.printer = gdb.default_visualizer(val['_value'])
        super(CorradeOptional, self).__init__(val)

    def children(self):
        if self.set is False:
            return self.OptionalIterator(None)
        if hasattr(self.printer, 'children'):
            return self.printer.children()
        return self.OptionalIterator(self.val['_value'])

    def to_string(self):
        if self.set is False:
            return f"{self.val.type} [no value]"
        return f"{self.val.type.name.split('<')[0]} containing {self.val.type.template_argument(0)}"

    def display_hint(self):
        if hasattr(self.printer, 'children') and hasattr(self.printer, 'display_hint'):
            return self.printer.display_hint()
        return None

class CorradePairTriple(CorradeTypePrinter):
    """Prints a Containers::Pair or Triple"""

    class Iterator:
        def __init__(self, val: gdb.Value, fields: list[gdb.Field]):
            self.val = val
            self.fields = fields
            self.index = 0

        def __iter__(self):
            return self

        def __next__(self):
            index = self.index
            self.index = self.index + 1
            if index == len(self.fields):
                raise StopIteration
            return (f"[{self.fields[index].name.split('_')[1]}]", self.val[self.fields[index]])

    def children(self):
        return self.Iterator(self.val, self.val.type.fields())

class CorradePointer(CorradeTypePrinter):
    """Prints a Containers::Pointer"""

    class PointerIterator:
        def __init__(self, val: gdb.Value):
            self.val = val

        def __iter__(self):
            return self

        def __next__(self):
            if self.val is None:
                raise StopIteration
            self.val, val = None, self.val
            if int(val) == 0:
                return ('get()', 'nullptr')
            if val.type != val.dynamic_type:
                return ('get()', val.cast(val.dynamic_type).dereference())
            return ('get()', val.dereference())

    def children(self):
        return self.PointerIterator(self.val['_pointer'])

class CorradeReference(CorradeTypePrinter):
    """Prints a Containers::(Any,Move)Reference"""

    class RefIterator:
        def __init__(self, val: gdb.Value):
            self.val = val

        def __iter__(self):
            return self

        def __next__(self):
            if self.val is None:
                raise StopIteration
            self.val, val = None, self.val
            if val.type != val.dynamic_type:
                return ('get()', val.cast(val.dynamic_type).referenced_value())
            return ('get()', val.referenced_value())

    def children(self):
        return self.RefIterator(self.val['_reference'])

class CorradeStaticArray(CorradeArrayTypePrinter):
    """Prints a Containers::StaticArray(View)"""

    def __init__(self, val: gdb.Value):
        super(CorradeStaticArray, self).__init__(val)
        self.size = int(val.type.name.split('<')[1].split(',')[0].strip())

    def children(self):
        return ArrayIterator(self.val['_data'][0].address, self.val['_data'][self.size].address)

class CorradeStridedArrayView(CorradeTypePrinter):
    """Prints a Containers::StridedArrayView"""

    class StridedIterator:
        def __init__(self, data: gdb.Value, dimensions: int, data_type: gdb.Type, size: gdb.Value):
            self.data = data
            self.dimensions = dimensions
            self.type = data_type
            self.count = 0
            self.size = size

            self.total_size = 1 # Starting at 1 so the multiplication works
            for i in range(0, dimensions):
                self.total_size *= int(size[i])

        def __iter__(self):
            return self

        def __next__(self):
            count = self.count
            self.count = self.count + 1
            if count == self.total_size:
                raise StopIteration

            data = self.data.cast(self.type.pointer())
            indexes: list[int] = []
            if self.dimensions > 1:
                index_size = 1
                sizes = []
                for i in range(1, self.dimensions):
                    sizes.append(int(self.size[i]))
                for size in sizes:
                    indexes.append(math.floor(count / index_size) % int(size))
                    index_size *= size
                indexes.append(math.floor(count / index_size))
                indexes.reverse()
            else:
                indexes.append(count)
            return (f"{indexes}", data[count])

    def __init__(self, val: gdb.Value):
        super(CorradeStridedArrayView, self).__init__(val)
        self.size = self.val['_size']['_data']
        self.type = self.val.type.strip_typedefs().template_argument(1)
        self.dimensions = int(self.val.type.strip_typedefs().name.split('<')[1].split(',')[0].strip())

    def children(self):
        return self.StridedIterator(self.val['_data'], self.dimensions, self.type, self.size)

    def to_string(self):
        return f"{self.val.type} of size {self.size}"

class CorradeString(CorradeStringTypePrinter):
    """Prints a Containers::String"""

    def to_string(self):
        if bool(self.val['_small']['size'] & 0x80) is True:
            return self.val['_small']['data'].string(length=int(self.val['_small']['size'] & ~0xc0))
        return self.val['_large']['data'].string(length=int(self.val['_large']['size']))

class CorradeStringView(CorradeStringTypePrinter):
    """Prints a Containers::BasicStringView"""

    def __init__(self, val: gdb.Value):
        super(CorradeStringView, self).__init__(val)
        sizeof_sizet: int = gdb.lookup_type('std::size_t').sizeof
        self.size: int = int(self.val['_sizePlusFlags']) & ~((1 << (sizeof_sizet * 8 - 2)) | (1 << (sizeof_sizet * 8 - 1)))

    def to_string(self):
        return self.val['_data'].string(length=self.size)

class CorradeConfiguration(CorradeTypePrinter):
    """Prints an Utility::Configuration"""

    def children(self):
        return ConfigurationIterator(self.val['_groups'], self.val['_values'])

    def to_string(self):
        return f"{self.val.type} (file: {self.val['_filename']})"

class CorradeConfigurationGroup(CorradeTypePrinter):
    """Prints an Utility::ConfigurationGroup"""

    def children(self):
        return ConfigurationIterator(self.val['_groups'], self.val['_values'])

    def to_string(self):
        return f"{self.val.type} named {self.val['_name']}"

class CorradeConfigurationGroupGroup(CorradeTypePrinter):
    """Prints an Utility::ConfigurationGroup::Group"""

    def children(self):
        actual_value = self.val['group'].dereference()
        return ConfigurationIterator(actual_value['_groups'], actual_value['_values'])

    def to_string(self):
        return f"{self.val.type} named {self.val['name']}"

corrade_printers: gdb.printing.RegexpCollectionPrettyPrinter | None = None

def build_corrade_printer():
    global corrade_printers

    if corrade_printers is not None:
        return

    corrade_printers = gdb.printing.RegexpCollectionPrettyPrinter("Corrade")
    corrade_printers.add_printer("Containers::Array",
                                 "^Corrade::Containers::Array<.*>$",
                                 CorradeArray)
    corrade_printers.add_printer("Containers::ArrayView",
                                 "^Corrade::Containers::ArrayView<.*>$",
                                 CorradeArray)
    corrade_printers.add_printer("Containers::BitArray",
                                 "^Corrade::Containers::BitArray$",
                                 CorradeBitArray)
    corrade_printers.add_printer("Containers::BasicBitArrayView",
                                 "^Corrade::Containers::BasicBitArrayView<.*>$",
                                 CorradeBitArray)
    corrade_printers.add_printer("Containers::EnumSet",
                                 "^Corrade::Containers::EnumSet<.*>$",
                                 CorradeEnumSet)
    corrade_printers.add_printer("Containers::BigEnumSet",
                                 "^Corrade::Containers::BigEnumSet<.*>$",
                                 CorradeBigEnumSet)
    corrade_printers.add_printer("Containers::LinkedList",
                                 "^Corrade::Containers::LinkedList<.*>$",
                                 CorradeLinkedList)
    corrade_printers.add_printer("Containers::LinkedListItem",
                                 "^Corrade::Containers::LinkedListItem<.*>$",
                                 CorradeLinkedListItem)
    corrade_printers.add_printer("Containers::Optional",
                                 "^Corrade::Containers::Optional<.*>$",
                                 CorradeOptional)
    corrade_printers.add_printer("Containers::Pair",
                                 "^Corrade::Containers::Pair<.*>$",
                                 CorradePairTriple)
    corrade_printers.add_printer("Containers::Triple",
                                 "^Corrade::Containers::Triple<.*>$",
                                 CorradePairTriple)
    corrade_printers.add_printer("Containers::Pointer",
                                 "^Corrade::Containers::Pointer<.*>$",
                                 CorradePointer)
    corrade_printers.add_printer("Containers::Reference",
                                 "^Corrade::Containers::Reference<.*>$",
                                 CorradeReference)
    corrade_printers.add_printer("Containers::AnyReference",
                                 "^Corrade::Containers::AnyReference<.*>$",
                                 CorradeReference)
    corrade_printers.add_printer("Containers::MoveReference",
                                 "^Corrade::Containers::MoveReference<.*>$",
                                 CorradeReference)
    corrade_printers.add_printer("Containers::StaticArray",
                                 "^Corrade::Containers::StaticArray<\d+, .*>$",
                                 CorradeStaticArray)
    corrade_printers.add_printer("Containers::StaticArrayView",
                                 "^Corrade::Containers::StaticArrayView<\d+, .*>$",
                                 CorradeStaticArray)
    corrade_printers.add_printer("Containers::StridedArrayView",
                                 "^Corrade::Containers::StridedArrayView<.*>$",
                                 CorradeStridedArrayView)
    corrade_printers.add_printer("Containers::String",
                                 "^Corrade::Containers::String$",
                                 CorradeString)
    corrade_printers.add_printer("Containers::BasicStringView",
                                 "^Corrade::Containers::BasicStringView<.*>$",
                                 CorradeStringView)
    corrade_printers.add_printer("Utility::Configuration",
                                 "^Corrade::Utility::Configuration$",
                                 CorradeConfiguration)
    corrade_printers.add_printer("Utility::ConfigurationGroup",
                                 "^Corrade::Utility::ConfigurationGroup$",
                                 CorradeConfigurationGroup)
    corrade_printers.add_printer("Utility::ConfigurationGroup::Group",
                                 "^Corrade::Utility::ConfigurationGroup::Group$",
                                 CorradeConfigurationGroupGroup)

def register_corrade_printers(obj: gdb.Objfile | gdb.Progspace):
    if obj is None:
        obj = gdb

    global corrade_printers

    if corrade_printers is None:
        build_corrade_printer()

    gdb.printing.register_pretty_printer(obj, corrade_printers)
