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

    String* text = CreateString(NULL);
    if (!text) {
        fclose(file);
        return -3;
    }

    if (ReserveSize(text, (size_t)size)) {
        DestroyString(&text);
        fclose(file);
        return -4;
    }

    // Scaning
    char* buffer = malloc(BASE_STRING_SIZE * sizeof(char));
    if (!buffer) {
        DestroyString(&text);
        fclose(file);
        return -5;
    }

    // TODO: add error-handler
    while(!feof(file)) {
        if (fgets(buffer, BASE_STRING_SIZE, file)) { AddString(text, buffer); }
    }
    free(buffer);
    fclose(file);

    char* title = GetTitle(filename);
    if (!title) {
        DestroyString(&text);
        return -6;
    }

    SetText(doc, &text);
    SetTitle(doc, &title);
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

    if ((*ppDoc)->title) { free((*ppDoc)->title); }
    if ((*ppDoc)->text) { DestroyString(&((*ppDoc)->text)); }

    free(*ppDoc);
    *ppDoc = NULL;
}
