#ifndef Corrade_Containers_LinkedList_h
#define Corrade_Containers_LinkedList_h
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
 * @brief Class Corrade::Containers::LinkedList, Corrade::Containers::LinkedListItem
 */

#include "Utility/Debug.h"

namespace Corrade { namespace Containers {

/**
@brief Linked list
@tparam T   Item type, derived from LinkedListItem

The list stores pointers to items which contain iterators in itself, not the
other way around, so it is possible to operate directly with pointers to the
items without any abstraction at *constant* time. The only downside of this is
that the items or list cannot be copied (but they can be moved).

@note For simplicity and memory usage reasons the list doesn't provide any
method to get count of stored items, but you can traverse them and count them
manually if desperately needed.

Example:
@code
class Object: public LinkedListItem<Object> {
    // ...
};

Object a, b, c;

LinkedList<Object> list;
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

@see LinkedListItem
*/
template<class T> class LinkedList {
    LinkedList(const LinkedList<T>& other) = delete;
    LinkedList<T>& operator=(const LinkedList<T>& other) = delete;

    public:
        /**
         * @brief Default constructor
         *
         * Creates empty list.
         */
        inline constexpr LinkedList(): _first(nullptr), _last(nullptr) {}

        /** @brief Move constructor */
        LinkedList(LinkedList<T>&& other): _first(other._first), _last(other._last) {
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
        inline ~LinkedList() { clear(); }

        /** @brief Move assignment */
        LinkedList<T>& operator=(LinkedList<T>&& other) {
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
        inline T* first() { return _first; }
        inline constexpr const T* first() const { return _first; } /**< @overload */

        /** @brief Last item or `nullptr`, if the list is empty */
        inline T* last() { return _last; }
        inline constexpr const T* last() const { return _last; } /**< @overload */

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
            CORRADE_ASSERT(!(item->_list), "Containers::LinkedList: Cannot insert item already connected elsewhere.", );
            CORRADE_ASSERT(!before || before->_list == this, "Containers::LinkedList: Cannot insert before item which is not part of the list.", );

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
            CORRADE_ASSERT(item->_list == this, "Containers::LinkedList: Cannot cut out item which is not part of the list.", );

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

/**
@brief Item of LinkedList
@param Derived  Dervied object type, i.e. type you want returned from previous() and next().
@param List     List object type, i.e. type you want returned from list().

This class is usually subclassed using [CRTP](http://en.wikipedia.org/wiki/Curiously_Recurring_Template_Pattern),
e.g.:
@code
class Item: public LinkedListItem<Item> {
    // ...
};
@endcode
*/
template<class Derived, class List = LinkedList<Derived>> class LinkedListItem {
    friend class LinkedList<Derived>;

    LinkedListItem(const LinkedListItem<Derived, List>& other) = delete;
    LinkedListItem& operator=(const LinkedListItem<Derived, List>& other) = delete;

    public:
        /**
         * @brief Default constructor
         *
         * Creates item not connected to any list.
         */
        inline LinkedListItem(): _list(nullptr), _previous(nullptr), _next(nullptr) {}

        /** @brief Move constructor */
        LinkedListItem(LinkedListItem<Derived, List>&& other): _list(nullptr), _previous(nullptr), _next(nullptr) {
            /* Replace other with self in the list */
            if(other._list) {
                other._list->insert(static_cast<Derived*>(this), other._next);
                other._list->cut(static_cast<Derived*>(&other));
            }
        }

        /** @brief Move assignment */
        LinkedListItem<Derived, List>& operator=(LinkedListItem<Derived, List>&& other) {
            /* Cut self from previous list */
            if(_list) _list->cut(static_cast<Derived*>(this));

            /* Replace other with self in new list */
            if(other._list) {
                other._list->insert(static_cast<Derived*>(this), other._next);
                other._list->cut(static_cast<Derived*>(&other));
            }

            return *this;
        }

        /**
         * @brief Destructor
         *
         * If the item is part of any list, it is removed from it.
         */
        virtual ~LinkedListItem() = 0;

        /** @brief List this item belongs to */
        inline List* list() { return _list; }
        inline const List* list() const { return _list; } /**< @overload */

        /** @brief Previous item or `nullptr`, if there is no previous item */
        inline Derived* previous() { return _previous; }
        inline const Derived* previous() const { return _previous; } /**< @overload */

        /** @brief Next item or `nullptr`, if there is no previous item */
        inline Derived* next() { return _next; }
        inline const Derived* next() const { return _next; } /**< @overload */

    private:
        List* _list;
        Derived *_previous, *_next;
};

template<class Derived, class List> inline LinkedListItem<Derived, List>::~LinkedListItem() {
    if(_list) _list->LinkedList<Derived>::cut(static_cast<Derived*>(this));
}

}}

#endif
