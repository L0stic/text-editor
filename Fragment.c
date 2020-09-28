#include "Fragment.h"

CREATE_NODE(Fragment, FragmentData_t) {
    Fragment* node = malloc(sizeof(Fragment));

    if (!node) { return NULL; }

    node->prev = prev;
    node->next = NULL;
    node->data = *data;

    return node;
}

DESTROY_NODE(Fragment) {
    assert(node && *node);
    free(*node);
    *node = NULL;
}

CREATE_LIST(Fragment) {
    ListFragment* list = calloc(1, sizeof(ListFragment));

    if (!list) { return NULL; }

    return list;
}

DESTROY_LIST(Fragment) {
    assert(list && *list);

    Fragment* node = (*list)->nodes;
    while(node) {
        Fragment* nextNode = node->next;
        
        DestroyFragment(&node);
        node = nextNode;
    }

    free(*list);
    *list = NULL;
}

ADD_DATA(Fragment, FragmentData_t) {
    assert(list);

    Fragment* prev = list->last;
    Fragment* node = CreateFragment(prev, data);

    if (!node) { return -1; }

    if (prev) {
        prev->next = node;
    } else {
        list->nodes = node;
    }

    list->last = node;
    list->len += 1;
    return 0;
}

INSERT_NODE(Fragment) {
    assert(list && node);

    if (node->prev) {
        node->next = node->prev->next;
        node->prev->next = node;
    } else {
        node->next = list->nodes;
        list->nodes = node;
    }

    if (!node->next) {
        list->last = node;
    }

    ++list->len;
}
