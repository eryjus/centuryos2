//===================================================================================================================
//
//  lists.h -- Standard list for the entire kernel
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The list structures in this file are standard for all lists in the entire Century-OS implementaiton.  All lists
//  will make use of these structures.
//
//  The inspiration for this list structure and implementation is taken from the Linux list implementation.
//  References can be found in the Linux Kernel Development book, chapter 6 and the linux source file at
//  http://www.cs.fsu.edu/~baker/devices/lxr/http/source/linux/include/linux/list.h
//
//  In short, the list implementation is a circular doubly linked list.  As such there is no specific head and tail.
//
// ------------------------------------------------------------------------------------------------------------------
//
//  IMPORTANT PROGRAMMING NOTE:
//  These functions are not atomic.  This means that all calling functions will also need to mantain locks on the
//  structures and lists before maintaining the list.  Put another way, the caller is required to ensure that
//  nothing else changes the list while these functions are being executed.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Feb-14  Initial  v0.0.4   ADCL  Initial version (leveraged)
//
//===================================================================================================================


#pragma once


#include "types.h"


//
// -- This macro determines the offset of a member of a structure.  The 'magic' of this macro is the calculation
//    of an address as a offset from the address 0x00000000.
//    ----------------------------------------------------------------------------------------------------------
#define MEMBER_OFFSET(type,member)  ((Addr_t)(&((type *)0x00000000)->member))


//
// -- This macro determines the address of the parent of a member structure.
//    Usage: FIND_PARENT(list->next, Process, global);
//    ----------------------------------------------------------------------
#define FIND_PARENT(ptr,type,member) ({                                    \
        const typeof(((type *)0x00000000)->member) *__mptr = (ptr);        \
        (type *)((char *)__mptr - MEMBER_OFFSET(type,member));  })


//
// -- This is the header of the list.
//    -------------------------------
typedef struct ListHead_t {
    typedef struct List_t {
        struct List_t *prev;
        struct List_t *next;
    } List_t;

    List_t list;
    Spinlock_t lock;            // -- this or a "bigger" lock must be obtained to change the list contents
    size_t count;               // -- this is available for use by software; not used by `lists.h`
} ListHead_t;


//
// -- Declare and initialize a new List not in a structure
//    ----------------------------------------------------
#define NEW_LIST(name) ListHead_t name = { { &(name.list), &(name.list) }, {0, 0}, 0 };


//
// -- Initialize a list to point to itself
//    ------------------------------------
inline void ListInit(ListHead_t::List_t * const list) { list->next = list->prev = list; }


//
// -- Low-level function to add a node to a list
//    ------------------------------------------
inline void __list_add(ListHead_t::List_t * const nw, ListHead_t::List_t * const pv, ListHead_t::List_t * const nx) {
    nx->prev = nw; nw->next = nx; nw->prev = pv; pv->next = nw;
}


//
// -- Low-level function to delete a node from a list
//    -----------------------------------------------
inline void __list_del(ListHead_t::List_t * const pv, ListHead_t::List_t * const nx) {
    nx->prev = pv; pv->next = nx;
}


//
// -- Add a new node to a list (which is right ahead of the head)
//    -----------------------------------------------------------
inline void ListAdd(ListHead_t * const head, ListHead_t::List_t * const nw) {
    __list_add(nw, &head->list, head->list.next);
}


//
// -- Add a new node to a list (which will be right behind the tail)
//    --------------------------------------------------------------
inline void ListAddTail(ListHead_t * const head, ListHead_t::List_t * const nw) {
    __list_add(nw, head->list.prev, &head->list);
}


//
// -- Delete a node from a list (and clear the node's pointers to NULL)
//    -----------------------------------------------------------------
inline void ListRemove(ListHead_t::List_t * const entry) {
    __list_del(entry->prev, entry->next); entry->next = entry->prev = 0;
}


//
// -- Delete a node from a list (and and initialize the node to be properly empty)
//    ----------------------------------------------------------------------------
inline void ListRemoveInit(ListHead_t::List_t * const entry) {
    __list_del(entry->prev, entry->next); ListInit(entry);
}


//
// -- Is this list empty or not?  Notice that both the address and the contents are constant
//    --------------------------------------------------------------------------------------
inline bool IsListEmpty(const ListHead_t * const head) {
    return (head->list.next == &head->list);
}


//
// -- Is this entry last in the list?  Notice that both the address and the contents are constant
//    -------------------------------------------------------------------------------------------
inline bool IsLastInList(const ListHead_t * const head, const ListHead_t::List_t * const entry) {
    return entry->next == &head->list;
}


//
// -- Move an entry from one list to another (in front of the head)
//    -------------------------------------------------------------
inline void ListMove(ListHead_t * const head, ListHead_t::List_t * const entry) {
    __list_del(entry->prev, entry->next); ListAdd(head, entry);
}


//
// -- Move an entry from one list to another (after the tail)
//    -------------------------------------------------------
inline void ListMoveTail(ListHead_t * const head, ListHead_t::List_t * const entry) {
    __list_del(entry->prev, entry->next); ListAddTail(head, entry);
}


//
// -- Count the number of items in the list
//    -------------------------------------
inline int ListCount(ListHead_t *const head) {
    int rv = 0;
    ListHead_t::List_t *wrk = head->list.next;
    while (wrk != &head->list) {
        rv ++;
        wrk = wrk->next;
    }
    return rv;
}


//
// -- This is a queue; the next thing to operate on is at head
//    --------------------------------------------------------
typedef ListHead_t QueueHead_t;


//
// -- Enqueue a node onto a queue
//    ---------------------------
inline void Enqueue(QueueHead_t *head, ListHead_t::List_t *list) { ListAddTail(head, list); }


//
// -- This is a stack; the next thing to operate on is at head
//    --------------------------------------------------------
typedef ListHead_t StackHead_t;


//
// -- Push a node onto a stack
//    ------------------------
inline void Push(StackHead_t *head, ListHead_t::List_t *list) { ListAdd(head, list); }

