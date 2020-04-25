#ifndef DOCUMENT_H_INCLUDED
#define DOCUMENT_H_INCLUDED

#include <stdio.h>
#include <assert.h>

#include "String.h"
#include "Block.h"

#define UNTITLED_TITLE "Untitled"

typedef struct Document_tag {
    char* title;
    String* text;
    ListBlock* blocks;
} Document;

Document* CreateDocument(const char* filename);
void DestroyDocument(Document** ppDoc);

int SetFile(Document* doc, const char* filename);
int SetUntitledFile(Document* doc);

size_t PrintDocument(FILE* output, const Document* doc);
void PrintDocumentParameters(FILE* output, const Document* doc);

#endif // DOCUMENT_H_INCLUDED
