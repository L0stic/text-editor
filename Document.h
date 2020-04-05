#ifndef DOCUMENT_H_INCLUDED
#define DOCUMENT_H_INCLUDED

#include <stdio.h>
#include <assert.h>

#include "String.h"

#define UNTITLED_TITLE "Untitled"

// typedef struct TextModel_tag {
//     size_t size;
//     size_t* blocks
// } TextModel;

typedef struct Document_tag {
    char* title;
    String* text;
    // TextModel* textModel;
} Document;

Document* CreateDocument(const char* filename);
void DestroyDocument(Document** ppDoc);

int SetFile(Document* doc, const char* filename);
int SetUntitledFile(Document* doc);

#endif // DOCUMENT_H_INCLUDED
