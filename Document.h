#pragma once
#ifndef DOCUMENT_H_INCLUDED
#define DOCUMENT_H_INCLUDED

#include <stdio.h>
#include <assert.h>

#include "Error.h"

#include "String.h"
#include "Fragment.h"
#include "Block.h"
typedef struct Document_tag {
    char* title;                // pointer to a title of file
    String* text;               // pointer to a text (main string)
    ListBlock* blocks;          // pointer to blocks. It stores the structure of the text
} Document;

/**
 * Creates a document object.
 * IN:
 * @param filename - pointer to a file name
 * 
 * OUT:
 * @return doc - pointer to a document object containing information from a file
 */
Document* CreateDocument(char const* filename);

/**
 * Destroys a document object.
 * IN:
 * @param ppDoc - pointer to pointer to a document object
 * 
 * OUT:
 * *ppDoc - filled with NULL value
 */
void DestroyDocument(Document** ppDoc);

/**
 * Sets a file in document object.
 * IN:
 * @param doc - pointer to a document object
 * @param filename - pointer to a file name. if file is not exist, then put NULL.
 * 
 * OUT:
 * @return errValue - value indicating the success of the operation
 */
int SetFile(Document* doc, char const* filename);

/**
 * Gets max length of a block in text.
 * IN:
 * @param blocks - pointer to blocks
 *
 * OUT:
 * @return maxLen - max length of a block
 */
size_t GetMaxBlockLen(ListBlock const* blocks);

// for debugging ===============================================
    /**
     * Prints text of document object.
     * IN:
     * @param output - pointer to a stream
     * @param doc - pointer to a document object
     * 
     */
    size_t PrintDocument(FILE* output, Document const* doc);

    /**
     * Prints parameters of document object.
     * IN:
     * @param output - pointer to a stream
     * @param doc - pointer to a document object
     * 
     */
    void PrintDocumentParameters(FILE* output, Document const* doc);
// =============================================================

#endif // DOCUMENT_H_INCLUDED
