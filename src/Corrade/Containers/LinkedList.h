#ifndef Corrade_Containers_LinkedList_h
#define Corrade_Containers_LinkedList_h
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

/** @file
 * @brief Class @ref Corrade::Containers::LinkedList, @ref Corrade::Containers::LinkedListItem
 */

#include "Corrade/Containers/Containers.h"
#include "Corrade/Utility/Assert.h"

namespace Corrade { namespace Containers {

/**
@brief Linked list
@tparam T   Item type, derived from @ref LinkedListItem

The list stores pointers to items which contain iterators in itself, not the
other way around, so it is possible to operate directly with pointers to the
items without any abstraction at *constant* time. The only downside of this is
that the items or list cannot be copied (but they can be moved).

@note For simplicity and memory usage reasons the list doesn't provide any
method to get count of stored items, but you can traverse them and count them
manually if desperately needed.

@section Containers-LinkedList-basic-usage Basic usage

Usage involves creating a subclass of @ref LinkedListItem and then adding them
to the list instance. By default, the list instance takes ownership of the
added items, calling @cpp delete @ce on each on destruction. See
@ref Containers-LinkedList-memory-management for details and other
possibilities.

@snippet Containers.cpp LinkedList-usage

Traversing through the list can be done using range-based for:

@snippet Containers.cpp LinkedList-traversal

Or, if you need more flexibility, like in the following code. It is also
possible to go in reverse order using @ref last() and
@ref LinkedListItem::previous().

@snippet Containers.cpp LinkedList-traversal-classic

@section Containers-LinkedList-list-pointer Making advantage of pointer to the list

Each node stores pointer to the list, which you can take advantage of. For
example, if you have group of some objects and want to access the group from
each object, you can reuse the @ref LinkedListItem::list() pointer, which will
be cast to type you specify as @p List template parameter of
@ref LinkedListItem class:

@snippet Containers.cpp LinkedList-list-pointer

@section Containers-LinkedList-private-inheritance Using private inheritance

You might want to subclass LinkedList and LinkedListItem privately and for
example provide wrapper functions with more descriptive names. In that case
you need to friend both LinkedList and LinkedListItem in both your subclasses.

@snippet Containers.cpp LinkedList-private-inheritance

@section Containers-LinkedList-memory-management Memory management

By default, the list takes ownership of all its items. When the list is
destructed, @ref clear() or @ref erase() is called,
@ref LinkedListItem::erase() is called on each item, which in turn calls
@cpp delete this @ce. For cases where such behavior is not desirable (e.g.
items meant to be owned by something else than the list),
@ref LinkedListItem::erase() can be overriden to prevent this behavior. See its
documentation for more information.
*/
template<class T> class LinkedList {
    public:
        #ifndef DOXYGEN_GENERATING_OUTPUT
        class Iterator;
        class ConstIterator;
        #endif

        /**
         * @brief Default constructor
         *
         * Creates empty list.
         */
        constexpr explicit LinkedList() noexcept: _first(nullptr), _last(nullptr) {}

        /** @brief Copying is not allowed */
        LinkedList(const LinkedList<T>&) = delete;

        /** @brief Move constructor */
        LinkedList(LinkedList<T>&& other) noexcept;

        /**
         * @brief Destructor
         *
         * Clears the list.
         */
        ~LinkedList() { clear(); }

        /** @brief Copying is not allowed */
        LinkedList<T>& operator=(const LinkedList<T>&) = delete;

        /** @brief Move assignment */
        LinkedList<T>& operator=(LinkedList<T>&& other);

        /** @brief First item or `nullptr`, if the list is empty */
        T* first() { return _first; }
        constexpr const T* first() const { return _first; } /**< @overload */

        /** @brief Last item or `nullptr`, if the list is empty */
        T* last() { return _last; }
        constexpr const T* last() const { return _last; } /**< @overload */

        /** @brief Whether the list is empty */
        constexpr bool isEmpty() const { return !_first; }

        /**
         * @brief Insert item
         * @param item      Item to insert
         * @param before    Item before which to insert or @cpp nullptr @ce, if
         *      inserting at the end.
         *
         * @attention The item must not be connected to any list.
         */
        void insert(T* item, T* before = nullptr);

        /**
         * @brief Cut item out
         * @param item      Item to cut out
         *
         * The item is disconnected from the list, but not deleted.
         */
        void cut(T* item);

        /**
         * @brief Move item before another
         * @param item      Item to move
         * @param before    Item before which to move or @cpp nullptr @ce, if
         *      moving at the end.
         *
         * Equivalent to the following:
         *
         * @snippet Containers.cpp LinkedList-move
         */
        void move(T* item, T* before);

        /**
         * @brief Erase item
         * @param item      Item to erase
         *
         * Equivalent to calling @ref LinkedListItem::erase(). See its
         * documentation for more information.
         */
        void erase(T* item);

        /** @brief Clear the list */
        void clear();

    private:
        T *_first, *_last;
};

/**
@brief Item of @ref LinkedList
@tparam Derived Dervied object type, i.e. type you want returned from @ref previous() and @ref next().
@tparam List    List object type, i.e. type you want returned from @ref list().

This class is usually subclassed using [CRTP](http://en.wikipedia.org/wiki/Curiously_Recurring_Template_Pattern),
e.g.:

@snippet Containers.cpp LinkedListItem-usage

See @ref LinkedList for more information.
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class Derived, class List = LinkedList<Derived>>
#else
template<class Derived, class List>
#endif
class LinkedListItem {
    friend LinkedList<Derived>;

    public:
        /**
         * @brief Default constructor
         *
         * Creates item not connected to any list.
         */
        LinkedListItem() noexcept: _list(nullptr), _previous(nullptr), _next(nullptr) {}

        /** @brief Copying is not allowed */
        LinkedListItem(const LinkedListItem<Derived, List>&) = delete;

        /** @brief Move constructor */
        LinkedListItem(LinkedListItem<Derived, List>&& other);

        /** @brief Copying is not allowed */
        LinkedListItem<Derived, List>& operator=(const LinkedListItem<Derived, List>&) = delete;

        /** @brief Move assignment */
        LinkedListItem<Derived, List>& operator=(LinkedListItem<Derived, List>&& other);

        /**
         * @brief Destructor
         *
         * If the item is part of any list, it is removed from it.
         */
        virtual ~LinkedListItem() = 0;

        /** @brief List this item belongs to */
        List* list() { return _list; }
        const List* list() const { return _list; } /**< @overload */

        /** @brief Previous item or `nullptr`, if there is no previous item */
        Derived* previous() { return _previous; }
        const Derived* previous() const { return _previous; } /**< @overload */

        /** @brief Next item or `nullptr`, if there is no next item */
        Derived* next() { return _next; }
        const Derived* next() const { return _next; } /**< @overload */

        /**
         * @brief Erase the item
         *
         * Called from @ref LinkedList destructor, @ref LinkedList::clear() and
         * @ref LinkedList::erase(). By default, calls @ref doErase(), which
         * then calls @ref LinkedList::cut() followed by @cpp delete this @ce.
         * For cases where this is not desired (for example when providing
         * bindings to reference-counted languages), it's possible to provide a
         * different behavior by either:
         *
         * -    *replacing* this non-virtual function in your derived class
         *      (faster, doesn't involve a @cpp virtual @ce call, but requires
         *      the corresponding @ref LinkedList to be templated on the type
         *      that provides the replacement function),
         * -    or overriding the private @ref doErase() function (slower due
         *      to the @cpp virtual @ce call, but without imposing any
         *      restrictions on the @ref LinkedList type)
         *
         * The overriden implementation has to call @ref LinkedList::cut() in
         * order to correctly remove itself from the list; the @ref list() is
         * guaranteed to be non-@cpp nullptr @ce in this context.
         *
         * @attention This function is not meant to be called by the user, use
         *      @ref LinkedList::erase() instead.
         */
        void erase();

    private:
        /**
         * @brief Erase the item
         *
         * Implementation for @ref erase(), see its documentation for more
         * information. Default implementation calls
         * @ref LinkedList::cut() followed by @cpp delete this @ce.
         */
        virtual void doErase();

        List* _list;
        Derived *_previous, *_next;
};

template<class T> LinkedList<T>::LinkedList(LinkedList<T>&& other) noexcept: _first(other._first), _last(other._last) {
    other._first = nullptr;
    other._last = nullptr;

    /* Backreference this list from the items */
    for(T* i = _first; i; i = i->_next)
        i->_list = static_cast<decltype(i->_list)>(this);
}

template<class T> LinkedList<T>& LinkedList<T>::operator=(LinkedList<T>&& other) {
    /** @todo Make it noexcept */
    clear();
    _first = other._first;
    _last = other._last;
    other._first = nullptr;
    other._last = nullptr;

    /* Backreference this list from the items */
    for(T* i = _first; i; i = i->_next)
        i->_list = static_cast<decltype(i->_list)>(this);

    return *this;
}

template<class T> void LinkedList<T>::insert(T* const item, T* const before) {
    CORRADE_ASSERT(!item->_list, "Containers::LinkedList::insert(): cannot insert an item already connected elsewhere", );
    CORRADE_ASSERT(!before || before->_list == this, "Containers::LinkedList::insert(): cannot insert before an item which is not a part of the list", );

    item->_list = static_cast<decltype(item->_list)>(this);

    /* Adding as last item */
    if(!before) {
        /* First item in the list ever */
        if(!_first) _first = item;

        else {
            _last->_next = item;
            item->_previous = _last;
        }

        _last = item;

    /* Adding as first item */
    } else if(!before->_previous) {
        item->_next = _first;
        _first->_previous = item;
        _first = item;

    /* Adding in the middle */
    } else {
        item->_previous = before->_previous;
        item->_next = before;
        before->_previous->_next = item;
        before->_previous = item;
    }
}

template<class T> void LinkedList<T>::cut(T* const item) {
    CORRADE_ASSERT(item->_list == this, "Containers::LinkedList::cut(): cannot cut out an item which is not a part of the list", );

    /* Removing first item */
    if(item == _first) {
        _first = _first->_next;
        if(_first) _first->_previous = nullptr;

        /* The item is last remaining in the list */
        if(item == _last)
            _last = nullptr;

    /* Removing last item */
    } else if(item == _last) {
        _last = _last->_previous;
        if(_last) _last->_next = nullptr;

    /* Removing item in the middle */
    } else {
        item->_previous->_next = item->_next;
        item->_next->_previous = item->_previous;
    }

    item->_list = nullptr;
    item->_previous = nullptr;
    item->_next = nullptr;
}

template<class T> inline void LinkedList<T>::move(T* const item, T* const before) {
    if(item == before) return;
    cut(item);
    insert(item, before);
}

template<class T> inline void LinkedList<T>::erase(T* const item) {
    CORRADE_ASSERT(item->_list == this, "Containers::LinkedList::erase(): cannot erase an item which is not a part of the list", );
    item->erase();
}

template<class T> void LinkedList<T>::clear() {
    /** @todo Make this simpler --- just deletion of all the items w/o any reconnecting */
    T* i = _first;
    while(i) {
        T* next = i->_next;
        erase(i);
        i = next;
    }
}

template<class Derived, class List> LinkedListItem<Derived, List>::LinkedListItem(LinkedListItem<Derived, List>&& other): _list(nullptr), _previous(nullptr), _next(nullptr) {
    /* Replace other with self in the list */
    if(other._list) {
        other._list->LinkedList<Derived>::insert(static_cast<Derived*>(this), other._next);
        other._list->LinkedList<Derived>::cut(static_cast<Derived*>(&other));
    }
}

template<class Derived, class List> LinkedListItem<Derived, List>::~LinkedListItem() {
    if(_list) _list->LinkedList<Derived>::cut(static_cast<Derived*>(this));
}

template<class Derived, class List> LinkedListItem<Derived, List>& LinkedListItem<Derived, List>::operator=(LinkedListItem<Derived, List>&& other) {
    /* Cut self from previous list */
    if(_list) _list->LinkedList<Derived>::cut(static_cast<Derived*>(this));

    /* Replace other with self in new list */
    if(other._list) {
        other._list->LinkedList<Derived>::insert(static_cast<Derived*>(this), other._next);
        other._list->LinkedList<Derived>::cut(static_cast<Derived*>(&other));
    }

    return *this;
}

template<class Derived, class List> void LinkedListItem<Derived, List>::erase() {
    doErase();
}

template<class Derived, class List> void LinkedListItem<Derived, List>::doErase() {
    _list->LinkedList<Derived>::cut(static_cast<Derived*>(this));
    delete this;
}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class T> class LinkedList<T>::Iterator {
    public:
        constexpr /*implicit*/ Iterator(T* item): _item{item} {}

        constexpr T& operator*() const { return *_item; }
        constexpr bool operator!=(const Iterator& other) const { return _item != other._item; }

        Iterator& operator++() {
            _item = _item->_next;
            return *this;
        }

    private:
        T* _item;
};

template<class T> class LinkedList<T>::ConstIterator {
    public:
        constexpr /*implicit*/ ConstIterator(const T* item): _item{item} {}

        constexpr const T& operator*() const { return *_item; }
        constexpr bool operator!=(const ConstIterator& other) const { return _item != other._item; }

        ConstIterator& operator++() {
            _item = _item->_next;
            return *this;
        }

    private:
        const T* _item;
};

template<class T> typename LinkedList<T>::Iterator begin(LinkedList<T>& list) { return list.first(); }
template<class T> constexpr typename LinkedList<T>::ConstIterator begin(const LinkedList<T>& list) { return list.first(); }

template<class T> typename LinkedList<T>::Iterator end(LinkedList<T>&) { return nullptr; }
template<class T> constexpr typename LinkedList<T>::ConstIterator end(const LinkedList<T>&) { return nullptr; }
#endif

}}

#endif
