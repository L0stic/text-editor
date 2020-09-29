#include "String.h"

static int ResizeString(String* str) {
    assert(str->len >= str->size);

    char* tmpData = NULL;

    str->size = DIV_WITH_ROUND_UP(str->len, BASE_STRING_SIZE);
    str->size *= BASE_STRING_SIZE;

    tmpData = realloc(str->data, str->size * sizeof(char));

    if (tmpData) {
        str->data = tmpData;
        return 0;
    }
    return -1;
}

int ReserveSize(String* str, size_t size) {
    assert(str && !str->data && size > 0);

    str->size = DIV_WITH_ROUND_UP(size, BASE_STRING_SIZE);
    str->size *= BASE_STRING_SIZE;

    str->data = malloc(sizeof(char) * str->size);
    if (!str->data) { return -1; }

    return 0;
}

static int SetString(String* str, const char* src) {
    assert(str && src);

    str->len = strlen(src);
    if (str->len > 0) {
        str->size = DIV_WITH_ROUND_UP(str->len, BASE_STRING_SIZE);
        str->size *= BASE_STRING_SIZE;

        str->data = malloc(str->size * sizeof(char));
        if (!str->data) { return -1; }

        strncpy(str->data, src, str->len);
    }
    return 0;
}

String* CreateString(const char* src) {
    String* str = calloc(1, sizeof(String));
    if (str && src) { 
        if (SetString(str, src)) {
            free(str);
            return NULL;
        }
    }
    return str;
}

void DestroyString(String** ppStr) {
    assert(ppStr && *ppStr);

    if ((*ppStr)->data) { free((*ppStr)->data); }

    free(*ppStr);
    *ppStr = NULL;
}

int AddString(String* str, const char* src) {
    assert(str && src);

    if (str->data) {
        size_t oldLen = str->len;

        str->len += strlen(src);

        if (oldLen < str->len) {
            if (str->len > str->size && ResizeString(str)) { return -1; }

            strncpy(str->data + oldLen, src, str->len - oldLen);
        }
        return str->len - oldLen;
    }

    return SetString(str, src);
}

int AddChar(String* str, char c) {
    assert(str);

    if (str->data) {
        ++str->len;
        if (str->len > str->size && ResizeString(str)) { return -1; }

        str->data[str->len - 1] = c;
        str->data[str->len] = '\0';    
        return str->len;
    }

    char tmp[2];

    tmp[0] = c;
    tmp[1] = '\0';

    return SetString(str, tmp);
}

size_t PrintString(FILE* output, const String* str) {
    assert(str && str->data);

    if (!output) { output = stdout; }

    for (size_t i = 0; i < str->len; ++i) {
        fputc(str->data[i], output);
    }

    return str->len;
}
