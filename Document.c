#include "Document.h"

static void SetTitle(Document* doc, char** title) {
    assert(doc && title && *title);

    if (doc->title) { free(doc->title); }
    doc->title = *title;
    *title = NULL;
}

static void SetText(Document* doc, String** text) {
    assert(doc && text && *text);

    if (doc->text) { DestroyString(&(doc->text)); }
    doc->text = *text;
    *text = NULL;
}

static void SetBlocks(Document* doc, ListBlock** blocks) {
    assert(doc && blocks && *blocks);

    if (doc->blocks) { DestroyListBlock(&(doc->blocks)); }
    doc->blocks = *blocks;
    *blocks = NULL;
}

static char* GetUntitledTittle() {
    const char* title = UNTITLED_TITLE;
    char* tmp = malloc((strlen(title) + 1) * sizeof(char));

    if (tmp) { strcpy(tmp, title); }

    return tmp;
}

int SetUntitledFile(Document* doc) {
    assert(doc);

    String* text = CreateString(NULL);
    if (!text) { return -1; }

    if (ReserveSize(text, BASE_STRING_SIZE)) {
        DestroyString(&text);
        return -2;
    }

    char* title = GetUntitledTittle();
    if (!title) {
        DestroyString(&text);
        return -3;
    }

    SetText(doc, &text);
    SetTitle(doc, &title);
    return 0;
}

static char* GetTitle(const char* filename) {
    assert(filename);

    char* titleStart = (char*)filename + strlen(filename);
    while((titleStart > filename) && (*titleStart != '\\')) { --titleStart; }

    char* title = malloc(sizeof(char) * (strlen(titleStart) + 1));
    if (title) { strcpy(title, titleStart); }

    return title;
}

static long GetFileSize(FILE* file) {
    assert(file);

    long fileSize;
    long lastSeek = ftell(file);

    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, lastSeek, SEEK_SET);

    return fileSize;
}

int SetFile(Document* doc, const char* filename) {
    assert(doc);

    FILE* file = fopen(filename, "r");
    if (!file) { return -1; }

    long size = GetFileSize(file);
    if (size < 0) {
        fclose(file);
        return -2;
    }

    ListBlock* blocks = CreateListBlock();
    if (!blocks) {
        fclose(file);
        return -3;
    }

    String* text = CreateString(NULL);
    if (!text) {
        DestroyListBlock(&blocks);
        fclose(file);
        return -3;
    }

    if (ReserveSize(text, (size_t)size)) {
        DestroyListBlock(&blocks);
        DestroyString(&text);
        fclose(file);
        return -4;
    }

    // Scaning
    char* buffer = malloc((BASE_STRING_SIZE + 1) * sizeof(char));
    if (!buffer) {
        DestroyListBlock(&blocks);
        DestroyString(&text);
        fclose(file);
        return -5;
    }

    // TODO: add error-handler
    for (size_t blockLen = 0; !feof(file); ) {
        if (fgets(buffer, BASE_STRING_SIZE + 1, file)) {
            size_t len = strlen(buffer);

            blockLen += len;

            if (len > 0 && buffer[len - 1] == '\n') {
                size_t pos = blocks->last ? blocks->last->data.pos + blocks->last->data.len : 0;
                BlockData_t blockData = {blockLen - 1, pos, NULL};

                if (AddBlockData(blocks, blockData)) {
                    DestroyListBlock(&blocks);
                    DestroyString(&text);
                    free(buffer);
                    fclose(file);
                    return -6;
                }

                blockLen = 0;
                buffer[len - 1] = '\0';
            }

            if (AddString(text, buffer) < 0) {
                DestroyListBlock(&blocks);
                DestroyString(&text);
                free(buffer);
                fclose(file);
                return -7;
            }
        }
    }
    free(buffer);
    fclose(file);

    char* title = GetTitle(filename);
    if (!title) {
        DestroyListBlock(&blocks);
        DestroyString(&text);
        return -8;
    }

    SetTitle(doc, &title);
    SetText(doc, &text);
    SetBlocks(doc, &blocks);
    return 0;
}

Document* CreateDocument(const char* filename) {
    Document* doc = calloc(1, sizeof(Document));

    if (!doc) { return NULL; }

    if (filename) {
        SetFile(doc, filename);
    } else {
        SetUntitledFile(doc);
    }

    return doc;
}

void DestroyDocument(Document** ppDoc) {
    assert(ppDoc && *ppDoc);

    Document* pDoc = *ppDoc;
    if (pDoc->title) { free(pDoc->title); }
    if (pDoc->text) { DestroyString(&(pDoc->text)); }
    if (pDoc->blocks) { DestroyListBlock(&(pDoc->blocks)); }

    free(pDoc);
    *ppDoc = NULL;
}

size_t PrintDocument(FILE* output, const Document* doc) {
    assert(doc);
    size_t counter = 0;

    if (!output) { output = stdout; }

    for (Block* block = doc->blocks->nodes; block; block = block->next) {
        for (size_t i = 0; i < block->data.len; ++i) {
            fputc(doc->text->data[block->data.pos + i], output);
            ++counter;
        }
        fputc('\n', output);
        ++counter;
    }

    return counter;
}

void PrintDocumentParameters(FILE* output, const Document* doc) {
    assert(doc);
    const char* null = "NULL";

    if (!output) { output = stdout; }

    fprintf(output, "Document's parameters:\n");
    
    fprintf(output, "\tTitle: %s\n", doc->title ? doc->title : null);
    
    fprintf(output, "\tText: "); 
    if (doc->text) {
        fprintf(output, "len = %u, size = %u\n", doc->text->len, doc->text->size);
    } else {
        fprintf(output, "%s\n", null);
    }

    fprintf(output, "\tBlocks: "); 
    if (doc->text) {
        fprintf(output, "len = %u\n", doc->blocks->len);
    } else {
        fprintf(output, "%s\n", null);
    }
}
