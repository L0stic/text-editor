#include "Block.h"

CREATE_NODE(Block, BlockData_t) {
    Block* node = malloc(sizeof(Block));

    if (!node) { return NULL; }

    node->prev = prev;
    node->next = NULL;
    node->data = data;

    return node;
}

DESTROY_NODE(Block) {
    assert(node && *node);
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
