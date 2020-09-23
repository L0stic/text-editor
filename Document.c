#include "Document.h"

#define UNTITLED_TITLE "Untitled"

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

static char* GetUntitledTitle() {
    const char* title = UNTITLED_TITLE;
    char* tmp = malloc((strlen(title) + 1) * sizeof(char));

    if (tmp) { strcpy(tmp, title); }

    return tmp;
}

static char* GetTitle(const char* filename) {
    assert(filename);

    char* title;
    char* titleStart = (char*)filename + strlen(filename);
    while((titleStart > filename) && (*(titleStart - 1) != '\\')) { --titleStart; }

    title = malloc(sizeof(char) * (strlen(titleStart) + 1));
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

static int InsertBlock(ListBlock* blocks, size_t blockLen) {
    assert(blocks);

    size_t pos = blocks->last ? blocks->last->data.pos + blocks->last->data.len : 0;
    BlockData_t blockData = {blockLen, pos, NULL};

    return AddBlockData(blocks, blockData);
}

static int ScanFile(FILE* file, ListBlock* blocks, String* text) {
    assert(file && blocks && text);

    char* buffer = malloc((BASE_STRING_SIZE + 1) * sizeof(char));

    if (!buffer) {
        PrintError(NULL, ERR_NOMEM, __FILE__, __LINE__);
        return ERR_NOMEM;
    }

    size_t blockLen = 0;
    while (!feof(file)) {
        if (fgets(buffer, BASE_STRING_SIZE + 1, file)) {
            size_t len = strlen(buffer);

            blockLen += len;

            if (len > 0 && buffer[len - 1] == '\n') {
                if (InsertBlock(blocks, blockLen - 1)) {
                    free(buffer);
                    PrintError(NULL, ERR_NOMEM, __FILE__, __LINE__);
                    return ERR_NOMEM;
                }

                blockLen = 0;
                buffer[len - 1] = '\0';
            }

            if (AddString(text, buffer) < 0) {
                free(buffer);
                PrintError(NULL, ERR_NOMEM, __FILE__, __LINE__);
                return ERR_NOMEM;
            }
        }
    }
    free(buffer);

    // if (blockLen > 0 && InsertBlock(blocks, blockLen)) {
    if (InsertBlock(blocks, blockLen)) {
        PrintError(NULL, ERR_NOMEM, __FILE__, __LINE__);
        return ERR_NOMEM;
    }

    return ERR_SUCCESS;
}

int SetFile(Document* doc, char const* filename) {
    assert(doc);

    ListBlock* blocks;
    String* text;
    FILE* file;
    long size;
    char* title;
    
    if (filename) {
        file = fopen(filename, "r");
        if (!file) {
            PrintError(NULL, ERR_OPEN_FILE, __FILE__, __LINE__);
            filename = NULL;
        } else {
            size = GetFileSize(file);
            if (size < 0) {
                fclose(file);
                PrintError(NULL, ERR_READ, __FILE__, __LINE__);
                return ERR_READ;
            } else if (!size) {
                size = BASE_STRING_SIZE;
            }
        }
    }

    blocks = CreateListBlock();
    if (!blocks) {
        if (filename) { fclose(file); }
        PrintError(NULL, ERR_NOMEM, __FILE__, __LINE__);
        return ERR_NOMEM;
    }

    text = CreateString(NULL);
    if (!text) {
        DestroyListBlock(&blocks);
        if (filename) { fclose(file); }
        PrintError(NULL, ERR_NOMEM, __FILE__, __LINE__);
        return ERR_NOMEM;
    }

    if (ReserveSize(text, filename ? (size_t)size : BASE_STRING_SIZE)) {
        DestroyListBlock(&blocks);
        DestroyString(&text);
        if (filename) { fclose(file); }
        PrintError(NULL, ERR_NOMEM, __FILE__, __LINE__);
        return ERR_NOMEM;
    }

    if (filename) {
        if (ScanFile(file, blocks, text)) {
            DestroyListBlock(&blocks);
            DestroyString(&text);
            fclose(file);
            PrintError(NULL, ERR_NOMEM, __FILE__, __LINE__);
            return ERR_NOMEM;
        }
        fclose(file);
    } else if (InsertBlock(blocks, 0)) {
        DestroyListBlock(&blocks);
        DestroyString(&text);
        PrintError(NULL, ERR_NOMEM, __FILE__, __LINE__);
        return ERR_NOMEM;
    }

    title = filename ? GetTitle(filename) : GetUntitledTitle();
    if (!title) {
        DestroyListBlock(&blocks);
        DestroyString(&text);
        PrintError(NULL, ERR_NOMEM, __FILE__, __LINE__);
        return ERR_NOMEM;
    }

    SetTitle(doc, &title);
    SetText(doc, &text);
    SetBlocks(doc, &blocks);
    return ERR_SUCCESS;
}

Document* CreateDocument(char const* filename) {
    assert(filename);

    Document* doc = calloc(1, sizeof(Document));

    if (!doc) { return NULL; }

    if (SetFile(doc, filename)) {
        DestroyDocument(&doc);
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

size_t PrintDocument(FILE* output, Document const* doc) {
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

void PrintDocumentParameters(FILE* output, Document const* doc) {
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

size_t GetMaxBlockLen(ListBlock const* blocks) {
    assert(blocks && blocks->nodes);

    size_t maxLen = 0;
    for (Block* block = blocks->nodes; block; block = block->next) {
        if (maxLen < block->data.len) {
            maxLen = block->data.len;
        }
    }
    return maxLen;
}
