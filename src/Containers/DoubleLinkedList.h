#ifndef Corrade_Containers_DoubleLinkedList_h
#define Corrade_Containers_DoubleLinkedList_h
/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Class Corrade::Containers::DoubleLinkedList
 */

#include "Utility/Debug.h"

namespace Corrade { namespace Containers {

/**
@brief Double linked list

The list stores pointers to items which contain iterators in itself, not the
other way around, so it is possible to operate directly with the items without
any abstraction at *constant* time. The only downside of this is that the
items or list cannot be copied (but they can be moved).

@note For simplicity and memory usage reasons the list doesn't provide any
method to get count of stored items, but you can traverse them and count them
manually if desperately needed.

Example:
@code
class Object: public DoubleLinkedListItem<Object> {
    // ...
};

Object a, b, c;

DoubleLinkedList<Object> list;
list.insert(&a);
list.insert(&b);
list.insert(&c);

list.cut(&b);
@endcode

Traversing through the list is done like following:
@code
for(Object* i = list.first(); i; i = i->next()) {
    // ...
}
@endcode
*/
template<class T> class DoubleLinkedList {
    DoubleLinkedList(const DoubleLinkedList<T>& other) = delete;
    DoubleLinkedList<T>& operator=(const DoubleLinkedList<T>& other) = delete;

    public:
        /**
         * @brief Default constructor
         *
         * Creates empty list.
         */
        inline constexpr DoubleLinkedList(): _first(nullptr), _last(nullptr) {}

        /** @brief Move constructor */
        DoubleLinkedList(DoubleLinkedList<T>&& other): _first(other._first), _last(other._last) {
            other._first = nullptr;
            other._last = nullptr;

            /* Backreference this list from the items */
            for(T* i = _first; i; i = i->_next)
                i->_list = static_cast<decltype(i->_list)>(this);
        }

        /**
         * @brief Destructor
         *
         * Clears the list.
         */
        inline ~DoubleLinkedList() { clear(); }

        /** @brief Move assignment */
        DoubleLinkedList<T>& operator=(DoubleLinkedList<T>&& other) {
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

        /** @brief First item or `nullptr`, if the list is empty */
        inline constexpr T* first() const { return _first; }

        /** @brief Last item or `nullptr`, if the list is empty */
        inline constexpr T* last() const { return _last; }

        /** @brief Whether the list is empty */
        inline constexpr bool isEmpty() const { return !_first; }

        /**
         * @brief Insert item
         * @param item      Item to insert
         * @param before    Item before which to insert or `nullptr`, if
         *      inserting at the end.
         *
         * @attention The item must not be connected to any list.
         */
        void insert(T* item, T* before = nullptr) {
            CORRADE_ASSERT(!item->list(), "Containers::DoubleLinkedList: Cannot insert item already connected elsewhere.", );
            CORRADE_ASSERT(!before || before->list() == this, "Containers::DoubleLinkedList: Cannot insert before item which is not part of the list.", );

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

        /**
         * @brief Cut item out
         * @param item      Item to cut out
         *
         * The item is disconnected from the list, but not deleted.
         */
        void cut(T* item) {
            CORRADE_ASSERT(item->list() == this, "Containers::DoubleLinkedList: Cannot cut out item which is not part of the list.", );

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

        /**
         * @brief Move item before another
         * @param item      Item to move
         * @param before    Item before which to move or `nullptr`, if moving
         *      at the end.
         *
         * Equivalent to:
         * @code
         * list.cut(item);
         * list.move(item, before);
         * @endcode
         */
        inline void move(T* item, T* before) {
            cut(item);
            insert(item, before);
        }

        /**
         * @brief Erase item
         * @param item      Item to erase
         *
         * Equivalent to:
         * @code
         * list.cut(item);
         * delete item;
         * @endcode
         */
        inline void erase(T* item) {
            cut(item);
            delete item;
        }

        /** @brief Clear the list */
        void clear() {
            T* i = _first;
            while(i) {
                T* next = i->_next;
                erase(i);
                i = next;
            }
        }

    private:
        T *_first, *_last;
};

/** @brief Item of double-linked list. */
template<class T, class List = DoubleLinkedList<T>> class DoubleLinkedListItem {
    friend class DoubleLinkedList<T>;

    DoubleLinkedListItem(const DoubleLinkedListItem<T, List>& other) = delete;
    DoubleLinkedListItem& operator=(const DoubleLinkedListItem<T, List>& other) = delete;

    public:
        /**
         * @brief Default constructor
         *
         * Creates item not connected to any list.
         */
        inline DoubleLinkedListItem(): _list(nullptr), _previous(nullptr), _next(nullptr) {}

        /** @brief Move constructor */
        DoubleLinkedListItem(DoubleLinkedListItem<T, List>&& other): _list(nullptr), _previous(nullptr), _next(nullptr) {
            /* Replace other with self in the list */
            if(other._list) {
                other._list->insert(static_cast<T*>(this), other._next);
                other._list->cut(static_cast<T*>(&other));
            }
        }

        /** @brief Move assignment */
        DoubleLinkedListItem<T, List>& operator=(DoubleLinkedListItem<T, List>&& other) {
            /* Cut self from previous list */
            if(_list) _list->cut(static_cast<T*>(this));

            /* Replace other with self in new list */
            if(other._list) {
                other._list->insert(static_cast<T*>(this), other._next);
                other._list->cut(static_cast<T*>(&other));
            }

            return *this;
        }

        /**
         * @brief Destructor
         *
         * If the item is part of any list, it is removed from it.
         */
        virtual ~DoubleLinkedListItem() = 0;

        /** @brief List this item belongs to */
        inline List* list() const { return _list; }

        /** @brief Previous item or `nullptr`, if there is no previous item */
        inline T* previous() const { return _previous; }

        /** @brief Next item or `nullptr`, if there is no previous item */
        inline T* next() const { return _next; }

    private:
        List* _list;
        T *_previous, *_next;
};

template<class T, class List> inline DoubleLinkedListItem<T, List>::~DoubleLinkedListItem() {
    if(_list) _list->cut(static_cast<T*>(this));
}

}}

#endif
