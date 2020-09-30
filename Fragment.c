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

INSERT_NODES(Fragment) {
    assert(list && node);
    size_t count = 0;
    Fragment* lastNode = node;

    // pass
    while (lastNode->next) {
        lastNode = lastNode->next;
        ++count;
    }

    if (node->prev) {
        lastNode->next = node->prev->next;
        node->prev->next = node;
    } else {
        lastNode->next = list->nodes;
        list->nodes = node;
    }

    if (lastNode->next) {
        lastNode->next->prev = lastNode;
    } else {
        list->last = lastNode;
    }

    list->len += count;
}

DELETE_NODE(Fragment) {
    assert(list && node);
    assert(list->len);

    if (node->prev) {
        node->prev->next = node->next;
    } else {
        list->nodes = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    } else {
        list->last = node->prev;
    }

    DestroyFragment(&node);

    --list->len;
}
