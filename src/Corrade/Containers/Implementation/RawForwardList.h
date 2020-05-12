#ifndef Corrade_Containers_Implementation_RawForwardList_h
#define Corrade_Containers_Implementation_RawForwardList_h
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

namespace Corrade { namespace Containers { namespace Implementation {

/*
    Utilities used by Resource and PluginManager to manage allocation-free
    resource / plugin registration at app startup.

    The item type is expected to have a `next` member, initialized to
    `nullptr`. When added to the list, the `next` member is either set to
    previous list head or to self, to indicate end of the list -- it's not
    `nullptr` in order to make it easy to check if it's already in the list.
*/

template<class T> inline void forwardListInsert(T*& list, T& item) {
    /* If the item is already in a list, do nothing -- handles plugin /
       resource registration done more than once. */
    if(item.next) return;

    /* If this is the first item in the list, set its next to `nullptr` to
       make it possible to distinguish if the item is in a list or not (as
       it would be `nullptr` otherwise as well. */
    if(list) item.next = list;
    else item.next = &item;

    /* The item is the new list head */
    list = &item;
}

template<class T> inline void forwardListRemove(T*& list, T& item) {
    /* If the item is not in a list, do nothing -- handles plugin /
       resource registration done more than once. */
    if(!item.next) return;

    /* Special case if the item is first */
    if(list == &item) {
        /* ... and if it's last as well */
        if(item.next == &item) list = nullptr;
        else list = item.next;
        item.next = nullptr;
        return;
    }

    /* Assuming the item is in the list, this shouldn't cycle */
    T* prev = list;
    while(prev->next != &item) prev = prev->next;

    /* Special case if the item is last */
    if(item.next == &item) prev->next = prev;
    else prev->next = item.next;
    item.next = nullptr;
}

template<class T> inline T* forwardListNext(T& item) {
    /* List end is denoted by pointer to self, return nullptr in that case */
    return item.next == &item ? nullptr : item.next;
}

}}}

#endif
