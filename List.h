#pragma once
#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#define NODE(T, D) \
    typedef struct T##_tag {    \
        struct T##_tag* prev;   /* pointer to previous node */  \
        struct T##_tag* next;   /* pointer to next node */      \
        D data;                 /* data of a node */  \
    } T;                        \
// END_NODE

#define LIST(T) \
    typedef struct List##T##_tag {  \
        size_t len;     /* length of list */        \
        T* nodes;       /* pointer to start node */ \
        T* last;        /* pointer to last node */  \
    } List##T;                      \
// END_LIST

#define CREATE_NODE(T, D)   T* Create##T(T* prev, const D* data)
#define DESTROY_NODE(T)     void Destroy##T(T** node)
#define CREATE_LIST(T)      List##T* CreateList##T()
#define DESTROY_LIST(T)     void DestroyList##T(List##T** list)
#define ADD_DATA(T, D)      int Add##T##Data(List##T* list, const D* data)
#define INSERT_NODES(T)     void Insert##T##s(List##T* list, T* node)
#define DELETE_NODE(T)      void Delete##T(List##T* list, T* node)

#define LIST_TEMPLATE(T, D) \
    NODE(T, D)          \
    LIST(T)             \
    \
    CREATE_NODE(T, D);  \
    DESTROY_NODE(T);    \
    \
    CREATE_LIST(T);     \
    DESTROY_LIST(T);    \
    \
    ADD_DATA(T, D);     \
    INSERT_NODES(T);    \
    DELETE_NODE(T);     \
// END_LIST_TEMPLATE

#endif // LIST_H_INCLUDED
