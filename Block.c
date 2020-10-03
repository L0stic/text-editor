#include "Block.h"

CREATE_NODE(Block, BlockData_t) {
    Block* node = malloc(sizeof(Block));

    if (!node) { return NULL; }

    node->prev = prev;
    node->next = NULL;
    node->data = *data;

    return node;
}

DESTROY_NODE(Block) {
    assert(node && *node);

    if ((*node)->data.fragments) {
        DestroyListFragment(&(*node)->data.fragments);
    }
    free(*node);
    *node = NULL;
}

CREATE_LIST(Block) {
    ListBlock* list = calloc(1, sizeof(ListBlock));

    if (!list) { return NULL; }

    return list;
}

DESTROY_LIST(Block) {
    assert(list && *list);

    Block* node = (*list)->nodes;
    while(node) {
        Block* nextNode = node->next;
        
        DestroyBlock(&node);
        node = nextNode;
    }

    free(*list);
    *list = NULL;
}

ADD_DATA(Block, BlockData_t) {
    assert(list);

    Block* prev = list->last;
    Block* node = CreateBlock(prev, data);

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

INSERT_NODES(Block) {
    assert(list && node);
    size_t count = 1;
    Block* lastNode = node;

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

DELETE_NODE(Block) {
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

    DestroyBlock(&node);

    --list->len;
}
