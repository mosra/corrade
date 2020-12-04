/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include "ArrayTuple.h"

#include "Corrade/Containers/Array.h"

namespace Corrade { namespace Containers {

/*

### Technical notes, aka "WHAT THE HELL IS THIS INSANITY"

The class instance stores just a pointer, size and a plain deleter function pointer. And everything else is contained in the pointed-to memory --- deleter
for the memory itself and all information needed to properly call destructors
on elements of non-trivially-destructible items. The binary layout is as
follows, the DestructibleItem struct is defined below in the code:

Offset          | Size      | Contents
----------------+-----------+------------------------------------
0               | 8 (4)     | Count of DestructibleItem records (N)
8 (4)           | 32 (16)   | First DestructibleItem record:
8 (8)           | 8 (4)     |   -   pointer to the beginning of the array (A1)
12 (12)         | 8 (4)     |   -   count of the elements in the array (C1)
24 (16)         | 8 (4)     |   -   size of each element (T) in the array (S1)
32 (20)         | 8 (4)     |   -   element destructor function pointer for T
40 (24)         | 32 (16)   | Second DestructibleItem record
...             | ...       | (remaining items)
8 + n*32 (16)   | 32 (16)   | DestructibleItem record for the memory itself
8 + n*32 (16)   | 8 (4)     |   -   pointer to the state (S) of the deleter D
12 + n*32 (16)  | 8 (4)     |   -   a value of `1`
16 + n*32 (16)  | 8 (4)     |   -   a value of `0`
24 + n*32 (16)  | 8 (4)     |   -   memory deleter function pointer for D
...             | ...       | (padding at the end)
A1              | C1*S2     | Contents of the first array,
...             | ...       | (padding at the end)
A2              | C2*S2     | Contents of the second array
...             | ...       | (padding at the end)
Ai              | Ci*Si     | (remaining arrays)
...             | ...       | (padding at the end)
S               | sizeof(D) | deleter state

The operation sequence when the ArrayTuple gets constructed is as follows
(simplified without taking the "Special cases / optimizations" below into
account):

1.  Memory gets allocated at a size appropriate for desired view types,
    alignments and element counts.
2.  For each type that needs explicit destruction, a DestructibleItem record is
    added and filled.
3.  Finally, one DestructibleItem is added for the memory itself.
4.  The view references passed in the constructor are filled with `{Ai, Ni}`
    values.
5.  For each of the views that wasn't specified with NoInit, elements are
    default-constructed.
6.  The class saves the allocation pointer, allocation size and a deleter
    function pointer.

On destruction, the following is done:

1.  The deleter function pointer stored in class is called with the allocation
    pointer and allocation size.
2.  The function goes through all DestructibleItem records, and for each calls
    the stored destructor function pointer for all `Ai + j*Si` memory locations
    where 0 <= i < N and 0 <= j <= Cj. This is the `arrayTupleDeleter()`
    function defined below in the code.

Because the memory deleter is stored as the last DestructibleItem in the list,
the last iteration of the loop also takes care of deleting the memory with the
correct deleter. This means we need to have a specially crafted function
pointer that's able to contain functionality for both calling destructors on
individual items as well as deleting the whole memory:

### The two kinds of DestructibleItem::destructor function pointers

The destructor function inside DestructibleItem takes two parameters -- (char*,
std::size_t) -- which is same as usual deleters in Array and elsewhere. The
same signature is used for the deleter function stored inside ArrayTuple
itself, and that allows for certain nice optimizations as noted in the
"Special cases / optimizations" section below.

For destructing particular elements, the function assumes the pointer is the
element instance, ignores whatever is passed in the second parameter and calls
a destructor:

    [](char* element, std::size_t) {
        reinterpret_cast<T*>(element)->~T();
    }

For deleting the memory itself, the function assumes it got a pointer to the
deleter state and size of the whole memory. Because the deleter is stored at
the very end of the allocation, it can use the knowledge of its own size and
size of the whole allocation to calculate a pointer to beginning of the memory
in order to pass it to the deleter. Finally, the deleter destructor is called
in order to account for deleters with non-trivial destructors.

    [](char* state, std::size_t size) {
        D deleter = *reinterpret_cast<D*>(state);
        deleter(state + sizeof(D) - size, size);
        deleter.~D();
    }

### Special cases / optimization

1, In case the memory deleter is a stateless function pointer, it can be stored
directly inside the DestructibleItem, without having to redirect to it like
above. To make it work, the DestructibleItem memory pointer points to the beginning of the allocation instead of the place where the deleter state would
be stored.

2. Similar case is when there's no custom deleter at all --- the deleter
function pointer is then simply the following:

    [](void* memory, std::size_t) {
        delete[] static_cast<char*>(memory);
    }

3. Another case is when all stored types are trivially destructible -- there's
only one DestructibleItem left for the memory deleter. If that one is a
stateless function pointer, it can be stored directly in the ArrayTuple deleter
pointer, and the allocated memory doesn't need to store any count of
DestructibleItem records at the beginning either, because there's nothing that
would iterate through these anyway. The top-level deleter call in ArrayTuple
gets passed the beginning of the allocation and its size, so that works the
same as in case (1).

4. Finally, if the stateless array deleter is the default deleter, there's no
need to store the wrapper lambda from case (2) in the ArrayTuple, Instead, the
ArrayTuple deleter pointer is `nullptr`, which makes the class simply do
`delete[] data` on destruction, consistently with what Array does.

*/

ArrayTuple::ArrayTuple(const ArrayView<const Item> items): ArrayTuple{items, [](std::size_t size, std::size_t) -> std::pair<char*, std::nullptr_t> {
    /** @todo use the alignment param once we implement aligned alloc */
    return {size ? new char[size] : nullptr, nullptr};
}} {}

ArrayTuple::ArrayTuple(ArrayTuple&& other) noexcept: _data{other._data}, _size{other._size}, _deleter{other._deleter} {
    other._data = nullptr;
    other._size = 0;
    other._deleter = {};
}

ArrayTuple::~ArrayTuple() {
    if(_deleter) _deleter(_data, _size);
    else delete[] _data;
}

ArrayTuple& ArrayTuple::operator=(ArrayTuple&& other) noexcept {
    using std::swap;
    swap(other._data, _data);
    swap(other._size, _size);
    swap(other._deleter, _deleter);
    return *this;
}

namespace {

struct DestructibleItem {
    char* data;
    std::size_t elementCount, elementSize;
    void(*destructor)(char*, std::size_t);
};

inline std::size_t alignFor(const std::size_t offset, const std::size_t alignment) {
    return ((offset + alignment - 1)/alignment)*alignment;
}

/* A deleter stored directly in ArrayTuple in case there's any types with
   non-trivial destructors or if there's a stateful array deleter. In case
   there are no non-trivially-destructible types and the array deleter is a
   stateless function pointer, the ArrayTuple stores directly the array
   deleter without going through this function, and the first 8 or 4 */
void arrayTupleDeleter(char* data, std::size_t dataSize) {
    /* The last destructor call may free the memory under us, so ensure it's
       not read again when checking the boundary conditions by saving the end
       condition to a temporary variable first */
    for(DestructibleItem *entry = reinterpret_cast<DestructibleItem*>(data + sizeof(void*)), *end = entry + *reinterpret_cast<std::size_t*>(data); entry != end; ++entry)
        for(std::size_t i = 0, iEnd = entry->elementCount; i != iEnd; ++i)
            entry->destructor(entry->data + i*entry->elementSize, dataSize);
}

}

ArrayTuple::operator Array<char>() && {
    CORRADE_ASSERT(_deleter != arrayTupleDeleter,
        "Containers::ArrayTuple: conversion to Array allowed only with trivially destructible types and a stateless destructor", {});
    const Deleter deleter = _deleter;
    const std::size_t size = _size;
    return Array<char>{release(), size, deleter};
}

std::pair<std::size_t, std::size_t> ArrayTuple::sizeAlignmentFor(const ArrayView<const Item> items, const Item& arrayDeleterItem, std::size_t& destructibleItemCount, bool& arrayDeleterItemNeeded) {
    /* Calculate how many items actually need their destructor called. If there
       is a non-trivial destructor but no actual items, we don't need to call
       anything either. */
    destructibleItemCount = 0;
    std::size_t maxAlignment = 1;
    for(const Item& item: items) {
        if(item._elementAlignment > maxAlignment)
            maxAlignment = item._elementAlignment;
        if(item._destructor && item._elementCount) ++destructibleItemCount;
    }

    /* If all items have trivially destructible types and the array deleter is
       a plain stateless function (indicated by zero alignment), the array
       deleter can be stored directly inside the ArrayTuple class. In any other
       case we need to store one more DestructibleItem for it. */
    arrayDeleterItemNeeded = destructibleItemCount || arrayDeleterItem._elementAlignment;

    /* If there are any destructible items, we need to store also their count
       at the beginning. Otherwise the allocation would be just (aligned) array
       contents. */
    std::size_t offset;
    if(const std::size_t totalDestructibleItems = destructibleItemCount + (arrayDeleterItemNeeded ? 1 : 0))
        offset = sizeof(std::size_t) + totalDestructibleItems*sizeof(DestructibleItem);
    else offset = 0;

    /* Add all array contents to the size */
    for(const Item& item: items) {
        offset = alignFor(offset, item._elementAlignment);
        offset += item._elementSize*item._elementCount;
    }

    /* If we have a stateful array deleter (indicated by non-zero alignment),
       we need to store its state as well */
    if(arrayDeleterItem._elementAlignment) {
        if(arrayDeleterItem._elementAlignment > maxAlignment)
            maxAlignment = arrayDeleterItem._elementAlignment;
        offset = alignFor(offset, arrayDeleterItem._elementAlignment);
        CORRADE_INTERNAL_ASSERT(arrayDeleterItem._elementCount == 1);
        offset += arrayDeleterItem._elementSize;
    }

    return {offset, maxAlignment};
}

void ArrayTuple::create(const ArrayView<const Item> items, const Item& arrayDeleterItem, const std::size_t destructibleItemCount, const bool arrayDeleterItemNeeded) {
    /* If we have destructible entries, store the total count and calculate the
       (unaligned) offset for the first array. If we don't have them, don't
       store anything -- the first array will be right at the start. */
    std::size_t offset;
    if(const std::size_t totalDestructibleItems = destructibleItemCount + (arrayDeleterItemNeeded ? 1 : 0)) {
        *reinterpret_cast<std::size_t*>(_data) = totalDestructibleItems;
        offset = sizeof(std::size_t) + totalDestructibleItems*sizeof(DestructibleItem);
    } else offset = 0;

    /* Store the items */
    auto* nextDestructibleItem = reinterpret_cast<DestructibleItem*>(_data + sizeof(std::size_t));
    for(std::size_t i = 0; i != items.size(); ++i) {
        offset = alignFor(offset, items[i]._elementAlignment);

        /* If the item has a default constructor, call it on each element */
        if(items[i]._constructor)
            for(std::size_t j = 0; j != items[i]._elementCount; ++j)
                items[i]._constructor(_data + offset + j*items[i]._elementAlignment);

        /* If the item has a destructor and there's not zero elements, populate
           the DestructibleItem instance */
        if(items[i]._destructor && items[i]._elementCount) {
            nextDestructibleItem->data = _data + offset;
            nextDestructibleItem->elementCount = items[i]._elementCount;
            nextDestructibleItem->elementSize = items[i]._elementSize;
            nextDestructibleItem->destructor = items[i]._destructor;
            ++nextDestructibleItem;
        }

        /* Save the data pointer to the output array. The size was already
           saved in the Item constructor */
        CORRADE_INTERNAL_ASSERT(items[i]._destinationPointer);
        *items[i]._destinationPointer = _data + offset;

        /* Increase the offset for next round */
        offset += items[i]._elementCount*items[i]._elementSize;
    }

    /* Check that we're consistent with what sizeFor() calculated */
    CORRADE_INTERNAL_ASSERT(nextDestructibleItem - destructibleItemCount == static_cast<void*>(_data + sizeof(std::size_t)));
    CORRADE_INTERNAL_ASSERT(offset == _size || (arrayDeleterItemNeeded && arrayDeleterItem._elementAlignment && arrayDeleterItem._elementSize));

    /* Store the array deleter, if needed */
    if(arrayDeleterItemNeeded) {
        nextDestructibleItem->elementCount = 1;
        nextDestructibleItem->elementSize = 0;

        CORRADE_INTERNAL_ASSERT(arrayDeleterItem._destinationPointer);

        /* If the deleter is the default one, pass it a pointer to the array
           memory and store a lambda wrapping delete[] (which was created in
           the Item constructor). The caller isn't expected to update the
           deleter destination with anything, so set that to nullptr. */
        if(arrayDeleterItem._elementSize == 0) {
            nextDestructibleItem->data = _data;
            CORRADE_INTERNAL_ASSERT_OUTPUT(nextDestructibleItem->destructor = arrayDeleterItem._destructor);

            *arrayDeleterItem._destinationPointer = nullptr;

        /* If it's a stateless function pointer, the actual pointer will be
           saved to the destructor through _outputPointer. */
        } else if(arrayDeleterItem._elementAlignment == 0) {
            nextDestructibleItem->data = _data;

            *arrayDeleterItem._destinationPointer = &nextDestructibleItem->destructor;

        /* Otherwise the location for the deleter state is at the (aligned)
           very end, sanity-check that sizeFor() calculated the same.
           The destructor function pointer wraps a call to it and the actual
           state will be saved by the caller. */
        } else {
            nextDestructibleItem->data = _data + alignFor(offset, arrayDeleterItem._elementAlignment);
            CORRADE_INTERNAL_ASSERT_OUTPUT(nextDestructibleItem->destructor = arrayDeleterItem._destructor);
            CORRADE_INTERNAL_ASSERT(nextDestructibleItem->data + arrayDeleterItem._elementSize == _data + _size);

            *arrayDeleterItem._destinationPointer = nextDestructibleItem->data;
        }

        /* The top-level deleter is then the nested loop that goes over all
           DestructibleItem deleters and calls them */
        _deleter = arrayTupleDeleter;

    /* Otherwise, if we have a stateless function pointer, call it directly.
       Like above, the pointer is filled afterwards via _outputPointer */
    } else if(arrayDeleterItem._elementSize) {
        *arrayDeleterItem._destinationPointer = &_deleter;

    /* Otherwise, it's the default deleter */
    } else _deleter = nullptr;
}

char* ArrayTuple::release() {
    char* const data = _data;
    _data = nullptr;
    _size = 0;
    _deleter = nullptr;
    return data;
}

}}
